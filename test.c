

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/reg.h>
//#include <sys/user.h>
#include <sys/syscall.h>

int main()
{   pid_t child;
    long orig_eax, eax;
    long params[3];
    int status;
    int insyscall = 0;
    child = fork();
    if(child == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execl("./hello", "hello", NULL);
    }
    else {
       while(1) {
          wait(&status);
          if(WIFEXITED(status))
              break;
          orig_eax = ptrace(PTRACE_PEEKUSER,
                     child, 8 * ORIG_RAX, NULL);
          if(orig_eax == SYS_write) {
             if(insyscall == 0) {
                /* Syscall entry */
                insyscall = 1;
                params[0] = ptrace(PTRACE_PEEKUSER,
                                   child, 8 * ORIG_RAX,
                                   NULL);
                printf("System Call is: %ld\n",
                       params[0]);
                }
          else { /* Syscall exit */
                eax = ptrace(PTRACE_PEEKUSER,
                             child, 8 * RAX, NULL);
                    printf("Write returned "
                           "with %ld\n", eax);
                    insyscall = 0;
                }
            }
            ptrace(PTRACE_SYSCALL,
                   child, NULL, NULL);
        }
    }
    return 0;
}