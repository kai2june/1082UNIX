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

__attribute__((constructor)) void init()
{
    printf("\n%d pre_proc_1\n", __LINE__);
    setenv("LD_PRELOAD", "./sandbox.so", 1);
	setenv("BASE_PATH", ".", 1);
}

// bool is_number(const char* str)
// {
// 	if(strlen(str) == 0 )
// 		return false;
// 	int i = 0;
// 	while(str[i] != '\0')
// 	{
// 		if (!isdigit(str[i++]))
// 			return false;
// 	}
// 	return true;
// }

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
			exit(-1);
		}
		printf("opt = %c\n", opt);
        printf("optarg = %s\n", optarg);
        printf("optind = %d\n", optind);
        printf("argv[optind - 1] = %s\n\n",  argv[optind - 1]);
    }
	if ( minus_optind )
		--optind;
	char* cmd = new char[100];
	if (argv[optind] != NULL)
		strcpy(cmd, argv[optind++]);
	std::cout << "\ncmd=>" << cmd << std::endl;

	/*
	/// @brief I want to execve a new process
	*/
	char* pathname = argv[optind];
	char* args[] = {cmd, pathname, argv[optind+1], (char*)NULL};
	std::cout << "\nCMD=>" << cmd << std::endl; 
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
	std::cout << "\nLD_PRELOAD=>" << LD_PRELOAD << "\nBASE_PATH=>" << BASE_PATH << "\nPWD=>" << PWD << "\nHOME=>" << HOME << std::endl; 
	char* envp[] = {LD_PRELOAD, BASE_PATH, PWD, HOME, NULL};
	execve("./choice", args, envp);

	std::cout << "ARGV[0]=>" << argv[0] << std::endl;
	/*
	/// @brief bottom of trying to execve a new process
	*/
	
	// /// @brief indigenous steelmaking
	// if ( strcmp(cmd, "chdir") == 0 )
	// {
	// 	const char* path = argv[optind++];
	// 	chdir(path);
	// } 
	// else if ( strcmp(cmd, "chmod") == 0 )
	// {
	// 	const char *pathname = argv[optind++];
	// 	mode_t mode = strtol(argv[optind++], NULL, 8);
	// 	chmod(pathname, mode);
	// }
	// else if ( strcmp(cmd, "chown") == 0 )
	// {
	// 	const char *pathname = argv[optind++];

	// 	struct passwd* pwd;
	// 	uid_t owner;
	// 	if (!is_number(argv[optind]))
	// 	{
	// 		pwd = getpwnam(argv[optind]);
	// 		owner = pwd->pw_uid;
	// 	}
	// 	else 
	// 		owner = atoi(argv[optind]);
	// 	optind++;

	// 	gid_t group;
	// 	if (!is_number(argv[optind]))
	// 	{
	// 		pwd = getpwnam(argv[optind]);
	// 		group = pwd->pw_gid;
	// 	}
	// 	else
	// 		group = atoi(argv[optind]); 
	// 	optind++;

	// 	chown(pathname, owner, group);
	// }
	// else if( strcmp(cmd, "creat") == 0 )
	// {
	// 	const char* pathname = argv[optind++];
	// 	mode_t mode = strtol(argv[optind++], NULL, 8);
	// 	creat(pathname, mode);
	// }
	// else if( strcmp(cmd, "fopen") == 0 )
	// {
	// 	const char* pathname = argv[optind++];
	// 	const char* mode = argv[optind++];
	// 	fopen(pathname, mode);
	// }
	// else if ( strcmp(cmd, "link") == 0 )
	// {
	// 	const char* oldpath = argv[optind++];
	// 	const char* newpath = argv[optind++];
	// 	link(oldpath, newpath);
	// }
	// else if ( strcmp(cmd, "mkdir") == 0 )
	// {
	// 	const char* pathname = argv[optind++];
	// 	mode_t mode = strtol(argv[optind++], NULL, 8);
	// 	mkdir(pathname, mode);
	// }
	// else if ( strcmp(cmd, "open") == 0 )
	// {
	// 	const char* pathname = argv[optind++];
	// 	int flags = atoi(argv[optind++]);
	// 	std::cout << flags<< std::endl;
	// 	mode_t mode = strtol(argv[optind++], NULL, 8);
	// 	open(pathname, flags, mode);
	// }
	// else if ( strcmp(cmd, "openat") == 0 )
	// {
	// 	int dirfd = atoi(argv[optind++]);
	// 	const char *pathname = argv[optind++];
	// 	int flags = atoi(argv[optind++]);
	// 	mode_t mode = strtol(argv[optind++], NULL, 8);
	// 	openat(dirfd, pathname, flags, mode);
	// }
	// else if ( strcmp(cmd, "opendir") == 0 )
	// {
	// 	const char* name = argv[optind++];
	// 	opendir(name);
	// }
	// else if ( strcmp(cmd, "readlink") == 0 )
	// {
	// 	const char *pathname = argv[optind++];
	// 	char *buf = argv[optind++];
	// 	size_t bufsiz = atoi(argv[optind++]);
	// 	readlink(pathname, buf, bufsiz);
	// }
	// else if ( strcmp(cmd, "remove") == 0 )
	// {
	// 	const char* pathname = argv[optind++];
	// 	remove(pathname);
	// }
	// else if ( strcmp(cmd, "rename") == 0 )
	// {
	// 	const char* oldpath = argv[optind++];
	// 	const char* newpath = argv[optind++];
	// 	rename(oldpath, newpath);
	// }
	// else if ( strcmp(cmd, "rmdir") == 0 )
	// {
	// 	const char* pathname = argv[optind++];
	// 	rmdir(pathname);
	// }
	// else if ( strcmp(cmd, "stat") == 0 )
	// {
	// 	const char *pathname = argv[optind++];
	// 	struct stat statbuf;
	// 	__xstat(3, pathname, &statbuf);
	// }
	// else if ( strcmp(cmd, "symlink") == 0 )
	// {
	// 	const char *target = argv[optind++];
	// 	const char *linkpath = argv[optind++];
	// 	symlink(target, linkpath);
	// }
	// else if ( strcmp(cmd, "unlink") == 0 )
	// {
	// 	const char* pathname = argv[optind++];
	// 	unlink(pathname);
	// }
	// else if ( strcmp(cmd, "execl") == 0 )
	// {
	// 	std::cerr << "execl is not allowed" << std::endl;
	// 	exit(-2);
	// }
	// else if ( strcmp(cmd, "execle") == 0 )
	// {
	// 	std::cerr << "execle is not allowed" << std::endl;
	// 	exit(-2);
	// }
	// else if ( strcmp(cmd, "execlp") == 0 )
	// {
	// 	std::cerr << "execlp is not allowed" << std::endl;
	// 	exit(-2);
	// }
	// else if ( strcmp(cmd, "execv") == 0 )
	// {
	// 	std::cerr << "execv is not allowed" << std::endl;
	// 	exit(-2);
	// }
	// else if ( strcmp(cmd, "execve") == 0 )
	// {
	// 	std::cerr << "execve is not allowed" << std::endl;
	// 	exit(-2);
	// }
	// else if ( strcmp(cmd, "execp") == 0 )
	// {
	// 	std::cerr << "execp is not allowed" << std::endl;
	// 	exit(-2);
	// }
	// else if ( strcmp(cmd, "system") == 0 )
	// {
	// 	std::cerr << "system is not allowed" << std::endl;
	// 	exit(-2);
	// }
	// else if ( strcmp(cmd, "sh") == 0 )
	// {
	// 	std::cerr << "sh is not allowed" << std::endl;
	// 	exit(-2);
	// }
	// else;

	return 0;
}

/// @brief command line 選項-- 
/// @brief 檢查 -d 給的path  (DONE)
/// @brief exec系列, system()實作輸出error message要擋
/// @brief exec系列, variadic參數要parse