#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/reg.h>
#include <sys/syscall.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define BUFFER_SIZE 200
#define PROGRAM_SIZE 100
#define SYSCALL_BUFFER_SIZE 500
#define SYSCALL_NAME_BUFFER_SIZE 48

int createCall(long syscallNumber, char *buffer);
int getSyscallName(long syscallNumber, char *buffer);
// returns -1 if not found and the index of the syscall if foind
int already_found(long syscall_num, long *syscalls_found, int found_len);
void print_auto_results(long *syscalls, int *syscall_count, int len);

int main(int argc, char *argv[])
{
    char program_name[PROGRAM_SIZE];
    int is_interactivo = 0;
    int is_automatico = 0;

    memset(program_name, 0, PROGRAM_SIZE);

    // if less than 3 arguments not valid input
    if (argc < 3)
    {
        printf("Error: No hay suficientes argumentos");
        return 0;
    }

    if (strcmp(argv[1], "interactivo") == 0)
        is_interactivo = 1;
    else if (strcmp(argv[1], "automatico") == 0)
        is_automatico = 1;

    if (!(is_interactivo || is_automatico))
    {
        printf("El modo seleccionado no existe\n");
        return 0;
    }

    else
    {

        pid_t child;
        child = fork();

        // child process
        if (child == 0)
        {

            ptrace(PTRACE_TRACEME, 0, NULL, NULL);
            int ret;
            if (argc == 3)
                ret = execl(argv[2], "", NULL);
            else
                ret = execv(argv[2], &argv[3]);

            if (ret < 0)
            {
                // this code should only run if execv fails, since it only returns in case of failure
                fprintf(stderr, "Error trying to run program %s  with arguments %s  --> Error Message: %s\n", argv[2], argv[3], strerror(errno));
                exit(EXIT_FAILURE);
            }

        } // child process end

        //parent process
        else
        {
            // long orig_eax, eax;
            char callBuffer[BUFFER_SIZE];
            long syscall_num;
            int status;
            int insyscall = 0;
            long syscalls[SYSCALL_BUFFER_SIZE];
            int syscall_count[SYSCALL_BUFFER_SIZE] = {0};
            int last_index = 0;
            int sc_index;
            char sc_name[SYSCALL_NAME_BUFFER_SIZE];

            while (1)
            {
                wait(&status);
                if (WIFEXITED(status))
                    break;
                // orig_eax = ptrace(PTRACE_PEEKUSER, child, 8 * ORIG_RAX, NULL);
                if (insyscall == 0)
                {
                    /* Syscall entry */
                    insyscall = 1;
                    syscall_num = ptrace(PTRACE_PEEKUSER, child, 8 * ORIG_RAX, NULL);

                    if (is_interactivo)
                    {
                        getSyscallName(syscall_num, callBuffer);
                        printf("Syscall made: %s", callBuffer); /* code */
                    }
                    if (is_automatico)
                    {

                        sc_index = already_found(syscall_num, syscalls, last_index);
                        // If syscall hadn't been recorded
                        if (sc_index == -1)
                        {
                            syscalls[last_index] = syscall_num;
                            syscall_count[last_index] += 1;
                            last_index++;
                        }
                        // if already recorded
                        else
                        {
                            // update counter
                            syscall_count[sc_index] += 1;
                        }
                    }
                }
                else
                { /* Syscall exit */
                    // eax = ptrace(PTRACE_PEEKUSER,
                    //              child, 8 * RAX, NULL);
                    // printf("Write returned "
                    //        "with %ld\n",
                    //        eax);
                    insyscall = 0;
                }

                //continue child run
                ptrace(PTRACE_SYSCALL,
                       child, NULL, NULL);
            } // end while 1

            if (is_automatico)
            {
                print_auto_results(syscalls, syscall_count, last_index);
            }

        } // parent process
        return 0;
    }
}

// genertaes ausyscall to get name of syscall by number
int createCall(long syscallNumber, char *buffer)
{
    memset(buffer, 0, BUFFER_SIZE);

    char number[30];

    strcat(buffer, "ausyscall ");
    sprintf(number, "%ld", syscallNumber);
    strcat(buffer, number);
    strcat(buffer, " x86_64 > temp.txt");

    return 0;
}

// return the name of the syscall in buffer according to the number of the syscall in x86_64
int getSyscallName(long syscallNumber, char *buffer)
{

    char temp_buffer[BUFFER_SIZE];
    createCall(syscallNumber, temp_buffer);
    system(temp_buffer);

    FILE *temp_txt;
    temp_txt = fopen("temp.txt", "r");
    if (temp_txt == NULL)
    {
        return -1;
    }
    else
    {
        fgets(buffer, BUFFER_SIZE, temp_txt);
    }
    fclose(temp_txt);
    return 0;
}

int already_found(long syscall_num, long *syscalls_found, int found_len)
{

    for (size_t i = 0; i < found_len; i++)
    {
        if (syscalls_found[i] == syscall_num)
        {
            // found
            return i;
        }
    }
    //not found
    return -1;
}

void print_auto_results(long *syscalls, int *syscall_counts, int len)
{
    char callBuffer[SYSCALL_NAME_BUFFER_SIZE];
    int total_syscall = 0;
    printf("------------------------------------------\n");
    printf("| %20s | Number |  Count |\n", "System Calls");
    printf("------------------------------------------\n");
    for (size_t i = 0; i < len; i++)
    {
        memset(callBuffer, 0, SYSCALL_NAME_BUFFER_SIZE);

        getSyscallName(syscalls[i], callBuffer);
        callBuffer[strcspn(callBuffer, "\n")] = 0;
        printf("| %20s | %6ld | %6d |\n", callBuffer, syscalls[i], syscall_counts[i]);
        total_syscall += syscall_counts[i];
    }
    printf("------------------------------------------\n");
    printf("Total System Calls: %d\n", total_syscall);
}