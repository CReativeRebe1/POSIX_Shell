// long long totalBlocks = 0;
// DIR *dir = opendir(lspath[i].c_str());
// struct dirent *entry;

// while ((entry = readdir(dir)) != nullptr) {
//     string entryName = entry->d_name;
//     if (!allhidden && entryName[0] == '.') continue;

//     string fullpath = lspath[i] + "/" + entryName;
//     struct stat st{};
//     if (lstat(fullpath.c_str(), &st) == 0) {
//         totalBlocks += st.st_blocks;
//     }
// }
// closedir(dir);

// cout << "total " << (totalBlocks / 2) << "\n";  // divide by 2 for 1K blocks









// if (S_ISLNK(status.st_mode)) {
//     char buf[PATH_MAX];
//     ssize_t len = readlink(path.c_str(), buf, sizeof(buf)-1);
//     if (len != -1) {
//         buf[len] = '\0';
//         cout << " -> " << buf;
//     }
// }




















// ce outside quotes (preserve quotes & inner spaces)
//     string quoted = preprocess_quotes(group);

//     // 2) insert spaces around operators outside quotes
//     string spaced = preprocess_ops_quote_aware(quoted);

//     // 3) tokenize by whitespace (strtok)
//     vector<string> toks = tokenize_ws_strtok(spaced);

//     // 4) build stages split at '|' tokens
//     Stage cur;
//     auto flush_stage = [&](bool allow_empty = false) {
//         if (!cur.argv.empty(#include <iostream>
// #include <string>
// #include <vector>
// #include <cstring>   // strdup, strtok
// #include <cctype>    // isspace
// #include <cstdlib>   // free
// #include <algorithm> // replace
// using namespace std;

// // --- Data structures ---
// struct Stage {
//     vector<string> argv;     // program + args (quotes preserved)
//     string in_file;          // "<" filename (may include quotes)
//     string out_file;         // ">" or ">>" filename (may include quotes)
//     bool   append = false;   // true if ">>"
// };

// struct Pipeline {
//     vector<Stage> stages;    // split by "|"
//     bool background = false; // "&" on the whole pipeline
// };

// // --- Utilities ---
// static inline string trim_ws(const string &s) {
//     size_t i = 0, j = s.size();
//     while (i < j && isspace(static_cast<unsigned char>(s[i]))) i++;
//     while (j > i && isspace(static_cast<unsigned char>(s[j-1]))) j--;
//     return s.substr(i, j - i);
// }

// // --- Step 1: split by semicolon BUT be quote-aware ---
// // We replace semicolons inside quotes with a placeholder, then call strtok(buf, ";").
// // After tokenizing, we restore the placeholder back to ';'.
// vector<string> split_groups_by_semicolon_quote_aware(const char *line) {
//     vector<string> groups;
//     if (!line) return groups;

//     char *buf = strdup(line);
//     if (!buf) return groups;

//     // placeholder for semicolons that are inside quotes
//     const char PLACEHOLDER = '\x1E'; // record separator (unlikely in normal input)

//     bool in_single = false, in_double = false;
//     for (size_t i = 0; buf[i]; ++i) {
//         char c = buf[i];
//         if (c == '"' && !in_single) {
//             in_double = !in_double;
//         } else if (c == '\'' && !in_double) {
//             in_single = !in_single;
//         } else if (c == ';' && (in_single || in_double)) {
//             buf[i] = PLACEHOLDER; // mask it so strtok won't split here
//         }
//     }

//     char *tok = strtok(buf, ";"); // strtok required by assignment
//     while (tok) {
//         string s(tok);
//         // restore any masked semicolons
//         replace(s.begin(), s.end(), PLACEHOLDER, ';');
//         s = trim_ws(s);
//         if (!s.empty()) groups.push_back(s);
//         tok = strtok(NULL, ";");
//     }

//     free(buf);
//     return groups;
// }

// // --- Quote-aware whitespace normalization ---
// // Collapse runs of spaces/tabs outside quotes into a single space.
// // Inside quotes keep spacing exactly as typed.
// // DO NOT remove the quotes (they stay part of tokens).
// string preprocess_quotes(const string &in) {
//     string out;
//     out.reserve(in.size());
//     bool in_single = false, in_double = false;

//     for (size_t i = 0; i < in.size(); ++i) {
//         char c = in[i];

//         if (c == '"' && !in_single) {
//             in_double = !in_double;
//             out.push_back(c);
//         } else if (c == '\'' && !in_double) {
//             in_single = !in_single;
//             out.push_back(c);
//         } else if ((c == ' ' || c == '\t') && !in_single && !in_double) {
//             // outside quotes → normalize to one space
//             if (!out.empty() && out.back() != ' ')
//                 out.push_back(' ');
//         } else {
//             out.push_back(c);
//         }
//     }
//     return out;
// }

