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

using namespace std;
char *input;
vector<vector<string>> DoubleInput;

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

void  multipleInputSplit(const char *input){
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


int main(){
    // string inp = "echo 'Hello  me  ; >> o World' > output.txt; cat output.txt | grep \"World\" >> result.log; echo Done";
    string inp="ls -a ~/Downloads ~ /home/yogendra-patel/MyGitWorkspace/aos/2025202018_A2/trim.cpp ~/Downloads/Web.pdf";
    multipleInputSplit(inp.c_str());
    for(auto i:DoubleInput){
        for(auto j:i){
            cout<<j<<endl;
        }
        cout<<endl;
    }
    return 0;
}