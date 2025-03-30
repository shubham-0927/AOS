#include <iostream>
#include <vector>
#include <dirent.h>
#include <algorithm>
#include <cstring>
#include <unistd.h>
#include "header.h"
#include <sys/types.h>

using namespace std;
vector<string> cmds = {"ls", "cat", "cd","search","pwd", "echo", "exit","pinto","vim","gedit","history"};

vector<string> get_file_completions(const string& instring) {
    vector<string> v;
    string path = "./";
    string prefix = instring;

    int lastSlash = instring.find_last_of('/');
    if (lastSlash != string::npos) {
        path = instring.substr(0, lastSlash + 1);
        prefix = instring.substr(lastSlash + 1);
    }






    DIR* dir = opendir(path.c_str());
    if (!dir) return v;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        string name = entry->d_name;
        if (name.find(prefix) == 0) {
            v.push_back(name);
        }
    }
    closedir(dir);
    return v;
}

vector<string> get_command_completions(const string& instring) {
    vector<string> v;
    for (const string& cmd : cmds) {
        if (cmd.find(instring) == 0) {
            v.push_back(cmd);
        }
    }
    return v;
}
void tabCompletion(string instring) {
    vector<string> v;
    string vpaths;
    if (instring.empty() || instring.find(' ') == string::npos) {
        v = get_command_completions(instring); 
    } else {
        int sp = instring.find_last_of(' ');
        string path = instring.substr(sp + 1); 
        v = get_file_completions(path); 
    }

    if (v.size() == 1) {
        int sp = instring.find_last_of(' ');
        if (sp != string::npos) {
            instring = instring.substr(0, sp + 1) + v[0];
        } else {
            instring = v[0];
        }
        // print_name_path(vpaths);
        cout <<"\n" << instring;
    } else if (v.size() > 1) {
        cout << "\n";
        for (const string& option : v) {
            cout << option << "  ";
        }
        cout << "\n";

        
    }
}