// // --- Insert spaces around operators but skip content inside quotes ---
// // We make '>>' a single token ">>" (we detect adjacent '>' outside quotes).
// string preprocess_ops_quote_aware(const string &in) {
//     string out;
//     out.reserve(in.size() * 2);

//     bool in_single = false, in_double = false;
//     for (size_t i = 0; i < in.size(); ++i) {
//         char c = in[i];

//         // toggle quote state, and always copy the quote char
//         if (c == '"' && !in_single) {
//             in_double = !in_double;
//             out.push_back(c);
//             continue;
//         } else if (c == '\'' && !in_double) {
//             in_single = !in_single;
//             out.push_back(c);
//             continue;
//         }

//         if (!in_single && !in_double) {
//             if (c == '>') {
//                 // detect ">>" (append) as one token
//                 if (i + 1 < in.size() && in[i+1] == '>') {
//                     out += " >> ";
//                     ++i; // skip the second '>'
//                 } else {
//                     out += " > ";
//                 }
//             } else if (c == '|' || c == '<' || c == '&') {
//                 out.push_back(' ');
//                 out.push_back(c);
//                 out.push_back(' ');
//             } else {
//                 out.push_back(c);
//             }
//         } else {
//             // inside quotes: copy as-is (do not space operators)
//             out.push_back(c);
//         }
//     }
//     return out;
// }

// // --- Tokenize on whitespace using strtok (required) ---
// // Because we already protected quoted segments, they remain whole tokens.
// vector<string> tokenize_ws_strtok(const string &s) {
//     vector<string> tokens;
//     char *buf = strdup(s.c_str());
//     if (!buf) return tokens;
//     char *tok = strtok(buf, " \t\r\n");
//     while (tok) {
//         tokens.emplace_back(tok);
//         tok = strtok(NULL, " \t\r\n");
//     }
//     free(buf);
//     return tokens;
// }

// // --- Parse a single group (one semicolon-separated chunk) into a Pipeline ---
// Pipeline parse_group_into_pipeline(const string &group) {
//     Pipeline pipe;

//     // 1) normalize whitespa) || !cur.in_file.empty() || !cur.out_file.empty() || allow_empty) {
//             pipe.stages.push_back(cur);
//         }
//         cur = Stage{};
//     };

//     for (size_t i = 0; i < toks.size(); ++i) {
//         const string &t = toks[i];

//         if (t == "|") {
//             // pipe boundary → finish current stage
//             flush_stage(false);
//         } else if (t == "<") {
//             if (i + 1 < toks.size()) {
//                 cur.in_file = toks[++i];
//             } else {
//                 cerr << "syntax error: expected filename after '<'\n";
//             }
//         } else if (t == ">" || t == ">>") {
//             bool append = (t == ">>");
//             if (i + 1 < toks.size()) {
//                 cur.out_file = toks[++i];
//                 cur.append   = append;
//             } else {
//                 cerr << "syntax error: expected filename after '" << t << "'\n";
//             }
//         } else if (t == "&") {
//             // background marker for the whole pipeline (accept anywhere)
//             pipe.background = true;
//             break; // ignore anything after '&' for this group
//         } else {
//             // normal argument/program (quotes still present if used)
//             cur.argv.push_back(t);
//         }
//     }

//     // push last stage (if any)
//     flush_stage(false);

//     // remove empty stages caused by stray '|' (keep stages that have any data)
//     vector<Stage> filtered;
//     for (auto &s : pipe.stages) {
//         if (!s.argv.empty() || !s.in_file.empty() || !s.out_file.empty())
//             filtered.push_back(std::move(s));
//     }
//     pipe.stages.swap(filtered);

//     return pipe;
// }

// // --- Top-level parse: full input line -> pipelines vector ---
// vector<Pipeline> parse_line_into_pipelines(const char *line) {
//     vector<Pipeline> result;
//     auto groups = split_groups_by_semicolon_quote_aware(line);
//     for (const string &g : groups) {
//         result.push_back(parse_group_into_pipeline(g));
//     }
//     return result;
// }

// // --- Debug printer ---
// void debug_print(const vector<Pipeline> &pipes) {
//     for (size_t i = 0; i < pipes.size(); ++i) {
//         const auto &p = pipes[i];
//         cout << "Command group " << i << (p.background ? " (background)" : "") << ":\n";
//         for (size_t s = 0; s < p.stages.size(); ++s) {
//             const auto &st = p.stages[s];
//             cout << "  Stage " << s << ":";
//             for (auto &a : st.argv) cout << " [" << a << "]";
//             if (!st.in_file.empty())  cout << "  < "  << st.in_file;
//             if (!st.out_file.empty()) cout << "  " << (st.append?">>":">") << " " << st.out_file;
//             cout << "\n";
//         }
//         if (p.stages.empty()) cout << "  (empty group)\n";
//     }
// }

