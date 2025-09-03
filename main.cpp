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

void getPermission(mode_t mode){
    char type;
    if(S_ISDIR(mode)) type='d';
    else if(S_ISLNK(mode)) type='l';
    else if(S_ISCHR(mode)) type='c';
    else if(S_ISBLK(mode)) type='b';
    else if(S_ISSOCK(mode)) type='s';
    else if(S_ISFIFO(mode)) type='p';
    else type='-';
    cout<<string(BLUE)<<type;

    cout<<((mode & S_IRUSR)?'r':'-');
    cout<<((mode & S_IWUSR)?'w':'-');
    cout<<((mode & S_IXUSR)?'x':'-');
    cout<<((mode & S_IRGRP)?'r':'-');
    cout<<((mode & S_IWGRP)?'w':'-');
    cout<<((mode & S_IXGRP)?'x':'-');
    cout<<((mode & S_IROTH)?'r':'-');
    cout<<((mode & S_IWOTH)?'w':'-');
    cout<<((mode & S_IXOTH)?'x':'-');
    cout<<string(RESET);
}

void lsprint_longlist(string &lspath,string &entryName){
    string path;
    if(!entryName.empty() && entryName[0]=='/'){
        path=entryName;
    }
    else{
        path =lspath +"/" +entryName;
    }
    struct stat status;
    if(lstat(path.c_str(), &status)<0){
        perror("lstat");
        return;
    }
    getPermission(status.st_mode);
    cout<<" "<<status.st_nlink;
    struct passwd *passWD = getpwuid(status.st_uid);
    cout <<" "<<string(GREEN)<<(passWD?passWD->pw_name:"?")<<string(RESET);
    struct group  *gr = getgrgid(status.st_gid);
    cout <<" "<<string(GREEN)<<(gr?gr->gr_name:"?")<<string(RESET);
    cout <<" "<<status.st_size;

    char timebuf[64];
    struct tm *tm = localtime(&status.st_mtime);
    strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", tm);
    cout <<" "<<timebuf;
    cout <<" "<<entryName+"\n";
}

void calDIR_TotalBlocks(string &lspath ,bool allhidden ){
    long long totalBlocks=0;
    DIR *dirpath = opendir(lspath.c_str());
    if(!dirpath){
        perror(lspath.c_str());
        return;
    }
    struct dirent *enterie;
    while((enterie = readdir(dirpath)) != nullptr) {
        string entryName = enterie->d_name;
        if(!allhidden && entryName[0] == '.') 
               continue;
        string fullpath = lspath + "/" + entryName;
        struct stat status;
        if (lstat(fullpath.c_str(), &status) == 0) {
            totalBlocks += status.st_blocks;
        }
    }
    cout<<"total "<<(totalBlocks/2)<<endl;
    closedir(dirpath);
}

void lsloop(vector<string> &lspath , bool allhidden , bool longlist){
    if(lspath.size()){
        for(long long i=0;i<lspath.size();i++){
            struct stat status; 
            if(lstat(lspath[i].c_str(),&status)<0){
                perror("Stat not found");
                continue;
            }
            if(S_ISDIR(status.st_mode)){
                if(lspath.size()>1){
                    cout<<lspath[i]<<":\n";
                }
                DIR *dirpath = opendir(lspath[i].c_str());
                if(!dirpath){
                    perror(lspath[i].c_str());
                    continue;
                }
                if(longlist){
                    calDIR_TotalBlocks(lspath[i],allhidden);
                }
                struct dirent *enteries;
                while((enteries = readdir(dirpath)) != nullptr) {
                    string entryName = enteries->d_name;
                    if(!allhidden && entryName[0] == '.') 
                        continue;
                    if(longlist){
                        lsprint_longlist(lspath[i],entryName);
                    }
                    else{
                        cout<<entryName<<endl;
                    }
                }
                closedir(dirpath);
            }
            else if(longlist){
                string temp = "";
                lsprint_longlist(temp,lspath[i]);
            }
            else{
                cout<<lspath[i]<<endl;
            }
        }
    }
}

