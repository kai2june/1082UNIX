#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <cstdio>
#include <cassert>
#include <cstdlib>

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
    if ( child == 0 ) /* child */
    {
        if ( ptrace(PTRACE_TRACEME, 0, 0, 0) < 0 )
            exit(-6);
        execvp(argv[1], argv+1);
        exit(-7);
    }
    else /* parent */
    {
        /// @brief parse inject code
        std::FILE* inject_file = std::fopen("./no_more_traps.txt", "r");
        if(!inject_file) 
        {
            std::perror("File opening failed");
            return EXIT_FAILURE;
        }
        int inject_byte[6281];
        for(int idx=0; idx<6281; ++idx)
        {
            char* tmp = new char[4];
            std::fgets(tmp, 3, inject_file);
            inject_byte[idx] = (int)strtol(tmp, NULL, 16);
            // std::cerr << inject_byte[idx] << std::endl;
        }

        /// @brief ptrace procedure below
        int wait_status = 0;
        /// @brief check if child normally complete
        if ( waitpid(child, &wait_status, 0) < 0 )
        {
            exit(-1);
        }
        assert(WIFSTOPPED(wait_status)); // check if child normally complete.
        ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_EXITKILL); // child will be killed if parent leave
        
        // long base_addr = 0x4000b0;
        int idx=0;
        ptrace(PTRACE_CONT, child, 0, 0); // start child process
        while(waitpid(child, &wait_status, 0) > 0) // child will stop when encountering 0xcc
        {
            struct user_regs_struct regs;
            if(!WIFSTOPPED(wait_status))
                continue;
            if(ptrace(PTRACE_GETREGS, child, 0, &regs) != 0) 
                exit(-2);
            long code = ptrace(PTRACE_PEEKTEXT, child, regs.rip-1, 0); // regs.rip-1 because 0xcc is 1 byte.
            if ( code == 0xcc )
                std::cout << "it's 0xcc!!!!!!!!!!!!" << std::endl;
            if(ptrace(PTRACE_POKETEXT, child, regs.rip-1, (code & 0xffffffffffffff00) | inject_byte[idx++]) != 0)
                exit(-3);
            regs.rip = regs.rip-1;
            regs.rdx = regs.rax;
            if(ptrace(PTRACE_SETREGS, child, 0, &regs) != 0)
                exit(-4);
            // fprintf(stderr, "0x%llx: rax=%llx rdi=%llx rsi=%llx rdx=%llx r10=%llx r8=%llx r9=%llx\n",
            // regs.rip-1, regs.orig_rax, regs.rdi, regs.rsi, regs.rdx, regs.r10, regs.r8, regs.r9);
            ptrace(PTRACE_CONT, child, 0, 0);
        }

        perror("done~~~");
    }

    return 0;
}