// // --- Simple REPL that demonstrates parsing ---
// int main() {
//     cout << "Quote-aware parser demo. Type lines to parse, Ctrl-D to quit.\n";
//     string line;
//     while (true) {
//         cout << "$ ";
//         if (!std::getline(cin, line)) break;
//         if (line.empty()) continue;

//         auto pipelines = parse_line_into_pipelines(line.c_str());
//         debug_print(pipelines);
//     }
//     cout << "\nGoodbye.\n";
//     return 0;
// }


// Full C++ parser (quote-aware + strtok + ; | < > >> &)

// Below is a complete, compile-ready C++ program that implements your chosen approach (preprocess → strtok) and also preserves quoted substrings (single ' and double ").
// It:

// Splits the input line into command groups separated by ; (but not splitting ; inside quotes),

// Normalizes whitespace outside quotes,

// Inserts spaces around operators | < > >> & only when they are outside quotes,

// Uses strtok to split on whitespace (so quoted chunks remain single tokens),

// Builds Pipeline → Stage structures: argv, in_file (<), out_file (> or >>, with append flag), and background &.

// Save as parser.cpp and compile with g++ -std=c++17 parser.cpp -o parser.

// Step-by-step explanation (what the program does for each input line)

// Mask semicolons inside quotes

// We must split the input on ; into groups, but ; inside quotes must not split.

// The function split_groups_by_semicolon_quote_aware() scans the input, toggles in_single / in_double when it sees '/", and replaces any ; found while inside quotes with a placeholder character (\x1E). Then it calls strtok(buf, ";") to split by the real semicolons only. Each resulting group then has the placeholder restored back to ;.

// Result: a vector of semicolon-separated command groups (quotes preserved).

// Normalize whitespace outside quotes (preprocess_quotes)

// For each group we collapse runs of spaces/tabs outside quotes into a single space. Inside quotes we preserve whitespace exactly.

// We also keep the quote characters themselves in the output (we do not strip the quotes).

// This makes subsequent tokenization with strtok reliable: quotes remain one contiguous token.

// Insert spaces around operators, outside quotes (preprocess_ops_quote_aware)

// Walk the string again, toggling quote state. Only when not inside quotes do we insert spaces around |, <, >, and &.

// If we see a > followed immediately by another > (outside quotes), we output " >> " (so >> becomes one token).

// Operators that appear inside quotes are left untouched (they remain part of the quoted token).

// Tokenize on whitespace using strtok (tokenize_ws_strtok)

// Because steps 2–3 protected quoted substrings and made operators their own space-separated tokens (outside quotes), a single strtok pass on whitespace yields the token list we need.

// NOTES:

// Tokens keep their quotes (e.g. "hello world" stays as one token including the quotes).

// Empty tokens are ignored by strtok.

// Split tokens into pipeline stages (parse_group_into_pipeline)

// We walk tokens left-to-right:

// A | token ends the current stage and starts the next.

// A < token consumes the next token as in_file.

// A > or >> token consumes the next token as out_file and sets append for >>.

// A & token marks the entire pipeline background; we set background = true and stop parsing the rest of that group.

// Any other token is appended to argv for the current stage.

// If the same redirection appears multiple times, the last one wins (common shell behavior).

// Stages that are empty (no argv and no redirects) are dropped.

// Return a structured AST

// parse_line_into_pipelines() returns a vector<Pipeline>, where each Pipeline contains stages (in order) and a background flag.
// Notes / design decisions

// Quotes are preserved in tokens (assignment requirement). If you later execve() you may want to strip quotes at execution time — I left them intact because you explicitly asked quotes to remain part of the token.

// The implementation uses strtok exactly where required (splitting groups by ; and tokenizing on whitespace).

// The semicolon splitting is implemented in a quote-aware manner by masking ; inside quotes before calling strtok.

// >> detection is done in preprocess_ops_quote_aware while not inside quotes so it becomes a single token.

// & is accepted anywhere in a group and marks the entire pipeline background; we then ignore tokens after it (common shell behavior). If you prefer a stricter rule (only accept & as the last non-space token), that's easy to adjust.

// This parser intentionally does not execute commands (fork/exec/dup2). It gives you a clean AST that you can feed into an executor (fork, pipe, dup2, execvp, waitpid, etc.).

// If you want, I can:

// Add an executor (fork/pipes/dup2) that runs the parsed Pipelines (foreground/background),

// Or add optional quote-stripping so argv entries passed to exec don't include the coupling quotes,

// Or show unit tests / sample inputs and their exact debug_print outputs.









// #include <bits/stdc++.h>
// #include <readline/readline.h>
// #include <readline/history.h>
// #include <unistd.h>
// #include <pwd.h>
// #include <limits.h>
// #include <cstring>
// #include <errno.h>
// #include <dirent.h>
// #include <sys/stat.h>
// #include <grp.h>
// #include <ctime>

// using namespace std;

