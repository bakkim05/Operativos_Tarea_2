#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/reg.h>
#include <sys/syscall.h>
#include <string.h>
#include <stdio.h>

#define BUFFER_SIZE 200
#define PROGRAM_SIZE 100


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

void interactive(char* executable){
    pid_t child;
    long orig_eax, eax;
    long params[0];
    int status;
    int insyscall = 0;
    child = fork();

    char callBuffer[BUFFER_SIZE];

    if(child == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execl(executable, "interactive", NULL);
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
}

int main(int argc, char *argv[])
{   
    char program[PROGRAM_SIZE];
    memset(program,0,PROGRAM_SIZE);
    for (int i = 2; i < argc ; i++){
        if (i == argc-1){
            strcat(program, argv[i]);
            strcat(program,"\0");
        }else{
            strcat(program, argv[i]);
            strcat(program, " ");
        }
    }
    
    if (strcmp(argv[1],"interactivo") == 0)
    {
        printf("[MODO INTERACTIVO]\n");
        interactive(program);
    }
    
    else if (strcmp(argv[1],"automatico") == 0)
    {
        printf("[MODO AUTOMATICO]\n");
        interactive(program);
    }
    else
    {
        printf("El modo seleccionado no existe\n");
    }
    

    return 0;
}