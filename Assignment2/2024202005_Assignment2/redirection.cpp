#include <sys/utsname.h>
#include<unistd.h>
#include <iostream>
#include<stack>
#include<string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "header.h"
#include <termios.h>
#include <vector>
using namespace std;
void redir(string semi_tok) {
    vector<string> args;
    string in_f, on_f;
    int app = 0;
    string tk;
    int i = 0;

    while (i < semi_tok.length()) {
        
        while (i < semi_tok.length() && isspace(semi_tok[i])) {
            ++i;
        }

        if (i >= semi_tok.length()) break;  

        
        if (semi_tok[i] == '<') {
            ++i;
           






            while (i < semi_tok.length() && isspace(semi_tok[i])) {
                ++i;
            }
           
            while (i < semi_tok.length() && !isspace(semi_tok[i]) && semi_tok[i] != '|' && semi_tok[i] != '>') {
                in_f += semi_tok[i++];
            }
        } else if (semi_tok[i] == '>') {
            ++i;
           
            if (i < semi_tok.length() && semi_tok[i] == '>') {
                app = 1;
                ++i;
            } else {
                app = 0;
            }
            
            while (i < semi_tok.length() && isspace(semi_tok[i])) {
                ++i;
            }
            
            while (i < semi_tok.length() && !isspace(semi_tok[i]) && semi_tok[i] != '|' && semi_tok[i] != '<') {
                on_f += semi_tok[i++];
            }
        } else {
        
            tk.clear();
            while (i < semi_tok.length() && !isspace(semi_tok[i]) && semi_tok[i] != '|' && semi_tok[i] != '<' && semi_tok[i] != '>') {
                tk += semi_tok[i++];
            }
            args.push_back(tk);
        }
    }

    
    vector<char*> vcmds;
    for (auto& arg : args) {
        vcmds.push_back(const_cast<char*>(arg.c_str()));
    }
    vcmds.push_back(nullptr);

    
    pid_t pid = fork();
    if (pid == 0) {
        
        
        if (!in_f.empty()) {
            int fd_in = open(in_f.c_str(), O_RDONLY);
            if (fd_in < 0) {
                perror("open");
                exit(1);
            }
            dup2(fd_in, 0); // for stdin
            close(fd_in);
        }

        
        if (!on_f.empty()) {
            int fd_out;
            if (app) {
                fd_out = open(on_f.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
            } else {
                fd_out = open(on_f.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            }
            if (fd_out < 0) {
                perror("open");
                exit(1);
            }
            dup2(fd_out, 1);// for stdout 
            close(fd_out);
        }

        
        if (execvp(vcmds[0], vcmds.data()) == -1) {
            perror("execvp");
            exit(1);
        }
    } else if (pid > 0) {
        
        waitpid(pid, nullptr, 0);
    } else {
        perror("fork");
        exit(1);
    }
}