// #define GREEN "\033[1;92m"
// #define RED   "\033[1;91m"
// #define BLUE  "\033[1;94m"
// #define RESET "\033[0m"

// string homeDir;
// string prevDir;
// char *input;

// // ----------------- existing helpers -----------------
// void home(){
//     char temp[PATH_MAX];
//     if(!getcwd(temp,sizeof(temp))){
//         perror("error in getcwd");
//         exit(1);
//     }
//     homeDir=temp;
// }

// vector<string> getDetails(){
//     vector<string> details;
//     struct passwd *pwuid = getpwuid(getuid());
//     if(pwuid){
//         details.push_back(pwuid->pw_name);//username
//     }
//     details.push_back("@");
//     char hostname[HOST_NAME_MAX + 1];
//     if (gethostname(hostname, sizeof(hostname)) == 0) {
//         details.push_back(hostname);//hostname
//     }
//     return details;
// }
// string getDirAdd(){
//     string currentDIR ;
//     char path[PATH_MAX];
//     if(!getcwd(path,sizeof(path))){
//         perror("error in getcwd");
//         exit(1);
//     }
//     string pathAdd = path;
//     if(pathAdd.rfind(homeDir,0)==0){
//         if(pathAdd.size() != homeDir.size()){
//             currentDIR =  pathAdd.substr(homeDir.size());
//         }
//     }
//     else{
//         currentDIR = path;
//     }
//     return currentDIR;
// }

// void printDetailsTakeInput(){
//     vector<string> details;
//     details = getDetails();
//     string basePath = string(GREEN);
//     for(int i=0;i<3;i++){
//         basePath += details[i];
//     }
//     basePath +=string(RESET) + ":";
//     basePath+= string(RED) +"~"+getDirAdd() +string(RESET)+"> ";
//     input=  readline(basePath.c_str());
//     if(input && *input) add_history(input);
// }

// // ----------------- permission & ls helpers -----------------
// void getPermission(mode_t mode){
//     char type;
//     if(S_ISDIR(mode)) type='d';
//     else if(S_ISLNK(mode)) type='l';
//     else if(S_ISCHR(mode)) type='c';
//     else if(S_ISBLK(mode)) type='b';
//     else if(S_ISSOCK(mode)) type='s';
//     else if(S_ISFIFO(mode)) type='p';
//     else type='-';
//     cout<<string(BLUE)<<type;

//     cout<<((mode & S_IRUSR)?'r':'-');
//     cout<<((mode & S_IWUSR)?'w':'-');
//     cout<<((mode & S_IXUSR)?'x':'-');
//     cout<<((mode & S_IRGRP)?'r':'-');
//     cout<<((mode & S_IWGRP)?'w':'-');
//     cout<<((mode & S_IXGRP)?'x':'-');
//     cout<<((mode & S_IROTH)?'r':'-');
//     cout<<((mode & S_IWOTH)?'w':'-');
//     cout<<((mode & S_IXOTH)?'x':'-');
//     cout<<string(RESET);
// }

// void lsprint_longlist(string &lspath,string &entryName){
//     string path;
//     if(!entryName.empty() && entryName[0]=='/'){
//         path=entryName;
//     }
//     else{
//         path =lspath +"/" +entryName;
//     }
//     struct stat status;
//     if(lstat(path.c_str(), &status)<0){
//         perror("lstat");
//         return;
//     }
//     getPermission(status.st_mode);
//     cout<<" "<<status.st_nlink;
//     struct passwd *passWD = getpwuid(status.st_uid);
//     cout <<" "<<string(GREEN)<<(passWD?passWD->pw_name:"?")<<string(RESET);
//     struct group  *gr = getgrgid(status.st_gid);
//     cout <<" "<<string(GREEN)<<(gr?gr->gr_name:"?")<<string(RESET);
//     cout <<" "<<status.st_size;

//     char timebuf[64];
//     struct tm *tm = localtime(&status.st_mtime);
//     strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", tm);
//     cout <<" "<<timebuf;
//     cout <<" "<<entryName+"\n";
// }

// void lsloop(vector<string> &lspath , bool allhidden , bool longlist){
//     if(lspath.size()){
//         for(long long i=0;i<lspath.size();i++){
//             struct stat status; 
//             if(lstat(lspath[i].c_str(),&status)<0){
//                 perror("Stat not found");
//                 continue;
//             }
//             if(S_ISDIR(status.st_mode)){
//                 if(lspath.size()>1){
//                     cout<<lspath[i]<<":\n";
//                 }
//                 DIR *dirpath = opendir(lspath[i].c_str());
//                 if(!dirpath){
//                     perror(lspath[i].c_str());
//                     continue;
//                 }
//                 struct dirent *enteries;
//                 while ((enteries = readdir(dirpath)) != nullptr) {
//                     string entryName = enteries->d_name;
//                     if (!allhidden && entryName[0] == '.') 
//                         continue;
//                     if(longlist){
//                         lsprint_longlist(lspath[i],entryName);
//                     }
//                     else{
//                         cout<<entryName<<endl;
//                     }
//                 }
//                 closedir(dirpath);
//             }
//             else if(longlist){
//                 string temp = "";
//                 lsprint_longlist(temp,lspath[i]);
//             }
//             else{
//                 cout<<lspath[i]<<endl;
//             }
//         }
//     }
// }

