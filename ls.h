#ifndef LS_H
#define LS_H
#include "utilities.h"

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
    bool flagNoinput=false;
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
                flagNoinput=true;
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
    if(lspath_FILE.size()==0 && lspath_DIR.size()==0 && !flagNoinput){
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

#endif