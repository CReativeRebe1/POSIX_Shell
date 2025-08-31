#include <bits/stdc++.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <pwd.h>
#include <limits.h>
#include <cstring>
using namespace std;

#define GREEN "\033[1;92m"
#define RED   "\033[1;91m"
#define BLUE  "\033[1;94m"
#define RESET "\033[0m"

string homeDir;
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
            currentDIR +=  pathAdd.substr(homeDir.size());
        }
    }
    return currentDIR;
}
void printDetailsTakeInput(){
    home();
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

void preProcess(){
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
    cout<<homeDir<<getDirAdd()<<endl;
}

int main(int argc ,char* argv[]){
    while(1){
        printDetailsTakeInput();
        vector<string> trimInput;
        if(input){
            trimInput= trim();
        }
        
        if(trimInput[0]=="pwd"){
            pwd();
        }
    }
    return 0;
}