// void ls(vector<string> &trimInput){
//     vector<string> lspath_FILE;
//     vector<string> lspath_DIR;//to print dir last  if given input
//     bool allhidden =false , longlist =false;
//     for(int i=1;i<trimInput.size();i++){
//         if(trimInput[i][0]=='-'){
//             for(int j=1;j<trimInput[i].size();j++){
//                 if(trimInput[i][j]=='a') allhidden=true;
//                 if(trimInput[i][j]=='l') longlist=true;
//             }
//         }
//         else if(trimInput[i] =="."){
//             char path[PATH_MAX];
//             if(!getcwd(path,sizeof(path))){
//                 perror("error in getcwd");
//                 exit(1);
//             }
//             string currentDIR = path;
//             lspath_DIR.push_back(currentDIR);
//         }
//         else if(trimInput[i][0] =='~'){
//             if(trimInput[i].size()>1){
//                 if(trimInput[i][1] =='/'){
//                     string temp=getenv("HOME");
//                     for(int k=1;k<trimInput[i].size();k++){
//                         temp+=trimInput[i][k];
//                     }
//                     struct stat status;
//                     if(lstat(trimInput[i].c_str(),&status)<0) {
//                         perror(trimInput[i].c_str());
//                         continue;
//                     }
//                     if(S_ISDIR(status.st_mode)){
//                         lspath_DIR.push_back(temp);
//                     }
//                     else{
//                         lspath_FILE.push_back(temp);
//                     }
//                 }
//             }else{
//                 lspath_DIR.push_back(getenv("HOME"));
//             }
//         }
//         else{
//             struct stat status;
//             if(lstat(trimInput[i].c_str(),&status)<0) {
//                 perror(trimInput[i].c_str());
//                 continue;
//             }
//             if(S_ISDIR(status.st_mode)){
//                 lspath_DIR.push_back(trimInput[i]);
//             }
//             else{
//                 lspath_FILE.push_back(trimInput[i]);
//             }
//         }
//     }
//     if(lspath_FILE.size()==0 && lspath_DIR.size()==0){
//         char path[PATH_MAX];
//             if(!getcwd(path,sizeof(path))){
//                 perror("error in getcwd");
//                 exit(1);
//             }
//             string currentDIR = path;
//             lspath_DIR.push_back(currentDIR);
//     }
//     if(lspath_FILE.size()!=0){
//         lsloop(lspath_FILE,allhidden,longlist);
//     }
//     if(lspath_FILE.size()!=0 && lspath_DIR.size()!=0){
//         cout<<endl;
//     }
//     if(lspath_DIR.size()!=0){
//         lsloop(lspath_DIR,allhidden,longlist);
//     }
// }

// // ----------------- pwd, cd, echo -----------------
// void pwd(){
//     char path[PATH_MAX];
//     if(!getcwd(path,sizeof(path))){
//         perror("error in getcwd");
//         exit(1);
//     }
//     string currentDIR = path;
//     cout<<currentDIR<<endl;
// }

// void cd(vector<string> &trimInput){
//     char path[PATH_MAX];
//     if(!getcwd(path,sizeof(path))){
//         perror("error in getcwd");
//         exit(1);
//     }
//     string currDir = path;
//     if(trimInput.size()>2){
//         cout<<"cd : Error Invalid arguements\n";
//         return;
//     }
//     if(trimInput.size()==1){
//         if(chdir(getenv("HOME"))!=0){
//             perror("chdir failed");
//         }
//     }
//     else if(trimInput[1]=="~"){
//         if(chdir(getenv("HOME"))!=0){
//             perror("chdir failed");
//         }
//     }
//     else if(trimInput[1]=="-"){
//         cout<<prevDir<<"\n";
//         if(chdir(prevDir.c_str())!=0){
//             perror("chdir failed");
//         }
//     }
//     else{
//         if(chdir(trimInput[1].c_str())!=0){
//             perror("chdir failed");
//         }
//     }
//     prevDir =currDir;
// }

// void echo(vector<string> &trimInput){
//     if(trimInput.size()==1){
//         cout<<endl;
//     }
//     else{
//         for(int i=1;i<trimInput.size();i++){
//             cout<<trimInput[i]<<" ";
//         }
//         cout<<endl;
//     }
// }

// // ----------------- NEW: quote-aware parsing helpers -----------------

