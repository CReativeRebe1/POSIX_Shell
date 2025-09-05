#ifndef PINFOSEARCH_H
#define PINFOSEARCH_H
#include "utilities.h"

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

#endif