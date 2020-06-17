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

bool is_number(const char* str)
{
	if(strlen(str) == 0 )
		return false;
	int i = 0;
	while(str[i] != '\0')
	{
		if (!isdigit(str[i++]))
			return false;
	}
	return true;
}

int main(int argc, char* argv[])
{
    // std::cout << "OPTIND=>" << optind << std::endl;
    // std::cout << "LD_PRELOAD=>" << getenv("LD_PRELOAD") << std::endl;
    // for (int i = 0; i<argc; ++i)
    //     std::cout << "i=>" << i << "ARGV[i]=>" << argv[i] << std::endl;
    /// @brief indigenous steelmaking
	if ( strcmp(argv[0], "chdir") == 0 )
	{
		const char* path = argv[optind++];
		chdir(path);
	} 
	else if ( strcmp(argv[0], "chmod") == 0 )
	{
        // std::cout << "ITS CHMOD" << std::endl;
		const char *pathname = argv[optind++];
		mode_t mode = strtol(argv[optind++], NULL, 8);
		chmod(pathname, mode);
	}
	else if ( strcmp(argv[0], "chown") == 0 )
	{
		const char *pathname = argv[optind++];

		struct passwd* pwd;
		uid_t owner;
		if (!is_number(argv[optind]))
		{
			pwd = getpwnam(argv[optind]);
            std::cout << "OWNER_UID=>" << pwd->pw_uid << std::endl;
			owner = pwd->pw_uid;
		}
		else 
			owner = atoi(argv[optind]);
		optind++;

		gid_t group;
		if (!is_number(argv[optind]))
		{
			pwd = getpwnam(argv[optind]);
            std::cout << "GROUP_GID=>" << pwd->pw_gid << std::endl;
			group = pwd->pw_gid;
		}
		else
			group = atoi(argv[optind]); 
		optind++;

		chown(pathname, owner, group);
	}
	else if( strcmp(argv[0], "creat") == 0 )
	{
		const char* pathname = argv[optind++];
		mode_t mode = strtol(argv[optind++], NULL, 8);
		creat(pathname, mode);
	}
	else if( strcmp(argv[0], "fopen") == 0 )
	{
		const char* pathname = argv[optind++];
		const char* mode = argv[optind++];
		fopen(pathname, mode);
	}
	else if ( strcmp(argv[0], "link") == 0 )
	{
		const char* oldpath = argv[optind++];
		const char* newpath = argv[optind++];
		link(oldpath, newpath);
	}
	else if ( strcmp(argv[0], "mkdir") == 0 )
	{
		const char* pathname = argv[optind++];
		mode_t mode = strtol(argv[optind++], NULL, 8);
		mkdir(pathname, mode);
	}
	else if ( strcmp(argv[0], "open") == 0 )
	{
        std::cout << "launch" << std::endl;
		const char* pathname = argv[optind++];
        std::cout << "pathname:" << pathname<< std::endl;
		int flags = atoi(argv[optind++]);
		std::cout << "flags:" << flags<< std::endl;
		mode_t mode = strtol(argv[optind++], NULL, 8);
        std::cout << "mode:" << mode << std::endl;
		open(pathname, flags, mode);
	}
	else if ( strcmp(argv[0], "openat") == 0 )
	{
		int dirfd = atoi(argv[optind++]);
		const char *pathname = argv[optind++];
		int flags = atoi(argv[optind++]);
		mode_t mode = strtol(argv[optind++], NULL, 8);
		openat(dirfd, pathname, flags, mode);
	}
	else if ( strcmp(argv[0], "opendir") == 0 )
	{
		const char* name = argv[optind++];
		opendir(name);
	}
	else if ( strcmp(argv[0], "readlink") == 0 )
	{
		const char *pathname = argv[optind++];
		char *buf = argv[optind++];
		size_t bufsiz = atoi(argv[optind++]);
		readlink(pathname, buf, bufsiz);
	}
	else if ( strcmp(argv[0], "remove") == 0 )
	{
		const char* pathname = argv[optind++];
		remove(pathname);
	}
	else if ( strcmp(argv[0], "rename") == 0 )
	{
		const char* oldpath = argv[optind++];
		const char* newpath = argv[optind++];
		rename(oldpath, newpath);
	}
	else if ( strcmp(argv[0], "rmdir") == 0 )
	{
		const char* pathname = argv[optind++];
		rmdir(pathname);
	}
	else if ( strcmp(argv[0], "stat") == 0 )
	{
		const char *pathname = argv[optind++];
		struct stat statbuf;
		__xstat(3, pathname, &statbuf);
	}
	else if ( strcmp(argv[0], "symlink") == 0 )
	{
		const char *target = argv[optind++];
		const char *linkpath = argv[optind++];
		symlink(target, linkpath);
	}
	else if ( strcmp(argv[0], "unlink") == 0 )
	{
		const char* pathname = argv[optind++];
		unlink(pathname);
	}
	// else if ( strcmp(argv[0], "execl") == 0 )
	// {
	// 	std::cerr << "execl is not allowed" << std::endl;
	// 	exit(-2);
	// }
	// else if ( strcmp(argv[0], "execle") == 0 )
	// {
	// 	std::cerr << "execle is not allowed" << std::endl;
	// 	exit(-2);
	// }
	// else if ( strcmp(argv[0], "execlp") == 0 )
	// {
	// 	std::cerr << "execlp is not allowed" << std::endl;
	// 	exit(-2);
	// }
	// else if ( strcmp(argv[0], "execv") == 0 )
	// {
	// 	std::cerr << "execv is not allowed" << std::endl;
	// 	exit(-2);
	// }
	// else if ( strcmp(argv[0], "execve") == 0 )
	// {
	// 	char* args[20];
	// 	args[0] = new char[200];
	// 	strcpy(args[0], argv[1]);
	// 	int cnt = 1;
	// 	for( int it = 2; it < argc; ++it, ++cnt)
	// 	{
	// 		std::cout << "\nIT=>" << it << "CNT=>" << cnt << std::endl;  
	// 		std::cout << "ARGV[it]=>" << argv[it] << std::endl;
	// 		args[cnt] = new char[200];
	// 		strcpy(args[cnt], argv[it]);
	// 		std::cout << "ARGS[cnt]=>" << args[cnt] << std::endl;
	// 	}
	// 	std::cout << "\nCNT=>" << cnt << std::endl;
	// 	args[cnt] = (char*)NULL;

	// 	char* LD_PRELOAD = new char[200];
	// 	strcpy(LD_PRELOAD, "LD_PRELOAD=");
	// 	strcat(LD_PRELOAD, getenv("LD_PRELOAD"));
	// 	char* BASE_PATH = new char[200];
	// 	strcpy(BASE_PATH, "BASE_PATH=");
	// 	strcat(BASE_PATH, getenv("BASE_PATH"));
	// 	char* PWD = new char[200];
	// 	strcpy(PWD, "PWD=");
	// 	strcat(PWD, getenv("PWD"));
	// 	char* HOME = new char[200];
	// 	strcpy(HOME, "HOME=");
	// 	strcat(HOME, getenv("HOME"));
	// 	std::cout << "COMMAND_LAUNCHER.CPP: LD_PRELOAD=>" << LD_PRELOAD 
	// 			<< " BASE_PATH=>" << BASE_PATH 
	// 			<< " PWD=>" << PWD 
	// 			<< " HOME=>" << HOME 
	// 			<< std::endl; 
	// 	char* envp[] = {LD_PRELOAD, BASE_PATH, PWD, HOME, NULL};
	// 	execve("./choice", args, envp);
	// }
	// else if ( strcmp(argv[0], "execp") == 0 )
	// {
	// 	std::cerr << "execp is not allowed" << std::endl;
	// 	exit(-2);
	// }
	// else if ( strcmp(argv[0], "system") == 0 )
	// {
	// 	char* command = new char[1000];
	// 	strcpy(command, argv[0]);
	// 	for (int it = 1; it < argc; ++it)
	// 	{
	// 		strcat(command, " ");
	// 		strcat(command, argv[it]);
	// 	}
	// 	system(command); 
	// }
	else
	{
		std::cerr << argv[0]<< ": command not found" << std::endl;
	}


    return 0;
}