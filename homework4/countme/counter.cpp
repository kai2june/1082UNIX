#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

int main(int argc, char* argv[])
{
    pid_t child;
    if (argc < 2)
    {
        fprintf(stderr, "usage: %s program [args ...]\n", argv[0]);
        return -1;
    }
    if ( (child=fork()) < 0 )
        exit(-1);
    if (child == 0)
    {
        if(ptrace(PTRACE_TRACEME, 0, 0, 0) < 0)
            exit(-2);
        execvp(argv[1], argv+1);
        exit(-3);
    }
    else 
    {
        long long counter = 0LL;
        int wait_status;
        if(waitpid(child, &wait_status, 0) < 0) // check if child can normally complete.
        {
            std::cerr << "yes" << std::endl; // this line will not be printed if child_pid >= 0 
            exit(-4);
        }
        ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_EXITKILL); // kill child after parent leave.
        while (WIFSTOPPED(wait_status)) // stopped every PTRACE_SINGLESTEP. 
        {
            // std::cerr << "wait_status:" << wait_status << std::endl; // wait_status always = 1407
            counter++;
            if(ptrace(PTRACE_SINGLESTEP, child, 0, 0) < 0) // ptrace one step at a time
                exit(-5);
            if(waitpid(child, &wait_status, 0) < 0) // wait for child executing one step
                exit(-6);
        }
        fprintf(stderr, "## %lld instruction(s) executed\n", counter);
    }

    return 0;
}