void ls(vector<string> &trimInput){
    vector<string> lspath_FILE;
    vector<string> lspath_DIR;//to print dir last  if given input
    bool allhidden =false , longlist =false;
    for(int i=1;i<trimInput.size();i++){
        if(trimInput[i][0]== '-'){
            for(int j=1;j<trimInput[i].size();j++){
                if(trimInput[i][j]=='a') allhidden=true;
                if(trimInput[i][j]=='l') longlist=true;
            }
        }
        else if(trimInput[i] == "."){
            char path[PATH_MAX];
            if(!getcwd(path,sizeof(path))){
                perror("error in getcwd");
                exit(1);
            }
            string currentDIR = path;
            lspath_DIR.push_back(currentDIR);
        }
        else if(trimInput[i][0] == '~'){
            if(trimInput[i].size()>1){
                if(trimInput[i][1] =='/'){
                    string temp = string(getenv("HOME")) + trimInput[i].substr(1);
                    struct stat status;
                    if(lstat(temp.c_str(),&status)<0) {
                        perror(temp.c_str());
                        continue;
                    }
                    if(S_ISDIR(status.st_mode)){
                        lspath_DIR.push_back(temp);
                    }
                    else{
                        lspath_FILE.push_back(temp);
                    }
                }
            }else{
                lspath_DIR.push_back(getenv("HOME"));
            }
        }
        else{
            struct stat status;
            if(lstat(trimInput[i].c_str(),&status)<0) {
                perror(trimInput[i].c_str());
                continue;
            }
            if(S_ISDIR(status.st_mode)){
                lspath_DIR.push_back(trimInput[i]);
            }
            else{
                lspath_FILE.push_back(trimInput[i]);
            }
        }
    }
    if(lspath_FILE.size()==0 && lspath_DIR.size()==0){
        char path[PATH_MAX];
            if(!getcwd(path,sizeof(path))){
                perror("error in getcwd");
                exit(1);
            }
            string currentDIR = path;
            lspath_DIR.push_back(currentDIR);
    }
    if(lspath_FILE.size()!=0){
        lsloop(lspath_FILE,allhidden,longlist);
    }
    if(lspath_FILE.size()!=0 && lspath_DIR.size()!=0){
        cout<<endl;
    }
    if(lspath_DIR.size()!=0){
        lsloop(lspath_DIR,allhidden,longlist);
    }
}

vector<string> Normaltrim(string currInput){
    vector<string> token;
    string result;
    bool FLAG_doublequote =false,FLAG_singlequote =false,prevSpace=true;

    for (size_t i=0;i<currInput.size();i++){
        unsigned char val = currInput[i];
        if(val== '"' && !FLAG_singlequote){
            FLAG_doublequote = !FLAG_doublequote;
            result.push_back(val);
            prevSpace=false;
        } 
        else if(val== '\'' && !FLAG_doublequote){
            FLAG_singlequote = !FLAG_singlequote;
            result.push_back(val);
            prevSpace=false;
        }
        else if(isspace(val) &&(!FLAG_singlequote && !FLAG_doublequote)){
            if(!prevSpace && !result.empty()){
                token.push_back(result);
                result.clear();
            }
            prevSpace=true;
        }
        else{
            result.push_back(val);
            prevSpace=false;
        }
    }
    if(!result.empty()){
        token.push_back(result);
    }
    return token;
}

string trimHandleOperator(string currInput){
    string result;
    result.reserve(currInput.size()*2);
    bool FLAG_doublequote =false,FLAG_singlequote =false;

    for(size_t i=0;i<currInput.size();i++){
        char val = currInput[i];
        if (val == '"' && !FLAG_singlequote){
            FLAG_doublequote = !FLAG_doublequote;
            result.push_back(val);
            continue;
        } 
        else if(val == '\'' && !FLAG_doublequote){
            FLAG_singlequote = !FLAG_singlequote;
            result.push_back(val);
            continue;
        }
        
        if(!FLAG_singlequote && !FLAG_doublequote){
            if(val=='>'){
                if(i+1<currInput.size() && currInput[i+1]=='>'){
                    result+=" >> ";
                    i++;
                }
                else{
                    result+=" > ";
                }
            }
            else if(val=='|' || val=='<' || val=='&'){
                result.push_back(' ');
                result.push_back(val);
                result.push_back(' ');
            }
            else{
                result.push_back(val);
            }
        }
        else{
            result.push_back(val);
        }
    }
    return result;
}

