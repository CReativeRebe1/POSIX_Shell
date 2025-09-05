#ifndef CD_H
#define CD_H
#include "utilities.h"

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

#endif