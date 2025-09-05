#ifndef REDIRECTIONPIPELINE_H
#define REDIRECTIONPIPELINE_H
#include "utilities.h"

RedirInfo parseRedirections(vector<string> &args) {
    RedirInfo r;
    for (size_t i = 0; i < args.size();) {
        if (args[i] == "<") {
            if (i + 1 >= args.size()) {
                cerr << "syntax error near unexpected token `<`\n";
                break;
            }
            r.infile = args[i+1];
            args.erase(args.begin() + i, args.begin() + i + 2);
        } else if (args[i] == ">") {
            if (i + 1 >= args.size()) {
                cerr << "syntax error near unexpected token `>`\n";
                break;
            }
            r.outfile = args[i+1];
            r.append = false;
            args.erase(args.begin() + i, args.begin() + i + 2);
        } else if (args[i] == ">>") {
            if (i + 1 >= args.size()) {
                cerr << "syntax error near unexpected token `>>`\n";
                break;
            }
            r.outfile = args[i+1];
            r.append = true;
            args.erase(args.begin() + i, args.begin() + i + 2);
        } else {
            i++;
        }
    }
    return r;
}

void applyRedirections(const RedirInfo &r) {
    if (!r.infile.empty()) {
        int fd_in = open(r.infile.c_str(), O_RDONLY);
        if (fd_in < 0) {
            perror(("cannot open " + r.infile).c_str());
            _exit(1);
        }
        dup2(fd_in, STDIN_FILENO);
        close(fd_in);
    }
    if (!r.outfile.empty()) {
        int flags = O_WRONLY | O_CREAT | (r.append ? O_APPEND : O_TRUNC);
        int fd_out = open(r.outfile.c_str(), flags, 0644);
        if (fd_out < 0) {
            perror(("cannot open " + r.outfile).c_str());
            _exit(1);
        }
        dup2(fd_out, STDOUT_FILENO);
        close(fd_out);
    }
}
bool IS_BUILT_IN_command(const string& cmd){
    bool temp = false;
    temp= (cmd== "pwd"||cmd =="cd"|| cmd =="echo" || cmd== "ls"|| cmd=="pinfo"||cmd=="search" ||cmd == "history");
    return temp;
}

string stripQuotesIfAny(const string &arg) {
    if (arg.size() >= 2) {
        if ((arg.front() == '"' && arg.back() == '"') ||
            (arg.front() == '\'' && arg.back() == '\'')) {
            return arg.substr(1, arg.size() - 2);
        }
    }
    return arg;
}

string stripQuotesAndTrim(const string &arg) {
    size_t start = arg.find_first_not_of(" \t\n");
    size_t end = arg.find_last_not_of(" \t\n");
    if (start == string::npos) return "";

    string s = arg.substr(start, end - start + 1);

    if (s.size() >= 2) {
        if ((s.front() == '"' && s.back() == '"') ||
            (s.front() == '\'' && s.back() == '\'')) {
            return s.substr(1, s.size() - 2);
        }
    }
    return s;
}


void RUN_NOT_BUILTIN(vector<string> args, RedirInfo r, bool IS_bg_process){
    if(args.empty()){
        return;
    }

    // prepare arguments (strip quotes for external commands)
    vector<char*> args_execute;
    vector<string> cleanedArgs;

    for (auto &val : args) {
        cleanedArgs.push_back(stripQuotesIfAny(val));
    }

    for (auto &arg : cleanedArgs) {
        args_execute.push_back(const_cast<char*>(arg.c_str()));
    }
    args_execute.push_back(nullptr);

    pid_t pid= fork();
    if(pid<0){
        perror("fork() failed");
        return;
    }

    if(pid==0){
        // child
        if(setpgid(0,0)<0){
            perror("setpgid(child) failed");
            _exit(1);
        }

        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
        signal(SIGINT,SIG_DFL);
        signal(SIGTSTP,SIG_DFL);

        applyRedirections(r);
        
        execvp(args_execute[0], args_execute.data());
        fprintf(stderr, "%s: command not found\n", args_execute[0]);
        _exit(127);
    }
    else{
        // parent side: put child in its own group
        for (int retry = 0; retry < 100; ++retry) {
            if (setpgid(pid, pid) == 0) break;
            if (errno == EACCES || errno == EPERM) {
                perror("setpgid (parent) failed");
                break;
            }
            if (errno == ESRCH) break;
            usleep(1000); // wait for child to setpgid
        }

        if(IS_bg_process){
            printf("[%d] %d running in background\n", (int)bg_process.size(), pid);
            fflush(stdout);
            bg_process.push_back({pid, cleanedArgs.empty() ? "" : cleanedArgs[0]});
            fg_processGID = -1;
            fg_processID = -1;
            return;
        }

        // foreground job
        fg_processID=pid;
        fg_processGID=pid;

        struct sigaction sa_old_ttou, sa_ignore;
        memset(&sa_ignore, 0, sizeof(sa_ignore));
        sa_ignore.sa_handler = SIG_IGN;
        sigaction(SIGTTOU, &sa_ignore, &sa_old_ttou);
        sigaction(SIGTTIN, &sa_ignore, nullptr);

        if(tcsetpgrp(STDIN_FILENO,fg_processGID)<0){
            perror("tcsetpgrp (to child) failed");
        }

        sigaction(SIGTTOU, &sa_old_ttou, nullptr);

        int status;
        while(1){
            pid_t bg_wait=waitpid(pid,&status,WUNTRACED);
            if(bg_wait<0){
                if(errno==EINTR) continue;
                perror("waitpid failed");
                break;
            }
            if(WIFEXITED(status) || WIFSIGNALED(status)){
                break;
            }
            if(WIFSTOPPED(status)){
                printf("stopped [%d]\n",pid);
                fflush(stdout);
                bg_process.push_back({pid, cleanedArgs.empty() ? "" : cleanedArgs[0]});
                printf("\n");
                break;
            }
        }
    }

    // restore terminal control to shell
    if(tcsetpgrp(STDIN_FILENO, shell_processGID)<0) {
        perror("tcsetpgrp (restore shell) failed");
    }
    fg_processGID= -1;
    fg_processID=-1;
}

