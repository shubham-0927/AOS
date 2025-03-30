#include <sys/utsname.h>
#include<unistd.h>
#include <iostream>
#include <string.h>
#include "header.h"
using namespace std; 

string change_dir(string rt,string prev,string rem){
    //tok = strtok(NULL, " ");
    // char* tok = strtok(rem.data()," ");
    // tok = strtok(NULL," ");
    // cout<<"tok"<<tok;
    int i =0;
    int j = rem.length()-1;
    while((int)rem[i]==32&&i<rem.length()) i++;
    while((int)rem[j]==32&&j>i){
        rem.erase(j);
        j--;
    }
    if(i>=rem.length()){
        perror("\nTake you to the root ;)");
        //cout<<rt;
        if (chdir(rt.data()) != 0) {
            perror("\nchdir() to root failed");
        }
        return rt;
    }
    string path = rem.substr(i);
    //tok = strtok(NULL, " ");
    bool s = false;
    
    int k =0;
    while(k<path.length()&& (int)path[k]!=32){
        k++;
    }
    
    if(k<path.length()){
        perror("\nToo many argument, have patience :) no");
        cout<<prev;
        return "";
    }
    
    /**************************/
    if(path=="-"){
        if (chdir(prev.data()) != 0){
            perror("\nGiven path failed ");
        }else{
            return path;
        }
    }
    if(path == "."){
        return "something";
    }
    if(path=="~"){
        perror("\nTake you to the root ;)");
        if (chdir(rt.data()) != 0) {
            perror("\nchdir() to root failed");
        }
        return rt; 
    }
    if (chdir(path.data()) != 0){
        perror("\n I was serching for ur input file or directory whatever but(");
    }else{
        if(path==".."){
            return "something";
        }
        return path;
    }
     
    return "";
}