#include "utilities.h"

int main(int argc ,char* argv[]){
    home();//shell hme
    historyFile = getHistoryPath();
    OGSHELL_CONTROL();
    loadHistory();
    suggestCommands();
    while(1){
        printDetailsTakeInput();
        if(!input) {
            printf("\n");
            break;
        }
        if(*input == '\0') {
            // empty line, just ignore and next line
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