// static inline string trim_ws(const string &s) {
//     size_t i = 0, j = s.size();
//     while (i < j && isspace(static_cast<unsigned char>(s[i]))) i++;
//     while (j > i && isspace(static_cast<unsigned char>(s[j-1]))) j--;
//     return s.substr(i, j - i);
// }

// // Replace semicolons inside quotes with a placeholder, then use strtok to split groups.
// vector<string> split_groups_by_semicolon_quote_aware(const char *line) {
//     vector<string> groups;
//     if (!line) return groups;

//     char *buf = strdup(line);
//     if (!buf) return groups;

//     const char PLACEHOLDER = '\x1E'; // unlikely char
//     bool in_single = false, in_double = false;
//     for (size_t i = 0; buf[i]; ++i) {
//         char c = buf[i];
//         if (c == '"' && !in_single) {
//             in_double = !in_double;
//         } else if (c == '\'' && !in_double) {
//             in_single = !in_single;
//         } else if (c == ';' && (in_single || in_double)) {
//             buf[i] = PLACEHOLDER;
//         }
//     }

//     char *tok = strtok(buf, ";");
//     while (tok) {
//         string s(tok);
//         replace(s.begin(), s.end(), PLACEHOLDER, ';');
//         s = trim_ws(s);
//         if (!s.empty()) groups.push_back(s);
//         tok = strtok(NULL, ";");
//     }

//     free(buf);
//     return groups;
// }

// // Collapse runs of spaces/tabs outside quotes into a single space, keep quotes and inner spaces.
// string preprocess_quotes(const string &in) {
//     string out;
//     out.reserve(in.size());
//     bool in_single = false, in_double = false;

//     for (size_t i = 0; i < in.size(); ++i) {
//         char c = in[i];

//         if (c == '"' && !in_single) {
//             in_double = !in_double;
//             out.push_back(c);
//         } else if (c == '\'' && !in_double) {
//             in_single = !in_single;
//             out.push_back(c);
//         } else if ((c == ' ' || c == '\t') && !in_single && !in_double) {
//             if (!out.empty() && out.back() != ' ')
//                 out.push_back(' ');
//         } else {
//             out.push_back(c);
//         }
//     }
//     return out;
// }

// // Insert spaces around operators outside quotes. Make ">>" one token ">>".
// string preprocess_ops_quote_aware(const string &in) {
//     string out;
//     out.reserve(in.size() * 2);

//     bool in_single = false, in_double = false;
//     for (size_t i = 0; i < in.size(); ++i) {
//         char c = in[i];

//         if (c == '"' && !in_single) {
//             in_double = !in_double;
//             out.push_back(c);
//             continue;
//         } else if (c == '\'' && !in_double) {
//             in_single = !in_single;
//             out.push_back(c);
//             continue;
//         }

//         if (!in_single && !in_double) {
//             if (c == '>') {
//                 if (i + 1 < in.size() && in[i+1] == '>') {
//                     out += " >> ";
//                     ++i;
//                 } else {
//                     out += " > ";
//                 }
//             } else if (c == '|' || c == '<' || c == '&') {
//                 out.push_back(' ');
//                 out.push_back(c);
//                 out.push_back(' ');
//             } else {
//                 out.push_back(c);
//             }
//         } else {
//             out.push_back(c);
//         }
//     }
//     return out;
// }

// // Tokenize on whitespace using strtok (so quoted segments remain single tokens)
// vector<string> tokenize_ws_strtok(const string &s) {
//     vector<string> tokens;
//     char *buf = strdup(s.c_str());
//     if (!buf) return tokens;
//     char *tok = strtok(buf, " \t\r\n");
//     while (tok) {
//         tokens.emplace_back(tok);
//         tok = strtok(NULL, " \t\r\n");
//     }
//     free(buf);
//     return tokens;
// }

// // New trim(): returns tokens similar to your previous trim(), but quote-aware & operator-aware.
// vector<string> trim(){
//     vector<string> trimInput;
//     if (!input) return trimInput;

//     // Step A: split by semicolon groups (we'll process only the first group here to maintain
//     // behavior similar to original trim: returning tokens for the single command typed).
//     // If you want to support ';' to run multiple groups, you'd loop over groups; for now,
//     // we produce tokens for the first group only (matching original single-command flow).
//     vector<string> groups = split_groups_by_semicolon_quote_aware(input);
//     if (groups.empty()) return trimInput;

//     string group = groups[0];

//     // 1) preserve quoted spacing, collapse other spaces
//     string quoted = preprocess_quotes(group);

//     // 2) spacing around operators outside quotes
//     string spaced = preprocess_ops_quote_aware(quoted);

//     // 3) tokenize with strtok on whitespace
//     vector<string> toks = tokenize_ws_strtok(spaced);

//     // Return tokens (quotes preserved)
//     return toks;
// }

