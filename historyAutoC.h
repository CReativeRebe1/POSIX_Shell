#ifndef HISTORYAUTOC_H
#define HISTORYAUTOC_H
#include "utilities.h"

char* matchCommands(const char *text, int state) {
    static size_t idx, len;
    if(!state){
        idx = 0;
        len = strlen(text);
    }
    while (idx < ACcommands.size()) {
        const char* name = ACcommands[idx].c_str();
        idx++;
        if(strncmp(name, text, len)== 0){
            return strdup(name); 
        }
    }
    return nullptr;
}

char** completeCommands(const char *text, int start, int end) {
    bool is_first_word = true;
    for (int i = 0; i < start; ++i) {
        if (!isspace(rl_line_buffer[i])) {
            is_first_word = false;
            break;
        }
    }
    if(is_first_word){
        rl_attempted_completion_over= 1;
        return rl_completion_matches(text, matchCommands);
    } 
    rl_attempted_completion_over =0;
    return nullptr;
}

void suggestCommands() {
    rl_attempted_completion_function = completeCommands;
    std::sort(ACcommands.begin(), ACcommands.end());
}

static string getHistoryPath() {
    const char* home =getenv("HOME");
    if(!home) home =".";
    return string(home)+ "/.mysh_history";
}
void loadHistory() {
    using_history();
    stifle_history(MaxHistory);
    read_history(historyFile.c_str());
}
void saveHistory() {
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
#endif