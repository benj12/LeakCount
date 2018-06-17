/*******************************************************************************
** shim.c                                                                     **
** Miles Bagwell                                                              **
** leonb                                                                      **
** ECE 322, Spring 2015                                                       **
** Project #1                                                                 **
**                                                                            **
** Purpose: Shim Library intercepts Malloc/Free calls                         **
*******************************************************************************/
#define _GNU_SOURCE

void __attribute__ ((constructor)) leak_init(void);
void __attribute__ ((destructor)) sender(void);

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <string.h>

void *(*original_malloc)(size_t size) = NULL;
void  (*original_free)(void *ptr) = NULL;

#define FIFO_FILE       "MYFIFO"

#define MAILBOX_NUMBER 1200    //The mailbox number I want to use



typedef struct list_node {
    int size;
	void *datap;
    struct list_node *next;
} node;

struct list_t{
    node *head;
    int entries;
} list;

struct {
   long mtype;        //message type
   int 	size;       //body of the message
} msgp, cmbox;


/*******************************************************************************
**                                                                            **
**                                                                            **
*******************************************************************************/
void leak_init(void){
  if (original_malloc == NULL) {
  		original_malloc = dlsym(RTLD_NEXT, "malloc");
  	}
  if (original_free == NULL) {
  		original_free = dlsym(RTLD_NEXT, "free");
  	}
}
/*******************************************************************************
**                                                                            **
**                                                                            **
*******************************************************************************/
void *malloc(size_t size){
	node *new_node;
	node *index;
	void *space_ptr = original_malloc(size);

    new_node = (node *) original_malloc(sizeof(node));
	new_node->size = (int) size;
	new_node->datap = space_ptr;

	if (list.head == NULL){
		list.head = new_node;
		list.entries = 1;
	}
	else {
		index = list.head;
		while (index->next != NULL){
			index = index->next;
		}
		index->next = new_node;
		list.entries++;
	}

	return space_ptr;
}
/*******************************************************************************
**                                                                            **
**                                                                            **
**                                                                            **
*******************************************************************************/
void free(void *ptr){
	node *index, *prev;

	index = list.head;

	if (index != NULL && index->datap == ptr){
		list.head = index->next;
		original_free(ptr);
		original_free(index);
		list.entries--;
	}
	else {
		while (index != NULL && index->datap != ptr){
			prev = index;
			index = index->next;
		}
		if (index != NULL){
			prev->next = index->next;
			original_free(ptr);
			original_free(index);
			list.entries--;
		}
		else{
		    original_free(ptr);
		}
	}
	
}
/*******************************************************************************
** Called when the library is unloaded                                        **
*******************************************************************************/
void sender(void){
	node *index;
	int i,length, status;
   
	//Create the Mailboxes
	int msqidS = msgget(MAILBOX_NUMBER, 0600 | IPC_CREAT);
	int msqidC = msgget((MAILBOX_NUMBER + 1), 0600 | IPC_CREAT);

	msgp.mtype = 1;
	length = sizeof(msgp) - sizeof(long);

	msgp.size = list.entries;

	msgsnd( msqidS, &msgp, length, 0);
	index = list.head;


	//msgrcv( msqidC, &cmbox, length, 1, 0);


	//Initialize the message to be sent
	for (i = 0; i < list.entries; i++){
		msgp.mtype = 1;
		msgp.size = (int) index->size;

		length = sizeof(msgp) - sizeof(long);
		msgsnd( msqidS, &msgp, length, 0);


		//Wait for a new message from the central server

		index = index->next;
	}

	//Remove the mailbox
	status = msgctl(msqidC, IPC_RMID, 0);

	//Validate nothing when wrong when trying to remove mailbox
	if(status != 0){
		perror("Closing mailbox");
	}
}

