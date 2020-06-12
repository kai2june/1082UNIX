#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <assert.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/types.h>
#include <sys/ptrace.h>

int main(int argc, char* argv[])
{
    std::cout << argv[1] << '\n' << argv << '\n' << (argv+1) << std::endl;
    // pid_t child;
    // if ( (child = fork()) < 0 )
    //     exit(-1);
    // if (child == 0)
    // {
    //     if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0)
    //         exit(-1);
    //     execvp(argv[1], argv+1);
    // }
}