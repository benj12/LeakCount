/*******************************************************************************
** sctracer.c                                                                 **
** Miles Bagwell                                                              **
** leonb                                                                      **
** ECE 322, Spring 2015                                                       **
** Project #1                                                                 **
**                                                                            **
** Purpose: System Call Tracer                                                **
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/reg.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ptrace.h>

#define MAX 350

#ifdef ORIG_EAX
#define EAX_OR_RAX ORIG_EAX
#else
#define EAX_OR_RAX ORIG_RAX
#endif

int main(int argc, char *argv[]) {
    pid_t pid;

    // Child Process
    if ((pid=fork()) == 0){
        // Set Trace
        ptrace(PTRACE_TRACEME);
        kill(getpid(), SIGSTOP);
        return execvp(argv[1], argv+1);
    } 

    // Parent Process
    else {
        int i,status,callnum;
        int calls[MAX] = {0};
        int max = 0;
        FILE *outptr;

        // Allows Parent to Distinguish System Calls from Other Traps
        ptrace(PTRACE_SETOPTIONS, pid, NULL, PTRACE_O_TRACESYSGOOD);

        for( ; ; ) {

            // Stops at System Call
            ptrace(PTRACE_SYSCALL, pid, NULL, 0);
            waitpid(pid, &status, 0);
            if (WIFEXITED(status)) break;

            // Records Each System Call
            callnum = ptrace(PTRACE_PEEKUSER, pid, sizeof(long)*EAX_OR_RAX);
            if(callnum > max) max = callnum;
            calls[callnum]++;

            ptrace(PTRACE_SYSCALL, pid, NULL, 0);
            waitpid(pid, &status, 0);
            if (WIFEXITED(status)) break;

        }

        // Print System Calls to Output File
        outptr = fopen(argv[2],"w");
        for(i = 0; i < max+1; i++){
            if (calls[i] > 0){
            	fprintf(outptr,"%d\t%d\n",i,calls[i]);
            }
        }
        fclose(outptr);
    }
    return (0);
}
