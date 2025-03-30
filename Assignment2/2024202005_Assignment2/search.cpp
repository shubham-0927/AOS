#include <unistd.h>
#include <iostream>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <vector>
#include <pwd.h>
#include <grp.h>
#include <ctime>

using namespace std;

void dir(){}

static int i = 0;
bool search( string currpath,string rt, string rem) {
    int w =0;
    while(rem[w]==' ')w++;
    string fl = rem.substr(w);
    // cout<<"path"<<currpath<<" fl"<<fl;
    char b[100];
    string path="";
    path.append(rt);
    // path.append("/");
    path.append(currpath);
    DIR* dir = opendir(currpath.c_str());
    if (dir == nullptr) {
        perror(("Cannot open directory:1 " + currpath).data());
        return false;
    }
    // cout<<"i "<<i++<<flush;
    struct dirent* q;
    while ((q = readdir(dir)) != nullptr) {

        if (string(q->d_name) == "." || string(q->d_name) == "..") {
            continue;
        }

        if (strcmp(q->d_name ,fl.data())==0) {
            closedir(dir);
            return true;
        }
        string fullPath = currpath + "/" + q->d_name;
        struct stat f_st;

        
        if (stat(fullPath.data(), &f_st) == 0 && f_st.st_mode&S_IFDIR) {


            if (search(fullPath,rt, fl)) {
                closedir(dir);
                return true;
            }
        }
    }

    closedir(dir);
    return false;
}
