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

char a[100];
string allcommand[] = {"cd", "echo", "handletab","ls","proid","search"};
vector<string> hist;
int h = 0;
string rt = getcwd(a, 100);
string currPath = "",prePath ="";
struct termios orig_termios;
char* usr_name;
string sys;
int f_pid = -1;  
bool z = false;
vector<int> forpid;
void handle_sigint(int sig) {
    if (f_pid != -1) {
        kill(f_pid, SIGINT); 
        z = true;
        
        f_pid = -1;  
    }
}

void handle_sigtstp(int sig) {
    if (f_pid != -1) {
        kill(f_pid, SIGTSTP);  
        cout << "\n[Process " << f_pid << " stopped]" << endl;
        forpid.push_back(f_pid);  
        f_pid = -1;  
    }
}
void handle_sigquit(int sig) {
    cout << "\nLogging you out..." << endl;
    exit; 
}
void enableRawMode(struct termios &orig_termios) {
    tcgetattr(0, &orig_termios); 
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO); 
    tcsetattr(0, TCSAFLUSH, &raw);
}

void disableRawMode(struct termios &orig_termios) {
    tcsetattr(0, TCSAFLUSH, &orig_termios); 
}

void print_name_path(string path){
    struct utsname usr;
    if (uname(&usr) != 0) {
        perror("uname");
        return;
    }
    usr_name = getlogin();
    if (usr_name == nullptr) {
        perror("getlogin");
        return;
    }
    sys =usr.sysname;
    cout<<"\n<"<<usr_name<<"@"<<sys<<":~"<<currPath<<">"<<flush;
} 





