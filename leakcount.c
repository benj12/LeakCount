/*******************************************************************************
** leakcount.c                                                                **
** Miles Bagwell                                                              **
** leonb                                                                      **
** ECE 322, Spring 2015                                                       **
** Project #1                                                                 **
**                                                                            **
** Purpose: Memory Leak Detector                                              **
*******************************************************************************/
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAILBOX_NUMBER 1200    //The mailbox number I want to use

struct {
   long mtype;        //message type
   int size;       //body of the message
} msgp, cmbox;

/******************************************************************************/
int main(int argc, char *argv[]){ 
	int i;
    pid_t pid;

    // Child Process
    if ((pid=fork()) == 0){
	    char *const envp[] = {"LD_PRELOAD=./memory_shim.so",NULL};
        char *args[argc];
        memcpy(args,argv+1,(argc)*sizeof(char*));
        
		return execvpe(argv[1],args,envp);
	}
	// Parent Recieves leak info 
	else {
		int total = 0;
		int result,length,status;
   
		//Create the Mailboxes
		int msqidS = msgget(MAILBOX_NUMBER, 0600 | IPC_CREAT);
		//int msqidC = msgget((MAILBOX_NUMBER + 1), 0600 | IPC_CREAT);

   		msgp.mtype = 1; 
		length = sizeof(msgp)-sizeof(long);
	    waitpid(pid, NULL, 0);


		//First Message Recieved is total number of leaks
		result = msgrcv(msqidS, &cmbox, length, 1, 0);
		if (result < 0){
			//perror("msgrcv");
		}

		int leak_count = cmbox.size;

		//Send Response
		//msgsnd(msqidC, &msgp, length, 0); 

		
		// Recieve Leak Info
		for (i = 0; i < leak_count; i++){
			result = msgrcv(msqidS, &cmbox, length, 1, 0);
			if (result < 0){
				//perror("msgrcv");
			}
			fprintf(stderr,"LEAK %d\n",cmbox.size);
			total += cmbox.size;

		}
		fprintf(stderr, "TOTAL %d %d\n",leak_count,total);

		//Remove my own mailbox
		status = msgctl(msqidS, IPC_RMID, 0);

		//Validate nothing when wrong when trying to remove mailbox
		if(status != 0){
			//perror("closing.");
			//printf("\nERROR closing mailbox\n");
   		}


	}


	return 0;
}

