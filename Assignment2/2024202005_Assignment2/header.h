
#ifndef HEADER_H
#define HEADER_H

extern std::string allcommand[];
#include <iostream>
#include <string>
#include <vector>
using namespace std;
string change_dir(string,string,string);
void print_name_path(string);

//echo
void to_echo(string);

//ls
vector<string> listf(string,string, string );

//search
bool search(string ,string, string);

//process
void getprocinfo(int, string);

void tabCompletion(string);

void redir(string);

void forpipe(string);


#endif