vector<vector<string>> splitPipeline(const vector<string>& trimInput) {
    vector<vector<string>> PipelineStages;
    vector<string> current;
    for (auto &val:trimInput){
        if(val== "|") {
            if(!current.empty()) PipelineStages.push_back(current);
            current.clear();
        } else {
            current.push_back(val);
        }
    }
    if (!current.empty()) PipelineStages.push_back(current);
    return PipelineStages;
}

void RUN_PIPELINE(vector<vector<string>> stages, bool IS_bg) {
    int n = stages.size();
    vector<int> pipesFD(2*(n-1));
    for(int i=0;i<n-1;i++){
        if(pipe(&pipesFD[2*i])<0){
            perror("pipe failed");
            return;
        }
    }

    vector<pid_t> pids;
    for(int i=0;i<n;i++){
        RedirInfo r = parseRedirections(stages[i]); // reuse redirection parser
        if(stages[i].empty()) continue;

        pid_t pid = fork();
        if(pid<0){
            perror("fork failed");
            return;
        }

        if(pid==0){
            // child
            // set pgid so all children in one group
            setpgid(0,0);

            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            signal(SIGTTIN, SIG_DFL);
            signal(SIGTTOU, SIG_DFL);

            // input pipe
            if(i>0){
                dup2(pipesFD[2*(i-1)], STDIN_FILENO);
            }
            // output pipe
            if(i<n-1){
                dup2(pipesFD[2*i+1], STDOUT_FILENO);
            }
            // close all fds
            for(int j=0;j<2*(n-1);j++) close(pipesFD[j]);

            // now redirections override pipes
            applyRedirections(r);

            // decide built-in or external
            if(IS_BUILT_IN_command(stages[i][0])){
                if(stages[i][0]=="cd"){
                    cerr<<"cd: not effective inside pipeline\n";
                } else if(stages[i][0]=="pwd") pwd();
                else if(stages[i][0]=="echo") echo(stages[i]);
                else if(stages[i][0]=="ls") ls(stages[i]);
                else if(stages[i][0]=="pinfo") pinfo(stages[i]);
                else if(stages[i][0]=="search") search(stages[i]);
                else if(stages[i][0]=="history") history(stages[i]);
                _exit(0);
            } else {
                vector<string> cleanedArgs;
                for(const auto &a : stages[i]) {
                    cleanedArgs.push_back(stripQuotesIfAny(a));
                }
                vector<char*> args_exec;
                for (const auto &arg : cleanedArgs) {
                    args_exec.push_back(const_cast<char*>(arg.c_str()));
                }
                args_exec.push_back(nullptr);

                execvp(args_exec[0], args_exec.data());
                perror(args_exec[0]);
                _exit(127);
            }
        } else {
            pids.push_back(pid);
            if(i==0){
                // first child creates new group
                setpgid(pid, pid);
            } else {
                // others join same group
                setpgid(pid, pids[0]);
            }
        }
    }
    // parent closes pipes
    for(int j=0;j<2*(n-1);j++) close(pipesFD[j]);

    if(IS_bg){
        cout<<"["<<bg_process.size()<<"] "<<pids.back()<<" running in background\n";
        bg_process.push_back({pids.back(), stages[0][0]});
        return;
    }
    // wait for all
    fg_processGID = pids[0];
    tcsetpgrp(STDIN_FILENO, fg_processGID);

    for(pid_t pid : pids) {
        int status;
        waitpid(pid, &status, 0);
    }

    // restore control to shell
    tcsetpgrp(STDIN_FILENO, shell_processGID);
    fg_processGID = -1;
}

#endif