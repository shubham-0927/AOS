#include <sys/utsname.h>
#include <unistd.h>
#include <iostream>
#include <stack>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <vector>
#include "header.h"

using namespace std;

void runpipe(vector<string>& commands) {
    int num_cmds = commands.size();
    int pipefds[2*num_cmds];

    for(int i = 0; i < num_cmds - 1; ++i) {
        if(pipe(pipefds + i*2) == -1) {
            perror("I could create pipe");
            exit(1);
        }
    }

    int pid;
    for(int i = 0; i < num_cmds; ++i) {
        pid = fork();
        if(pid == 0) {
        



            if(i < num_cmds - 1) {
                dup2(pipefds[i*2 + 1], 1);
            }
            
            

            if(i != 0) {
                dup2(pipefds[(i-1)*2], 0);
            }

            int nc = 2*num_cmds;
            for(int j = 0; j < nc; ++j) {
                close(pipefds[j]);
            }

            
            redir(commands[i]);
            exit(1);
        }
    }

    
    for(int i = 0; i < 2*num_cmds; ++i) {
        close(pipefds[i]);
    }

    
    for(int i = 0; i < num_cmds; ++i) {
        wait(nullptr);
    }
}

void forpipe(string semi_tok) {
    vector<string> commands;
    long p = 0;
    string token;
    //int t = 0;
    //while(semi_tok[t]!='|'){
    //t++;
    //}
    //p = t;
    while ((p = semi_tok.find('|')) != string::npos) {
        token = semi_tok.substr(0, p);
        commands.push_back(token);
        semi_tok.erase(0, p + 1);
    }
    commands.push_back(semi_tok);
    runpipe(commands);
}
