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

// Function to print file permissions
void print_perm(mode_t mode) {
    if (mode & S_IFDIR) cout << 'd';
    else cout << '-';
    if (mode & S_IRUSR) cout << 'r';
    else cout << '-';
    if (mode & S_IWUSR) cout << 'w';
    else cout << '-';
    if (mode & S_IXUSR) cout << 'x';
    else cout << '-';
    if (mode & S_IRGRP) cout << 'r';
    else cout << '-';
    if (mode & S_IWGRP) cout << 'w';
    else cout << '-';
    if (mode & S_IXGRP) cout << 'x';
    else cout << '-';
    if (mode & S_IROTH) cout << 'r';
    else cout << '-';
    if (mode & S_IWOTH) cout << 'w';
    else cout << '-';
    if (mode & S_IXOTH) cout << 'x';
    else cout << '-';
}

bool isfile(string path) {
    struct stat path_stat;
    stat(path.c_str(), &path_stat);
    return S_ISREG(path_stat.st_mode);
}

bool isabs(string path, string rt) {
    string h = path.substr(0, rt.length());
    return !path.empty() && (strcmp(h.data(), rt.data()) == 0);
}

// Function to print detailed file information
void print_details_la(struct stat& filest, const string& fn) {
    print_perm(filest.st_mode);
    struct passwd* pw = getpwuid(filest.st_uid);
    struct group* gr = getgrgid(filest.st_gid);
    cout << "  " << filest.st_nlink << " " << (pw ? pw->pw_name : "???")
         << " " << (gr ? gr->gr_name : "???") << " " << filest.st_size << " ";
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%b %d %H:%M", localtime(&filest.st_mtime));
    cout << buffer << " " << fn << '\n';
}

vector<string> listf(string rt, string curr_path, string rem) {
    bool l = false, a = false;
    vector<string> vf;
    vector<string> directories;

    vector<string> args;
    int pos = 0, found;
    while ((found = rem.find(' ', pos)) != string::npos) {
        args.push_back(rem.substr(pos, found - pos));
        pos = found + 1;
    }
    args.push_back(rem.substr(pos));

    for (const string& arg : args) {
        if (arg == "-l") {
            l = true;
        } else if (arg == "-a") {
            a = true;
        } else if (arg == "-la" || arg == "-al") {
            l = true;
            a = true;
        } else if (arg == "~") {
            directories.push_back(".");
        } else {
            directories.push_back(arg);
        }
    }

    if (directories.empty()) {
        directories.push_back(curr_path);
    }

    bool f = false;
    if (directories.size() > 1) f = true;

    for (const string& dir_name : directories) {
        if (f) {
            f = false;
            continue;
        }

        string path;
        if (isabs(dir_name, rt)) {
            path = dir_name;
        } else {
            path = curr_path + "/" + dir_name;
        }
        cout << "" << dir_name << '\n';

        long w = 0;
        DIR* dir = opendir(path.c_str());
        if (dir) {
            struct dirent* ent;

            while ((ent = readdir(dir)) != NULL) {
                if (!a && ent->d_name[0] == '.') {
                    continue;
                }

                struct stat file_stat;
                string full_path = path + "/" + ent->d_name;

                if (stat(full_path.c_str(), &file_stat) == 0) {
                    w += file_stat.st_blocks;
                } else {
                    perror(("stat failed on " + full_path).c_str());
                }
            }

            if (l) {
                cout << "total " << w / 2 << '\n'; 
            }

            rewinddir(dir); 
            while ((ent = readdir(dir)) != NULL) {
                if (!a && ent->d_name[0] == '.') {
                    continue;
                }

                struct stat file_stat;
                string full_path = path + "/" + ent->d_name;

                if (stat(full_path.c_str(), &file_stat) == 0) {
                    if (l) {
                        print_details_la(file_stat, ent->d_name);
                    } else {
                        cout << ent->d_name << "  ";
                    }
                }
            }

            closedir(dir);
            if (!l) {
                cout << '\n';
            }
        } else {
            perror(("I was looking for " + dir_name + " but @_@").c_str());
        }
    }

    return vf;
}
