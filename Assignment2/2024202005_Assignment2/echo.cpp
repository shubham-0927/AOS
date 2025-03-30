#include <sys/utsname.h>
#include<unistd.h>
#include <iostream>
#include <string.h>
#include "header.h"

void to_echo(string tok){
    // tok = strtok(NULL," ");
    string s(tok);
    int i = 0;
    while((int)s[i]==32 && i<s.length()) i++;
    if(s[i]==' '){
        perror("I think you have some space issue :)");
        perror("Try echo <statement>");
        return;
    }
    // cout<<"tok:"<<s;
    
    bool dic = false,ic = false;







    string t ="";
    // cout<<"s:"<<s<<"\n";
    while(i<s.length()){
        if((int)s[i]==34){
            i++;
            while(i<s.length() && s[i]!=34){
                // cout<<s[i];
                t+=s[i];
                i++;
            }
            if(i>=s.length()){
                cout<<" you have missed \" , i know it hurts ;_:";
                return;
            }
        }
        if(i<s.length()&&(int)s[i] == 39){
            i++;
            while(i<s.length()&& (int)s[i]!=39){
                // cout<<s[i];
                t+=s[i];
                i++;
            }
            if(i>=s.length()){
                cout<<" I think you have missed \' , I know it hurts ;_:";
                return;
            }
            
        }
        if(i<s.length()){
            // cout<<s[i];
            if((int)s[i]!=34 && (int)s[i]!=39)
            t+=s[i];
            if(i<=s.length() && (int)s[i]==32){
                while(i<s.length() && (int)s[i]==32) i++;
                i--;
            }
            
        } 
        i++;
    }
    cout<<t;
}