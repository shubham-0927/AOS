#include<unistd.h>
#include <iostream>
#include<string.h>
#include <fcntl.h>
#include "header.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <pwd.h>
#include <sstream>

using namespace std;

void getprocinfo(int pid,string rt){
    cout<<"pid -- "<<pid<<'\n';

    string filestat = "/proc/" + to_string(pid) + "/stat";
    int fd = open(filestat.data(), O_RDONLY);
    if (fd == -1) {
        cout << "Error: Could not open " << filestat;
        return;
    }
    
    char b[1024];
    int i = read(fd, b, sizeof(b) - 1);
    if (i == -1) {
        cout << "Error: Could not read " << filestat << '\n';
        close(fd);
        return;
    }
    
    b[i] = '\0';
    close(fd);
    /********************************************* */
    stringstream ss(b);
    string token;
    int count = 1;
    while (ss >> token) {
        if (count == 3) {
            bool isfpid;
            int pgid = getpgid(pid);

            int fd = open("/dev/tty", O_RDONLY);
            if (fd == -1) {
                isfpid = false; 
            }

            int tpid = tcgetpgrp(fd);
            close(fd);

            isfpid = (pgid == tpid);
            if (token == "R" || token == "S") {
                if (isfpid) {
                    token += "+";
                }
            }
            cout << "Process Status -- " << token << '\n';
            break;
        }
        count++;
    }

    string filestatm = "/proc/" + to_string(pid) + "/statm";
    fd = open(filestatm.data(), O_RDONLY);
    if (fd == -1) {
        cout << "Error: Could not open " << filestatm << '\n';
        cout<<"process not found";
        return;
    }

    i = read(fd, b, sizeof(b) - 1);
    if (i == -1) {
        cout << "Error: Could not read " << filestatm << '\n';
        close(fd);
        return;
    }

    b[i] = '\0';
    close(fd);
    
    stringstream streammm(b);
    streammm >> token;
    cout << "memory -- " << token << " {Virtual Memory}" << '\n';

    string exe_path = "/proc/" + to_string(pid) + "/exe";
    char path[PATH_MAX];
    long len = readlink(exe_path.data(), path, sizeof(path) - 1);
    if (len != -1) {
        path[len] = '\0';
        string cpath = path;
        string dispath = path;
        dispath.erase(0,rt.length());
        string ppath = "~";
        ppath.append(dispath);
        if(rt.length()<cpath.length())
        cout << "Executable Path -- " << ppath << '\n';
        else
        cout << "Executable Path -- " << path << '\n';
    } else {
        cout << "Error: Could not read executable path for PID ;_;" << pid << '\n';
    }
}