// // ----------------- main -----------------
// int main(int argc ,char* argv[]){
//     home();
//     while(1){
//         printDetailsTakeInput();
//         vector<string> trimInput;
//         if(input){
//             trimInput= trim();
//         }
//         else{
//             continue;
//         }
//         if(trimInput.empty()) continue;

//         if(trimInput[0]=="pwd"){
//             pwd();
//         }
//         else if(trimInput[0]=="cd"){
//             cd(trimInput);
//         }
//         else if(trimInput[0]=="echo"){
//             echo(trimInput);
//         }
//         else if(trimInput[0]=="ls"){
//             ls(trimInput);
//         }
//         // future: parsed operators like |, <, >, >>, & are now tokenized properly and can be used
//         // by an executor you may add later (fork/dup2/execvp).
//     }
//     return 0;
// }



// #include <string>
// #include <vector>
// #include <cctype>
// using namespace std;

// vector<string> Normaltrim(const string &currInput) {
//     vector<string> tokens;
//     string current;
//     bool in_single = false, in_double = false;
//     bool prev_space = false;

//     for (size_t i = 0; i < currInput.size(); i++) {
//         unsigned char c = currInput[i];

//         if (c == '"' && !in_single) {
//             in_double = !in_double;
//             current.push_back(c);
//             prev_space = false;
//         }
//         else if (c == '\'' && !in_double) {
//             in_single = !in_single;
//             current.push_back(c);
//             prev_space = false;
//         }
//         else if (isspace(c) && !in_single && !in_double) {
//             // delimiter (space) found outside quotes
//             if (!current.empty()) {
//                 tokens.push_back(current);
//                 current.clear();
//             }
//             prev_space = true;
//         }
//         else {
//             current.push_back(c);
//             prev_space = false;
//         }
//     }

//     if (!current.empty()) {
//         tokens.push_back(current);
//     }

//     return tokens;
// }

// vector<string> Normaltrim(const string &currInput) {
//     vector<string> tokens;
//     string current;
//     bool in_single = false, in_double = false;
//     bool prev_space = true; // assume "space" before string starts

//     for (size_t i = 0; i < currInput.size(); i++) {
//         char c = currInput[i];

//         if (c == '"' && !in_single) {
//             in_double = !in_double;
//             current.push_back(c);
//             prev_space = false;
//         }
//         else if (c == '\'' && !in_double) {
//             in_single = !in_single;
//             current.push_back(c);
//             prev_space = false;
//         }
//         else if (isspace(c) && !in_single && !in_double) {
//             if (!prev_space && !current.empty()) {
//                 tokens.push_back(current);
//                 current.clear();
//             }
//             prev_space = true; // mark that we saw a space
//         }
//         else {
//             current.push_back(c);
//             prev_space = false;
//         }
//     }

//     if (!current.empty()) {
//         tokens.push_back(current);
//     }

//     return tokens;
// }

























#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <termios.h>

using namespace std;

// Track the foreground process group ID
pid_t fg_pgid = -1;

// =====================
// Signal Handlers
// =====================
void sigint_handler(int sig) {
    if (fg_pgid > 0) {
        // Forward Ctrl-C to the foreground process group
        kill(-fg_pgid, SIGINT);
    }
}

void sigtstp_handler(int sig) {
    if (fg_pgid > 0) {
        // Forward Ctrl-Z to the foreground process group
        kill(-fg_pgid, SIGTSTP);
    }
}

// =====================
// Utility: Split input into args
// =====================
vector<char*> parse_command(string &line) {
    stringstream ss(line);
    string word;
    vector<char*> args;

    while (ss >> word) {
        char *arg = new char[word.size() + 1];
        strcpy(arg, word.c_str());
        args.push_back(arg);
    }
    args.push_back(nullptr);
    return args;
}

// =====================
// Main Loop
// =====================
int main() {
    // Ignore signals in shell itself
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);

    while (true) {
        cout << "myshell> " << flush;

        string line;
        if (!getline(cin, line)) {
            // Ctrl-D -> EOF
            cout << "\nExiting shell...\n";
            break;
        }

        if (line.empty()) continue;

        // Parse input
        vector<char*> args = parse_command(line);
        if (!args[0]) continue;

        // Built-in: exit
        if (string(args[0]) == "exit") {
            cout << "Goodbye!\n";
            break;
        }

        // Fork and exec
        pid_t pid = fork();
        if (pid == 0) {
            // ---- Child ----
            setpgid(0, 0);                  // new process group
            tcsetpgrp(STDIN_FILENO, getpid()); // take terminal control

            // Restore default signal behavior in child
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            execvp(args[0], args.data());
            perror("execvp");
            exit(1);
        } else {
            // ---- Parent (Shell) ----
            fg_pgid = pid;
            setpgid(pid, pid);              // put child in new group
            tcsetpgrp(STDIN_FILENO, pid);   // give terminal to child

            int status;
            waitpid(pid, &status, WUNTRACED); // wait for finish/stop

            // After child finishes/stops, regain terminal
            tcsetpgrp(STDIN_FILENO, getpgrp());
            fg_pgid = -1;

            if (WIFSTOPPED(status)) {
                cout << "Process " << pid << " stopped\n";
                // Later: push to job list here
            }
        }

        // Cleanup args
        for (auto arg : args) delete[] arg;
    }

    return 0;
}






