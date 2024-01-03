#include <sys/types.h> /* pid_t */
#include <sys/wait.h>  /* wait */
#include <stdio.h>     /* printf */
#include <unistd.h>    /* _exit, fork, execvp */


int main(int argc, char* argv[])
{
    pid_t pid;
    printf("Father: my pid is %d\n", getpid());

    pid = fork();
    if (pid == 0) {  
        printf("Child process: My pid is %d\n", getpid());
        if(argc > 1) {
           printf("-------------- Child doing exec: %s\n", argv[1]);
           execvp(argv[1], &argv[1]);
            _exit(2);
        } 
        _exit(1);
    } 
    printf("Father: Gonna wait for Child\n");

    int status;
    wait(&status);
    printf("-------------- Father: Child finished\n");

    // WIFEXITED, WEXITSTATUS Macro of the gnu lib POSIX standard to recover end status
    if ( WIFEXITED(status) ) {   
        const int es = WEXITSTATUS(status);
        printf("Father: Child Complete with exit status %d\n", es);
        if(es == 1) printf("Father: Child didn't execute any command\n");
        else if(es == 2) printf("Father: Child command was not found\n");
    }

    return 0;
}