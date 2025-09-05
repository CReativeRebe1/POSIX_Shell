#ifndef UTILITIES_H
#define UTILITIES_H
#include "essentials.h"
#include "cd.h"
#include "historyAutoC.h"
#include "ls.h"
#include "pinfoSearch.h"
#include "redirectionPipeline.h"

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

void fixedinputloop(){
    for(size_t i=0;i<DoubleInput.size();i++){
        vector<string> trimInput = DoubleInput[i];
        if(trimInput.empty()) continue;

        if(trimInput[0] == "exit") {
            DoubleInput.clear();
            printf("Exiting shell...\n");
            exit(0); // terminate shell immediately
        }

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

#endif