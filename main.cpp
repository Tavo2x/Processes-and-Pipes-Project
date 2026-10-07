#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

using namespace std;

const int WRITE_END = 1; // writing side of pipe 
const int READ_END = 0; // reading side of pipe
const int codeSize = 64; // size of every message in the pipe

// Returns true if the codes are the same or 1 letter apart
bool CoP(string code1, string code2){
    bool retVal = false;
    int counter = 0;
    // different lengths pass
    if(code1.length() != code2.length()){
        return false;
    }
    else{
        // looping to count the number characters that are diffrent 
        for(size_t i =0; i < code1.length(); i++){
            if(code1[i] != code2[i]){
                counter++;
            }
        }
    }
    // 0 or 1 difference means too close
    if(counter == 0 || counter == 1){
        retVal = true;
    }
    else{retVal = false;}
    return retVal;
}
// Method inteded to hold the code 
int filter(int fd, string code){
    int count = 1; // counts filter
    char buffer[codeSize];
    int OUT = -1; // num tied to piping into next filter
    pid_t cPid = -1; // next filters process id
    int val = read(fd,buffer,codeSize);
        while (val > 0)
        {
            string txt = buffer;
            bool retVal = CoP(code,txt);
            // if the cod is to close, discard it
            if(retVal == 1 ){}
            else{
                // if there have been no filter yet creats one
                if(OUT == -1){
                    int nPipe [2];
                    int result = pipe(nPipe);
                    if(result < 0){
                        exit(1);
                    }
                    cout.flush();
                    cPid = fork();
                    if(cPid < 0 ){
                        exit(1);
                    }
                    // new filter process
                    else if(cPid == 0){
                        close(nPipe[WRITE_END]); // reading only
                        close(fd); // old pipe not needed
                        int cCount = filter(nPipe[READ_END],txt);
                        exit(cCount); // count sent 
                    }
                    close(nPipe[READ_END]);
                    OUT = nPipe[WRITE_END];
                }
                // 
                else{
                    write(OUT,buffer,codeSize);
                }
            }
            val = read(fd,buffer,codeSize);
        }
        // if there is no more input we close 
        if(OUT != -1){
            close(OUT);    
        }
        cout << "Process: " << getpid() <<  " my code is " << "\"" << code << "\"" << endl;
        close(fd);
        if(cPid != -1){
            int status;
            wait(&status);
            count += WEXITSTATUS(status);
        }
    return count;
}
int main(int argc, char* argv[]) 
{    
    // need at least one code to fall into the else block
    if (argc < 2){
        cout << "Error pass in values as such: ./xxx CODE1 CODE2 ..." << endl;
        return 1;
    }
    else{
        cout << "Finding maximally distinct codes..." << endl;
        int parentToChild[2];
        int pipeResults = pipe(parentToChild);
        
        if(pipeResults < 0 ){
            cout << "Unable to creat pipe" << endl;
            return 1;
        }
        pid_t pid = fork();
 
        if(pid < 0){
            cout << "Unable to creat fork" << endl;
            return 1;
        }
        // childs is the first filters, holding the frist code
        if(pid == 0){
            close(parentToChild[WRITE_END]);
            int total = filter(parentToChild[READ_END], argv[1]);
            return total;
        }
        // parrents contineus the porcess for the rest of the codes
        else{
            close(parentToChild[READ_END]);
            char msg[codeSize];
            for(int i = 2; i < argc; i++){
                memset(msg,0,codeSize); // clearing old char's
                strncpy(msg, argv[i],codeSize-1); // replacing old code w new
                write(parentToChild[WRITE_END],msg,codeSize);
            }
            close(parentToChild[WRITE_END]);
            int status;
            wait(&status);
            int dif = WEXITSTATUS(status);
            cout << dif << " distinct codes found (no two are one edit apart)" << endl;
        }    
    }
    return 0;
}
