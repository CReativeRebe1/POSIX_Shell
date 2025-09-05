#ifndef ESSENTIALS_H
#define ESSENTIALS_H

#include <bits/stdc++.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <pwd.h>
#include <limits.h>
#include <cstring>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <grp.h>
#include <ctime>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>

using namespace std;

#define GREEN "\033[1;92m"
#define RED   "\033[1;91m"
#define BLUE  "\033[1;94m"
#define RESET "\033[0m"

string homeDir;
string prevDir;
char *input;
vector<vector<string>> DoubleInput;
pid_t shell_processGID =0;
int IS_shell_Interactive =0;//TTY ?? if 0 not interactive
volatile sig_atomic_t fg_processGID = -1;//G = group : fg group
volatile sig_atomic_t fg_processID = -1;//current fg child
volatile sig_atomic_t FLAG_processFinish = 0;
vector<string> ACcommands = {"pwd", "cd", "echo", "ls", "pinfo", "search", "history"};

struct bgprocess{ 
    pid_t pid;
    string command;
};
vector<bgprocess> bg_process;

const int MaxHistory = 20;
string historyFile ;

struct RedirInfo {
    string infile;
    string outfile;
    bool append = false;
};

void home(){
    char temp[PATH_MAX];
    if(!getcwd(temp,sizeof(temp))){
        perror("error in getcwd");
        exit(1);
    }
    homeDir=temp;
}
vector<string> getDetails(){
    vector<string> details;
    struct passwd *pwuid = getpwuid(getuid());
    if(pwuid){
        details.push_back(pwuid->pw_name);//username
    }
    details.push_back("@");
    char hostname[HOST_NAME_MAX + 1];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        details.push_back(hostname);//hostname
    }
    return details;
}
string getDirAdd(){
    string currentDIR ;
    char path[PATH_MAX];
    if(!getcwd(path,sizeof(path))){
        perror("error in getcwd");
        exit(1);
    }
    string pathAdd = path;
    if(pathAdd.rfind(homeDir,0)==0){
        if(pathAdd.size() != homeDir.size()){
            currentDIR =  pathAdd.substr(homeDir.size());
        }
    }
    else{
        currentDIR = path;
    }
    return currentDIR;
}

void printDetailsTakeInput(){
    vector<string> details;
    details = getDetails();
    string basePath = string(GREEN);
    for(int i=0;i<details.size() && i<3;i++){
        basePath += details[i];
    }
    basePath +=string(RESET) + ":";
    basePath+= string(RED) +"~"+getDirAdd() +string(RESET)+"> ";
    input=  readline(basePath.c_str());
}
void SIGINT_CTRL_C(int signal){
    if(fg_processGID>0){
        kill(-fg_processGID,SIGINT);
        //send SIGINT to fg group
    }
}

void SIGTSTP_CTRL_Z(int signal){
    if(fg_processGID>0){
        kill(-fg_processGID,SIGTSTP);
        //send SIGTSTP to fg grp
    }
}
void SIGCHILD_HANDLE(int){
    int prev_errorno = errno;
    while(1){
        int status;
        pid_t pid =waitpid(-1,&status,WNOHANG);
        if(pid<=0) break;

        auto iter = remove_if(bg_process.begin(), bg_process.end(),[pid](const bgprocess &bg){ 
            return bg.pid == pid; 
        });
        bg_process.erase(iter, bg_process.end());

        char buffer[256];
        int n=0;
        if(WIFEXITED(status)){
            n=snprintf(buffer,sizeof(buffer),"process [%d] exited with status %d .\n" , pid,WEXITSTATUS(status));
        }
        else if(WIFSIGNALED(status)){
            n=snprintf(buffer,sizeof(buffer),"process [%d] killed with status %d .\n" , pid,WTERMSIG(status));
        }
        else{
            n=snprintf(buffer,sizeof(buffer),"process [%d] changed state.\n" , pid);
        }
        if(n>0){
            write(STDOUT_FILENO,buffer,(size_t)n);
        }
    }
    errno = prev_errorno;
}

void OGSHELL_CONTROL(){
    IS_shell_Interactive = isatty(STDIN_FILENO);
    if(!IS_shell_Interactive) return;//if not in OGSHELL return

    shell_processGID = getpid();
    if(setpgid(shell_processGID,shell_processGID)<0){
        perror("setpgid (SHELL) failed");
        exit(1);
    }
    //SHELL is in fg process group check
    if(tcsetpgrp(0,shell_processGID)<0){
        perror("tcsetpgrp (SHELL) failed");
        //meow
    }

    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    struct sigaction SA_CHILD;
    memset(&SA_CHILD,0,sizeof(SA_CHILD));
    SA_CHILD.sa_handler=SIGCHILD_HANDLE;
    sigemptyset(&SA_CHILD.sa_mask);
    SA_CHILD.sa_flags = SA_RESTART | SA_NOCLDSTOP;//ignore stopped children 
    if(sigaction(SIGCHLD,&SA_CHILD,nullptr)<0){//if child stopped
        perror("sigaction(SIGCHLD) failed");
        exit(1);
    }
    // forward CTRL+C and CTRL+Z
    struct sigaction SA_INT;
    memset(&SA_INT, 0, sizeof(SA_INT));
    SA_INT.sa_handler = SIGINT_CTRL_C;
    sigemptyset(&SA_INT.sa_mask);
    SA_INT.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &SA_INT, nullptr) < 0) {
        perror("sigaction(SIGINT) failed");
        exit(1);
    }

    struct sigaction SA_TSTP;
    memset(&SA_TSTP, 0, sizeof(SA_TSTP));
    // SA_TSTP.sa_handler = SIG_IGN;
    SA_TSTP.sa_handler = SIGTSTP_CTRL_Z;
    sigemptyset(&SA_TSTP.sa_mask);
    SA_TSTP.sa_flags = SA_RESTART;
    if (sigaction(SIGTSTP, &SA_TSTP, nullptr) < 0) {
        perror("sigaction(SIGTSTP) failed");
        exit(1);
    }
}

#endif