void multipleInputSplit(){
    if(!input) return ;

    char *currInput = strdup(input);
    if(!currInput) return ;

    const char ESCAPEDELIMITER = '\x1E';
    bool FLAG_doublequote =false,FLAG_singlequote =false;

    for (size_t i = 0;currInput[i];i++){
        unsigned char val = currInput[i];
        if (val == '"' && !FLAG_singlequote){
            FLAG_doublequote = !FLAG_doublequote;
        } 
        else if(val == '\'' && !FLAG_doublequote){
            FLAG_singlequote = !FLAG_singlequote;
        }
        else if(val == ';' && (FLAG_singlequote || FLAG_doublequote)){
            currInput[i] =ESCAPEDELIMITER;
        }
    }
    char *token = strtok(currInput,";");
    while(token){
        string str(token);
        replace(str.begin(),str.end(),ESCAPEDELIMITER,';');
        string temp1 = trimHandleOperator(str);
        vector<string> temp2 = Normaltrim(temp1);
        DoubleInput.push_back(temp2);
        token =strtok(NULL, ";");
    }
    free(currInput);
}

void pwd(){
    char path[PATH_MAX];
    if(!getcwd(path,sizeof(path))){
        perror("error in getcwd");
        exit(1);
    }
    string currentDIR = path;
    cout<<currentDIR<<endl;
}

void cd(vector<string> &trimInput){
    char path[PATH_MAX];
    if(!getcwd(path,sizeof(path))){
        perror("error in getcwd");
        exit(1);
    }
    string currDir = path;
    if(trimInput.size()>2){
        cout<<"cd : Error Invalid arguements\n";
        return;
    }
    if(trimInput.size()==1){
        if(chdir(getenv("HOME"))!=0){
            perror("chdir failed");
        }
    }
    else if(trimInput[1]=="~"){
        if(chdir(getenv("HOME"))!=0){
            perror("chdir failed");
        }
    }
    // else if(trimInput[1]=="."){
    //     if(chdir(trimInput[1].c_str())!=0){
    //         perror("chdir failed");
    //     }
    // }
    // else if(trimInput[1]==".."){
    //     if(chdir(trimInput[1].c_str())!=0){
    //         perror("chdir failed");
    //     }
    // }
    else if(trimInput[1]=="-"){
        cout<<prevDir<<"\n";
        if(chdir(prevDir.c_str())!=0){
            perror("chdir failed");
        }
    }
    else{
        if(chdir(trimInput[1].c_str())!=0){
            perror("chdir failed");
        }
    }
    prevDir =currDir;
}

void echo(vector<string> &trimInput){
    if(trimInput.size()==1){
        cout<<endl;
    }
    else{
        for(int i=1;i<trimInput.size();i++){
            cout<<trimInput[i]<<" ";
        }
        cout<<endl;
    }
}

char pinfo_getstate(FILE *&Piter){
    char stateline[512];
    char processState='?';
    while(fgets(stateline,sizeof(stateline),Piter)){
        if(strncmp(stateline,"State:",6)==0){
            char*ptr =strchr(stateline,'\t');
            if(ptr){
                while(*ptr && !isalpha(*ptr))
                    ptr++;
                if(*ptr)
                    processState=*ptr;
            }
            break;
        }
    }
    return processState;
}

string pinfo_resolvepath(const string &processEXEpath) {
    vector<char> buffer(PATH_MAX);
    ssize_t pathlength=readlink(processEXEpath.c_str(), buffer.data(),buffer.size()-1);

    if (pathlength ==-1) {
        return "[N/A]";//unreadable process
    }

    buffer[pathlength] = '\0';
    string resolvedPath(buffer.data());

    if(!homeDir.empty()&& resolvedPath.compare(0,homeDir.size(),homeDir)== 0) {
        resolvedPath = "~" +resolvedPath.substr(homeDir.size());
    }
    return resolvedPath;
}