// === full file begins ===
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

// Added:
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <termios.h>

using namespace std;

#define GREEN "\033[1;92m"
#define RED   "\033[1;91m"
#define BLUE  "\033[1;94m"
#define RESET "\033[0m"

string homeDir;
string prevDir;
char *input;
vector<vector<string>> DoubleInput;

// --- job control globals ---
pid_t shell_pgid = 0;                 // shell's process group id
int shell_interactive = 0;            // are we attached to a TTY?
volatile sig_atomic_t fg_pid = -1;    // current foreground child pid (if any)

// Optional tiny BG list just to print when starting (you can expand later)
struct BgJob { pid_t pid; std::string cmd; };
std::vector<BgJob> bg_jobs;


void sigchld_handler(int) {
    int saved_errno = errno;
    while (true) {
        int status;
        pid_t pid = waitpid(-1, &status, WNOHANG);
        if (pid <= 0) break;

        char buf[128];
        int n;
        if (WIFEXITED(status)) {
            n = snprintf(buf, sizeof(buf), "process [%d] exited with status %d\n",
                         pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            n = snprintf(buf, sizeof(buf), "process [%d] killed by signal %d\n",
                         pid, WTERMSIG(status));
        } else {
            n = snprintf(buf, sizeof(buf), "process [%d] exited\n", pid);
        }
        if (n > 0) write(STDOUT_FILENO, buf, (size_t)n);
    }
    errno = saved_errno;
}

void init_shell_job_control() {
    shell_interactive = isatty(STDIN_FILENO);
    if (!shell_interactive) return;

    shell_pgid = getpid();
    if (setpgid(shell_pgid, shell_pgid) < 0) {
        perror("setpgid (shell)");
        exit(1);
    }

    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, nullptr) < 0) {
        perror("sigaction(SIGCHLD)");
        exit(1);
    }
}

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

void  multipleInputSplit(){
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

// --- helpers to identify built-ins and run external ---
bool is_internal_cmd(const std::string& cmd) {
    return (cmd == "pwd" || cmd == "cd" || cmd == "echo" || cmd == "ls");
}

void run_external(std::vector<std::string> args) {
    if (args.empty()) return;

    bool is_background = false;
    if (!args.empty() && args.back() == "&") {
        is_background = true;
        args.pop_back();
    }

    std::vector<char*> argv_exec;
    argv_exec.reserve(args.size() + 1);
    for (auto& s : args) argv_exec.push_back(const_cast<char*>(s.c_str()));
    argv_exec.push_back(nullptr);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }
    if (pid == 0) {
        if (setpgid(0, 0) < 0) {
            perror("setpgid (child)");
            _exit(1);
        }
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);

        execvp(argv_exec[0], argv_exec.data());
        fprintf(stderr, "%s: command not found or failed to exec\n", argv_exec[0]);
        _exit(127);
    } else {
        if (setpgid(pid, pid) < 0 && errno != EACCES) {
            perror("setpgid (parent)");
        }

        if (is_background) {
            printf("started [%d]\n", pid);
            bg_jobs.push_back({pid, args[0]});
            return;
        }

        fg_pid = pid;
        int status;
        while (true) {
            pid_t w = waitpid(pid, &status, WUNTRACED);
            if (w == -1) {
                if (errno == EINTR) continue;
                perror("waitpid");
                break;
            }
            if (WIFEXITED(status) || WIFSIGNALED(status)) {
                break;
            }
            if (WIFSTOPPED(status)) {
                printf("stopped [%d]\n", pid);
                bg_jobs.push_back({pid, args[0]});
                break;
            }
        }
        fg_pid = -1;
    }
}

void fixedinputloop(){
    for(size_t i=0;i<DoubleInput.size();i++){
        vector<string> trimInput;
        trimInput = DoubleInput[i];
        if(trimInput.empty()) continue;

        if(trimInput[0]=="pwd"){
            pwd();
        }
        else if(trimInput[0]=="cd"){
            cd(trimInput);
        }
        else if(trimInput[0]=="echo"){
            echo(trimInput);
        }
        else if(trimInput[0]=="ls"){
            ls(trimInput);
        }
        else{
            run_external(trimInput);
        }
    }
    DoubleInput.clear();
}

int main(int argc ,char* argv[]){
    home();
    init_shell_job_control(); // <<< added

    while(1){
        printDetailsTakeInput();
        if(input){
            multipleInputSplit();
        }
        else{
            continue;
        }
        fixedinputloop();
    }
    return 0;
}
// === full file ends ===
