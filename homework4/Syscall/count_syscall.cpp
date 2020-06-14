#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/user.h>
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
    if ( (child = fork()) < 0 )
    {
        exit(-1);
    }
    if ( child == 0 )
    {
        if ( ptrace(PTRACE_TRACEME, 0, 0, 0) < 0 )
            exit(-6);
        execvp(argv[1], argv+1);
        exit(-7);
    }
    else
    {
        long long counter = 0LL;
        int wait_status;
        int enter = 0x01;

        if ( waitpid(child, &wait_status, 0) < 0 )
        {
            exit(-5);
        }
        ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_EXITKILL | PTRACE_O_TRACESYSGOOD);

        while (WIFSTOPPED(wait_status))
        {
            struct user_regs_struct regs;
            if ( ptrace(PTRACE_SYSCALL, child, 0, 0) != 0 )
                exit(-6);
            if ( waitpid(child, &wait_status, 0) < 0 )
                exit(-2);
            if ( !WIFSTOPPED(wait_status) || !(WSTOPSIG(wait_status) & 0x80) )
                continue;
            if ( ptrace(PTRACE_GETREGS, child, 0, &regs) != 0 )
                exit(-1);
            if ( enter )
            {
                fprintf(stderr, "0x%llx: rax=%llx rdi=%llx rsi=%llx rdx=%llx r10=%llx r8=%llx r9=%llx\n",
                    regs.rip-2, regs.orig_rax, regs.rdi, regs.rsi, regs.rdx, regs.r10, regs.r8, regs.r9);
                if ( regs.orig_rax == 0x3c || regs.orig_rax == 0xe7 )
                    fprintf(stderr, "\n");
                counter++;
            }
            else
                fprintf(stderr, "0x%llx: ret = 0x%llx\n", regs.rip-2, regs.rax);
            enter ^= 0x01;
        }
        fprintf(stderr, "## %lld syscall(s) executed\n", counter);
    }

    return 0;
}