void pinfo(vector<string> &trimInput){
    pid_t pid;
    if(trimInput.size()==1){
        pid=getpid();
    }
    else{
        try{
            pid=stoi(trimInput[1]);
            if(pid<=0){
                cerr<<"pinfo : invalid PID\n";
                return;
            }
        }catch(...){
            cerr<<"pinfo : invalid PID\n";
            return;
        }
    }

    string stat_path ="/proc/"+to_string(pid)+ "/stat";
    FILE* file =fopen(stat_path.c_str(), "r");
    if(!file){
        perror(("Cannot open " + stat_path).c_str());
        return;
    }

    string processEXEpath ="/proc/"+ to_string(pid)+ "/exe";
    string RES_exepath = pinfo_resolvepath(processEXEpath);

    long long temppid;
    char comm[256];
    char processState;
    long long temp;
    long long vsize;
    long rsetsize;

    // Read first 3 fields: pid, executable name, state
    if (fscanf(file, "%lld %s %c", &temppid, comm, &processState) != 3) {
        cerr << "Failed to read /proc/[pid]/stat" << endl;
        fclose(file);
        return;
    }
    for(int i = 0; i < 19; i++) {
        if(fscanf(file, "%lld", &temp) != 1) {
            cerr << "Failed to skip fields" << endl;
            fclose(file);
            return;
        }
    }

    //Field23:virtual memory size
    if(fscanf(file, "%lld", &vsize) != 1) vsize = -1;

    //Field 24resident set size
    if(fscanf(file, "%ld", &rsetsize) != 1) rsetsize = -1;

    fclose(file);
    
    string state =string(1,processState);
    if(getpgid(pid) != -1 && getpgid(pid)==fg_processGID){
        state+="+";
    }
    long page_size = sysconf(_SC_PAGESIZE); // bytes per page
    long long VMSize = rsetsize * page_size / 1024;

    cout<<"Process Status : "<<state<<endl;
    if(VMSize>=0){
        cout<<"Memory : "<<VMSize<<" KB"<<endl;
    }
    else{
        cout<<"Memory : N/A"<<endl;
    }
    cout<<"Executable Path : "<<RES_exepath<<endl;
}

bool SearchRecursive(const string &dirpath,const string &target){
    DIR *curdir =opendir(dirpath.c_str());
    if(!curdir){
        perror(dirpath.c_str());
        return false;
    }
    struct dirent *entries;
    while ((entries = readdir(curdir)) != nullptr) {
        string name = entries->d_name;

        if (name == "." || name == "..") continue;

        string fullpath =dirpath;
        if (!fullpath.empty()&& fullpath.back()!= '/') 
            fullpath+= '/';
        fullpath +=name;

        struct stat status;
        if (lstat(fullpath.c_str(), &status) < 0) {
            perror("lstat failed");
            continue;
        }
        if(S_ISLNK(status.st_mode))
            continue;

        if (name == target) {
            closedir(curdir);
            return true;
        }

        if (S_ISDIR(status.st_mode)) {
            if (SearchRecursive(fullpath, target)) {
                closedir(curdir);
                return true;
            }
        }
    }
    closedir(curdir);
    return false;
}

void search(vector<string> &trimInput){
    if(trimInput.size()!=2){
        cerr<<"Invalid , correct arg : search <name>"<<endl;
        return;
    }
    bool targetPresent =false;
    string currentDIR ;
    char path[PATH_MAX];
    if(!getcwd(path,sizeof(path))){
        perror("error in getcwd");
        exit(1);
    }
    currentDIR = path;
    string target =trimInput[1];

    targetPresent=SearchRecursive(currentDIR,target);
    if (targetPresent) 
        cout<<"True"<<endl;
    else 
        cout<<"False"<<endl;
}

