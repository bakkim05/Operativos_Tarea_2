#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/reg.h>
#include <sys/syscall.h>

#define SYSCALL_BUFFER_SIZE 500
#define SYSCALL_NAME_BUFFER_SIZE 48

// returns -1 if not found and the index of the syscall if foind
int already_found(long syscall_num, long *syscalls_found, int found_len);
int getSyscallName(int sys_number, char *name);
void print_auto_results(long *syscalls, int *syscall_count, int len);

int main()
{
    pid_t child;
    long orig_eax, eax;
    long params[3];
    int status;
    int insyscall = 0;
    long syscalls[SYSCALL_BUFFER_SIZE];
    int syscall_count[SYSCALL_BUFFER_SIZE] = {0};
    int last_index = 0;
    int sc_index;
    char sc_name[SYSCALL_NAME_BUFFER_SIZE];

    child = fork();
    if (child == 0)
    {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execl("/bin/ls", "ls", NULL);
    }
    else
    {
        while (1)
        {
            wait(&status);
            if (WIFEXITED(status))
                break;
            orig_eax = ptrace(PTRACE_PEEKUSER,
                              child, 8 * ORIG_RAX, NULL);

            if (insyscall == 0)
            {
                /* Syscall entry */
                insyscall = 1;
                sc_index = already_found(orig_eax, syscalls, last_index);

                // If syscall hadn't been recorded
                if (sc_index == -1)
                {
                    syscalls[last_index] = orig_eax;
                    syscall_count[last_index] += 1;
                    last_index++;
                }
                // if already recorded
                else
                {
                    // update counter
                    syscall_count[last_index] += 1;
                }
            }
            else
            { /* Syscall exit */
                eax = ptrace(PTRACE_PEEKUSER,
                             child, 8 * RAX, NULL);
                printf("Write returned "
                       "with %ld\n",
                       eax);
                insyscall = 0;
            }

            ptrace(PTRACE_SYSCALL,
                   child, NULL, NULL);
        }
    }

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