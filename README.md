# Advanced Operating Systems – Assignment 2

## Author
**Roll Number:** *2025202018*  
**Course:** CS3.304 – Advanced Operating Systems  

---

## File Structure
2025202018_Assignment2  
│__ README.md  
│__ makefile  
│__ main.cpp             
│__ essentials.h          
│__ utilities.h          
│__ cd.h                 
│__ historyAutoC.h       
│__ pinfoSearch.h  
│__ redirectionPipeline.h

Implementation
--------------

1. Shell Prompt
- Displays dynamically as <user>@<system>:<cwd>.
- Handles ~, ., .., - directories.

2. Built-in Commands
- cd: Supports ~, ., .., -. Error on multiple args.
- pwd: Prints current working directory.
- echo: Prints arguments with space/tab handling (no escape/quotes parsing).
- ls: Supports -a, -l, combinations, multiple paths.

3. System Commands
- Any other command runs in foreground or background (&).
- Background process prints its PID.

4. Process Info (pinfo)
- Prints status, memory usage, and executable path.
- Works for current shell or a specific PID.

5. Search
- Recursively searches for a file/folder from current directory.

6. Redirection
- <: input redirection.
- >: overwrite output redirection.
- >>: append output redirection.

7. Pipelines
- Any number of piped commands supported (cmd1 | cmd2 | cmd3 ...).
- Supports mixed redirection + pipeline.

8. Signals
- Ctrl-C: Interrupt foreground process.
- Ctrl-Z: Push foreground to background (stopped).
- Ctrl-D: Exit shell.

9. Autocomplete
- TAB: Suggests/matches files/directories or commands.

10. History
- Persistent across sessions.
- Stores up to 20 commands (prints last 10).
- history <num> prints last <num> commands.
- UP/DOWN arrows navigate command history.



Example Input
-------------
```bash
ls -a -l ; cd test
echo Hello World
gedit &
pinfo
pinfo 1234
search file.txt
cat < input.txt | sort > out.txt
```

Example Output
--------------
```bash
user@UBUNTU:~> ls -a -l ; cd test
total 12
drwxr-xr-x 2 user user 4096 Sep  2 10:00 .
drwxr-xr-x 4 user user 4096 Sep  2 09:59 ..
-rw-r--r-- 1 user user   10 Sep  2 09:58 a.txt

user@UBUNTU:~/test> echo Hello World
Hello World

user@UBUNTU:~/test> gedit &
[1] 12345   # PID printed

user@UBUNTU:~/test> pinfo
pid -- 5678
Process Status -- S+
memory -- 123456
Executable Path -- ~/a.out
```

Compilation
-----------
Compile with the provided makefile:
make

This generates an executable a.out.


Execution
---------
Run the shell:
```bash
./myshell
```

Exit using Ctrl-D or exit.


Example Runs
------------
### Multiple commands
```bash
ls -al ; echo Done
```

### Foreground vs Background
```bash
gedit
gedit &
```

### Pipelines + Redirection
```bash
cat input.txt | grep "main" > result.txt
```

### Process info
```bash
pinfo
pinfo 7777
```

### Search
```bash
search notes.txt
```

### History
```bash
history
history 5
```


Important Notes
---------------
- No use of system(), popen(), pclose().
- Signals, pipes, redirection handled with low-level syscalls (fork, execvp, wait, dup2, etc.).
- Supports random spaces/tabs in input.
- Persistent history across sessions.
- Strictly modular design (functions split across headers).

