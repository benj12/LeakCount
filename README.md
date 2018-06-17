# LeakCount

Miles Bagwell

ECE322 Spring 2015 

Project 1

Archive Contents:
    
    shim.c      - shim library used with leakcount. Intercepts calls to 
                  malloc and free

    leakcount.c - Memory leak detector runs given command and outputs memory
                  leaks to stderr
                  ./leakcount <command>

    sctracer.c  - System call tracer runs given command and outputs system 
                  calls to given output file 
                  ./sctracer <command> <output_file>
    makefile


DESIGN
    
    leakcount - Leakcount forks to create a new process. In the child
                the given command is run with the shim library made by
                shim.c. When calls are made to malloc or free by the 
                command, the shim library intercepts these calls. 
                My design uses a one way linked list to keep track of
                the size and location of each allocation. Calls to 
                free search through the list for the specified pointer
                and frees it. When the program is finished the 
                destructor is called. The processes communicates with
                with a message passing mailbox. the destructor's first
                message is the number of leaks, and each subsequent 
                message is the size of a leak if any. The parent waits
                on the child to finish and then checks the mail. From
                the first message the parent knows how many messages 
                to expect. It then outputs to stderr.
       
    sctracer  - Tracer begins with a fork to create the process to  
                trace. The child request to be traced, tells itself
                to kill when the SIGSTOP signal is recieved and then
                returns the execution of the given command. The parent
                sets the PTRACE_SETOPTIONS to ignore normal traps from 
                system calls. The parent then enters an endless loop
                that will end once the child process has ended. Inside
                the loop, the count of system calls are stored in an   
                integer array. Each time a system call is made, the 
                system call number in the array is incremented. Once 
                the child has ended, the parent leaves the loop and 
                prints the call info to the given output file.
           
KNOWN ISSUES
    
    leakcount - I was having problems with the message passing service
                trying to confirm each time a message was recieved. I
                think this made my program get stuck and not execute so
                now the message passing is just a one-way service. 


REFRENCES

    Piazza
    SVN files
    http://linux.die.net/man/2/ptrace
    http://phoxis.org/2011/04/27/c-language-constructors-and-destructors-
    https://blog.nelhage.com/2010/08/write-yourself-an-strace-in-70-lines-of-code/