int main(){
    ios_base::sync_with_stdio(true);
    cin.tie(NULL);
    cout.tie(NULL);
    cout<<'\n';
    signal(SIGINT, handle_sigint);     // ctrl c
    signal(SIGTSTP, handle_sigtstp);   // ctrl z
    signal(SIGQUIT, handle_sigquit);    //ctrl d
    ///home/shubham/Documents/myFolder/IIIT/AOS/Posix/AOS_assignment_2_M24.pdf
    char* b = (char *)calloc(2048,sizeof(char));
    int fd = open("history.txt",O_RDONLY);
    if(fd != -1){
        string store_c;
        int sz = lseek(fd,0,SEEK_END);
        lseek(fd,0,SEEK_SET);
        int count = read(fd,b,sz);
        int t =0;
        while(t<sz){
            if(b[t] == '\n'){
                // cout<<store_c;
                hist.push_back(store_c);
                store_c.erase();
            }else store_c+=b[t];
            // cout<<b[t];
            t++;
        }
        close(fd);
    }
    else{
        fd = open("history.txt",O_CREAT,0646);
        write(fd," ",2);
    }
    // hist.push_back("");
    h = hist.size()-1;
    do{ 
        
        enableRawMode(orig_termios);
        char c;
        string query_in;
        print_name_path("~");
        // cin>>ws;
        //getline(cin,query_in);
        int ht = h;
        bool w = false;
        while (true) {
            if (read(0, &c, 1) == 1) {
                
                w = false;
                if (c == '\n') break;
                if (c == '\b' || c == 127) { 
                    if (!query_in.empty()) {
                        query_in.pop_back();
                        cout << "\b \b" << flush;
                    }
                } else if (c == '\t') {
                    if(!query_in.empty()){
                        tabCompletion(query_in);
                        query_in.erase();
                        break;
                    }
                    cout<<"    "<<flush;
                } else if (c == '\033') { 
                    char seq[2];
                    if (read(0, &seq[0], 1) == 1 && read(0, &seq[1], 1) == 1) {
                        if (seq[0] == '[') {
                            switch (seq[1]) {
                                case 'A':
                                    if(ht<0) ht = 0; 
                                    else
                                    if(ht>=0&&ht<hist.size()){
                                        int p = hist[ht].length();
                                        w = true;
                                        query_in = hist[ht];
                                        cout<<"\033[2K\r<"<<usr_name<<"@"<<sys<<":~"<<currPath<<">"<<hist[ht]<<flush;

                                        // write(1,hist[ht].data(),hist[ht].length());
                                        ht--;
                                    }
                                    
                                    // else ht = hist.size()-1;
                                    break;
                                    case 'B':
                                        // cout<<"\033[2K\r<"<<usr_name<<"@"<<sys<<":~"<<currPath<<">"<<flush;
                                        // query_in.erase(); 
                                        // ht = h;
                                        if(ht>=hist.size()) ht = hist.size()-1;
                                        else
                                        if(ht>=0&&ht<hist.size()){
                                        int p = hist[ht].length();
                                        // w = true;
                                        
                                        query_in = hist[ht];
                                        cout<<"\033[2K\r<"<<usr_name<<"@"<<sys<<":~"<<currPath<<">"<<hist[ht]<<flush;

                                        // write(1,hist[ht].data(),hist[ht].length());
                                        ht++;
                                    }
                                

                                    break;
                            }
                        }
                    }
                } else if (c == 4) {  
                        cout << "\nLogging out... :);" << endl;
                        chdir(rt.data());
                        string hist_file = rt;
                        hist_file.append("history.txt");
                        int fdw = open("history.txt",O_WRONLY |O_TRUNC);
                        if(fdw != -1){
                            int t = hist.size();
                            int k = 0;
                            while(k<t){
                                string store_c = hist[k]+'\n';
                                // cout<<store_c;
                                write(fdw,store_c.data(),store_c.length());
                                k++;
                            }
                    close(fdw);
                }
                        disableRawMode(orig_termios);
                        exit(0);
                    } else {
                        write(1,&c,1);
                        query_in+=c;
                }
            }
        }
        // cout<<query_in;
        disableRawMode(orig_termios);



        cout<<"\n";
        char* query= query_in.data();
        char* semi_tok = strtok(query,";"); 
        while(semi_tok != NULL){
            
            hist.push_back(semi_tok);
            h++;
            if(h>19){
                hist.erase(hist.begin());
                h--;
            }

            string st(semi_tok);
            int i =0,j=i;
            while((int)st[i]==32&&i<st.length())i++;
            if(i>=st.length()) {
                cout<<"YoU nEeD tO EnTeR sOmEtHiNg '-'";
                break;
            }
            j = i;
            while((int)st[j]!=32 && j<st.length()) j++;
            if(j>st.length()){
                cout<<"sorry..! Invalid command";
                return 0;
            }
            string q = st.substr(i,j-i);
            //char* tok = strtok(semi_tok," ");
            // while (tok != NULL) {
            // cout<<tok;
            // tok = strtok(NULL, " ");
            // }
            string rem = st.substr(j);
            bool rd = false,p = false;
            int temp = 0;
            while(temp<st.length()){
                if((st[temp] == '>' || st[temp]=='<')){
                    rd = true;
                }
                if(st[temp] == '|'){
                    p = true;
                }
                temp++;
            }
            if(p){
                forpipe(st);
                cout<<"\n";
            }else
            if(rd){
                redir(st);
                cout<<"\n";
            }else
            if(strcmp(q.data(),"cd")==0){

                


                string c = getcwd(a,100);
                string s = change_dir(rt,prePath,rem);
                //cout<<s<<"\n";
                if(s != ""){
                    prePath = c;
                    // cout<<getcwd(a,100)<<" this path";
                    currPath = getcwd(a,100);
                    string t = rt;
                    if(strcmp(rt.data(),currPath.data())==0)
                        currPath = "";
                    else if(t.length()<=currPath.length()){
                        currPath.erase(0,t.length());
                    }
                    if(strcmp(currPath.data(),"")!=0){
                        cout<<"\n"<<currPath;
                    }
                    if(currPath.length()==0){
                    cout<<"\n/"<<getlogin();
                    }
                }
                
            }else
            if(strcmp(q.data(),"pwd")==0){
                
                if(currPath.length()==0){
                    cout<<"/"<<getlogin();
                }
                else cout<<currPath;
            }else
            if(strcmp(q.data(),"echo")==0){
                to_echo(rem);
            }else
            if(strcmp(q.data(),"ls")==0){
                vector<string> vs = listf(rt,getcwd(a,100),rem);
            }else
            if(strcmp(q.data(),"search")==0){
                
                bool f = search(getcwd(a,100),rt,rem);
                if(f) cout<<"True";
                else cout<<"False";
            }
            else
            if(strcmp(q.data(),"history")==0){
                string vq = rem;
                int f;
                if(vq.length()>0)
                    f = stoi(vq);
                else f = 10;
                int t = hist.size()-f;
                    if(t<0) t =0;
                    // cout<<"t :"<<t;
                    while(t<hist.size()){
                        string store_c = hist[t]+'\n';
                        cout<<store_c;
                        // write(fdw,store_c.data(),store_c.length());
                        t++;
                    }


            }else
            if(strcmp(q.data(),"pinfo")==0){
                int i = rem.length()-1;
                while (i>0 && rem[i]==' ')
                {
                    i++;
                    rem.pop_back();
                }
                int pid;
                if(rem.length()<1) pid = getpid();
                else pid = stoi(rem);
                getprocinfo(pid,rt);
            }
            else
            if(strcmp(q.data(),"exit")==0){

                chdir(rt.data());
                string hist_file = rt;
                hist_file.append("history.txt");
                int fdw = open("history.txt",O_WRONLY |O_TRUNC);
                if(fdw != -1){
                    int t = hist.size();
                    int k = 0;
                    while(k<t){
                        string store_c = hist[k]+'\n';
                        // cout<<store_c;
                        write(fdw,store_c.data(),store_c.length());
                        k++;
                    }
                    cout<<"byeeee byeeeeeeeeeee :);\n";
                    close(fdw);
                }else{
                    cout<<"cant create history file";
                }
                return 0;
            }
            else{
                string check_q =semi_tok;
                int i = check_q.length()-1;
                while(i>0&&(int)check_q[i]==32){
                    i--;
                    check_q.pop_back();
                }
                bool asyncit = false; 
                if(check_q[i] == '&') {
                    asyncit = true;
                    check_q.pop_back();
                }
                vector<char*> q_tok;
                char* tok = strtok(&check_q[0], " ");
                while (tok != NULL) {
                    q_tok.push_back(tok);
                    tok = strtok(NULL, " ");
                }
                q_tok.push_back(NULL);  

                int pid = fork();
                if (pid == 0) {
                    
                    if (asyncit) {
                    
                    freopen("/dev/null", "w", stdout);
                    freopen("/dev/null", "w", stderr);
                    }
                    // setsid();

                    execvp(q_tok[0], q_tok.data());
                    perror("This command was executed by execvp .\nExecution failed because ");
                    
                    exit(0);
                } else if (pid > 0) {  // Parent process
                        f_pid = pid;
                        cout<<pid;
                        int status;

                        
                        if (!asyncit) {
                            // waitpid(pid, &status, WUNTRACED);  
                            // if (WIFSTOPPED(status)) {
                               
                            //     continue;
                            // }
                            // f_pid = -1;  

                            
                            while (true) {
                                waitpid(pid, &status, WUNTRACED | WCONTINUED);
                                if (WIFEXITED(status) || WIFSIGNALED(status)) {
                                    f_pid = -1;
                                    break;
                                } else if (WIFSTOPPED(status)) {
                                    // Handle the process stopping with CTRL-Z
                                    cout << pid;
                                    f_pid = -1;
                                    break;
                                }
                            }
                        } else {
                            cout << "\nPID: " << pid << " running in background.\n";
                            forpid.push_back(pid);
                        }
                    } else {
                    perror("Fork failed\n");
                }

                // cout<<"No cmd"<<q;
            }
            semi_tok = strtok(NULL,";");
        }
        

    }while(1);
   
    return 0;
}