void loadHistory() {
    using_history();
    stifle_history(MaxHistory);
    read_history(historyFile.c_str());
}
void saveHistory() {
    int length = history_length;
    if (length > MaxHistory) {
        for (int i = 0; i < length - MaxHistory; i++) {
            remove_history(0);
        }
    }
    write_history(historyFile.c_str());
}
void addToHistory(const string &cmd) {
    bool onlyWhitespace = true;
    for(char c:cmd){
        if(!isspace(static_cast<unsigned char>(c))) {
            onlyWhitespace = false;
            break;
        }
    }
    if(onlyWhitespace)return;
    int length = history_length;
    if(length > 0){
        HIST_ENTRY *last = history_get(length);
        if(last && strcmp(cmd.c_str(),last->line)== 0){
            return;
        }
    }
    add_history(cmd.c_str());
    if (history_length>MaxHistory) {
        remove_history(0);
    }
    saveHistory();
}
void history(vector<string> &trimInput){
    int count =10;
    if(trimInput.size()==2){
        try{
            count=stoi(trimInput[1]);
            if(count<0) count=0;
            if(count>MaxHistory) count=MaxHistory;
        }
        catch(...){
            cerr<<"history: invalid number\n";
            return;
        }
    }
    int length =history_length;
    int st=max(0,length-count);
    for(int i=st;i<length;i++){
        if(HIST_ENTRY* entry = history_get(i+1)){
            cout<<entry->line<<endl;
        }
    }
}
static string getHistoryPath() {
    const char* h = getenv("HOME");
    if (!h) h = ".";
    return string(h) + "/.mysh_history";
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
            sigset_t mask, oldmask;
            sigemptyset(&mask);
            sigaddset(&mask, SIGCHLD);
            sigprocmask(SIG_BLOCK, &mask, &oldmask);
            printf("[%d] %d running in background\n", (int)bg_process.size(), pid);
            fflush(stdout);
            bg_process.push_back({pid, cleanedArgs.empty() ? "" : cleanedArgs[0]});
            sigprocmask(SIG_SETMASK, &oldmask, nullptr);
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
                sigset_t mask, oldmask;
                sigemptyset(&mask);
                sigaddset(&mask, SIGCHLD);
                sigprocmask(SIG_BLOCK, &mask, &oldmask);
                printf("stopped [%d]\n",pid);
                fflush(stdout);
                bg_process.push_back({pid, cleanedArgs.empty() ? "" : cleanedArgs[0]});
                sigprocmask(SIG_SETMASK, &oldmask, nullptr);
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
                vector<char*> args_exec;
                for(auto &a: stages[i]) args_exec.push_back(const_cast<char*>(a.c_str()));
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

void fixedinputloop(){
    for(size_t i=0;i<DoubleInput.size();i++){
        vector<string> trimInput = DoubleInput[i];
        if(trimInput.empty()) continue;

        // detect background
        bool IS_bg = false;
        if(!trimInput.empty() && trimInput.back()=="&"){
            IS_bg=true;
            trimInput.pop_back();
        }

        // check if pipeline
        bool hasPipe = find(trimInput.begin(), trimInput.end(), "|")!=trimInput.end();
        if(hasPipe){
            auto stages = splitPipeline(trimInput);
            RUN_PIPELINE(stages, IS_bg);
            continue;
        }

        // no pipeline
        RedirInfo r = parseRedirections(trimInput);
        if(trimInput.empty()) continue;

        if(IS_BUILT_IN_command(trimInput[0])){
            if(trimInput[0]=="cd"){
                if(!r.infile.empty() || !r.outfile.empty()){
                    cerr<<"cd: redirection not supported\n";
                } else cd(trimInput);
            } else {
                pid_t pid=fork();
                if(pid==0){
                    applyRedirections(r);
                    if(trimInput[0]=="pwd") pwd();
                    else if(trimInput[0]=="echo") echo(trimInput);
                    else if(trimInput[0]=="ls") ls(trimInput);
                    else if(trimInput[0]=="pinfo") pinfo(trimInput);
                    else if(trimInput[0]=="search") search(trimInput);
                    else if(trimInput[0]=="history") history(trimInput);
                    _exit(0);
                } else {
                    int status;
                    waitpid(pid,&status,0);
                }
            }
        } else {
            RUN_NOT_BUILTIN(trimInput, r, IS_bg);
        }
    }
    DoubleInput.clear();
}

int main(int argc ,char* argv[]){
    home();
    historyFile = getHistoryPath();
    OGSHELL_CONTROL();
    loadHistory();
    while(1){
        printDetailsTakeInput();
        if(!input) {
            printf("\n");
            break;
        }
        if(*input == '\0') {
            // empty line, just re-prompt
            free(input);
            continue;
        }
        string cmd(input);
        if (!cmd.empty()) {
            addToHistory(cmd);
        }
        multipleInputSplit();
        fixedinputloop();
        free(input);
        input=nullptr;
    }
    saveHistory();
    return 0;
}