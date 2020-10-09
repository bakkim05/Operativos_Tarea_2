#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/reg.h>
#include <sys/syscall.h>
#include <string.h>
#include <stdio.h>

#define BUFFER_SIZE 200


int createCall(long syscallNumber, char* buffer){
    memset(buffer,0,BUFFER_SIZE);

    char number[30];

    strcat(buffer,"ausyscall ");
    sprintf(number,"%ld",syscallNumber);
    strcat(buffer, number);
    strcat(buffer, " x86_64 > temp.txt");

    return 0;
}

int getSyscallName(long syscallNumber, char*buffer){
    char temp_buffer[BUFFER_SIZE];
    createCall(syscallNumber,&temp_buffer);
    system(temp_buffer);

    FILE * temp_txt;
    temp_txt = fopen("temp.txt","r");
    if (temp_txt == NULL){
        return -1;
    }
    else{
        fgets(buffer,BUFFER_SIZE,temp_txt);
    }
    fclose(temp_txt);
    return 0;
}

int main()
{   
    pid_t child;
    long orig_eax, eax;
    long params[0];
    int status;
    int insyscall = 0;
    child = fork();

    char callBuffer[BUFFER_SIZE];

    if(child == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execl("./hello", "hello", NULL);
    }
    else {
    while(1) {
        wait(&status);
        if(WIFEXITED(status))
            break;
        orig_eax = ptrace(PTRACE_PEEKUSER, child, 8 * ORIG_RAX, NULL);
        if(insyscall == 0) {
            /* Syscall entry */
            insyscall = 1;
            params[0] = ptrace(PTRACE_PEEKUSER, child, 8 * ORIG_RAX, NULL);
            getSyscallName(params[0],&callBuffer);
            printf("Syscall made: %s", callBuffer);
        }
        else { /* Syscall exit */
            insyscall = 0;
        }
        ptrace(PTRACE_SYSCALL, child, NULL, NULL);
    }
}
return 0;
}