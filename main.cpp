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
#include <grp.h>

using namespace std;

#define GREEN "\033[1;92m"
#define RED   "\033[1;91m"
#define BLUE  "\033[1;94m"
#define RESET "\033[0m"

string homeDir;
string prevDir;
char *input;

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
    for(int i=0;i<3;i++){
        basePath += details[i];
    }
    basePath +=string(RESET) + ":";
    basePath+= string(RED) +"~"+getDirAdd() +string(RESET)+"> ";
    input=  readline(basePath.c_str());
    add_history(input);
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
                struct dirent *enteries;
                while ((enteries = readdir(dirpath)) != nullptr) {
                    string entryName = enteries->d_name;
                    if (!allhidden && entryName[0] == '.') 
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
        if(trimInput[i][0]=='-'){
            for(int j=1;j<trimInput[i].size();j++){
                if(trimInput[i][j]=='a') allhidden=true;
                if(trimInput[i][j]=='l') longlist=true;
            }
        }
        else if(trimInput[i] =="."){
            char path[PATH_MAX];
            if(!getcwd(path,sizeof(path))){
                perror("error in getcwd");
                exit(1);
            }
            string currentDIR = path;
            lspath_DIR.push_back(currentDIR);
        }
        else if(trimInput[i][0] =='~'){
            if(trimInput[i].size()>1){
                if(trimInput[i][1] =='/'){
                    string temp=getenv("HOME");
                    for(int k=1;k<trimInput[i].size();k++){
                        temp+=trimInput[i][k];
                    }
                    struct stat status;
                    if(lstat(trimInput[i].c_str(),&status)<0) {
                        perror(trimInput[i].c_str());
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

vector<string> trim(){
    char *temp = input;
    vector<string> trimInput;
    char *token;
    token= strtok(temp," \t");
    while (token!= NULL) {
        trimInput.push_back(token);
        token = strtok(NULL, " \t");
    }
    return trimInput;
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

int main(int argc ,char* argv[]){
    home();
    while(1){
        printDetailsTakeInput();
        vector<string> trimInput;
        if(input){
            trimInput= trim();
        }
        else{
            continue;
        }
        if(trimInput[0]=="pwd"){
            pwd();
        }
        if(trimInput[0]=="cd"){
            cd(trimInput);
        }
        if(trimInput[0]=="echo"){
            echo(trimInput);
        }
        if(trimInput[0]=="ls"){
            ls(trimInput);
        }
    }
    return 0;
}