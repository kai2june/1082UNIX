#include <unistd.h>
#include <getopt.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <type_traits>
#include <dlfcn.h>
#include <stdarg.h>
#include <vector>
#include <pwd.h>
#include <errno.h>

__attribute__((constructor)) void init()
{
    // printf("__attribute__((constructor)) %d pre_proc_1\n", __LINE__);
    setenv("LD_PRELOAD", "./sandbox.so", 1);
	setenv("BASE_PATH", ".", 1);
}

int main(int argc, char* argv[])
{
	if( argc == 1)
	{
		std::cout << "no command given" << std::endl; 
		return 0;
	}
    int opt;
    char optstring[] = "p:d:";

	bool minus_optind = false;
    while ((opt = getopt(argc, argv, optstring)) != -1)
    {
		minus_optind = true;
        if ( opt == 'p' )
		{
			setenv("LD_PRELOAD", optarg, 1);
			minus_optind = false;
		}
		else if ( opt == 'd' )
		{
			minus_optind = false;
			setenv("BASE_PATH", optarg, 1);
		}
		else
		{
			std::cerr << "usage: ./sandbox [-p sopath] [-d basedir] [--] cmd [cmd args ...]\n-p: set the path to sandbox.so, default = ./sandbox.so\n-d: the base directory that is allowed to access, default = .\n--: seperate the arguments for sandbox and for the executed command\n" << std::endl;
			exit(EXIT_FAILURE);
		}
		// printf("opt = %c\n", opt);
        // printf("optarg = %s\n", optarg);
        // printf("optind = %d\n", optind);
        // printf("argv[optind - 1] = %s\n\n",  argv[optind - 1]);
    }
	if ( minus_optind )
		--optind;
	char* cmd = new char[100];
	if ( strcmp(argv[optind-1], "--") != 0 )
	{
		std::cerr << "usage: ./sandbox [-p sopath] [-d basedir] [--] cmd [cmd args ...]\n-p: set the path to sandbox.so, default = ./sandbox.so\n-d: the base directory that is allowed to access, default = .\n--: seperate the arguments for sandbox and for the executed command\n" << std::endl;
		std::cerr << "In short, you forgot '--'" << std::endl;
		exit(EXIT_FAILURE);
	}
	if (argv[optind] != NULL)
		strcpy(cmd, argv[optind++]);
	// std::cout << "COMMAND_LAUNCHER.CPP: CMD=>" << cmd << std::endl;

	/*
	/// @brief I want to execve a new process
	*/
	// std::cout << "ARGC=>" << argc << std::endl;
	char* args[20];
	strcpy(args[0], cmd);
	int cnt = 1;
	for( int it = optind; it != argc; ++it, ++cnt)
	{
		// std::cout << "\nIT=>" << it << "CNT=>" << cnt << std::endl;  
		// std::cout << "ARGV[it]=>" << argv[it] << std::endl;
		args[cnt] = new char[200];
		strcpy(args[cnt], argv[it]);
		// std::cout << "ARGS[cnt]=>" << args[cnt] << std::endl;
	}
	// std::cout << "\nCNT=>" << cnt << std::endl;
	args[cnt] = (char*)NULL;
	// std::cout << "\nCMD=>" << cmd << std::endl; 

	char* LD_PRELOAD = new char[200];
	strcpy(LD_PRELOAD, "LD_PRELOAD=");
	strcat(LD_PRELOAD, getenv("LD_PRELOAD"));
	char* BASE_PATH = new char[200];
	strcpy(BASE_PATH, "BASE_PATH=");
	strcat(BASE_PATH, getenv("BASE_PATH"));
	char* PWD = new char[200];
	strcpy(PWD, "PWD=");
	strcat(PWD, getenv("PWD"));
	char* HOME = new char[200];
	strcpy(HOME, "HOME=");
	strcat(HOME, getenv("HOME"));
	// std::cout << "COMMAND_LAUNCHER.CPP: LD_PRELOAD=>" << LD_PRELOAD 
	// 		  << " BASE_PATH=>" << BASE_PATH 
	// 		  << " PWD=>" << PWD 
	// 		  << " HOME=>" << HOME 
	// 		  << std::endl; 
	char* envp[] = {LD_PRELOAD, BASE_PATH, PWD, HOME, NULL};

	execve("./choice", args, envp);

	std::cout << "THIS LINE SHOULD NOT BE PRINTED "<< "ARGV[0]=>" << argv[0] << std::endl;
	/*
	/// @brief bottom of trying to execve a new process
	*/
	
	return 0;
}

/// @brief command line 選項-- 
/// @brief 檢查 -d 給的path  (DONE)
/// @brief exec系列, system()實作輸出error message要擋
/// @brief exec系列, variadic參數要parse