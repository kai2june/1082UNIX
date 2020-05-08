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

#define	FILE_A	"trash.txt"
#define DIR_A "dirrr"
// #define	FILE_NULL	"/dev/null"

int main(int argc, char* argv[])
{
	setenv("LD_PRELOAD", "./sandbox.so", 1);
	setenv("SO_PATH", "./sandbox.so", 1);
    setenv("BASE_PATH", ".", 1);
	if( argc == 1)
	{
		std::cout << "no command given" << std::endl; 
		return 0;
	}
    int opt;
    char optstring[] = "p:d:";

	char* SO_PATH = getenv("SO_PATH");
	char* BASE_PATH = getenv("BASE_PATH");

	bool minus_optind = false;
    while ((opt = getopt(argc, argv, optstring)) != -1)
    {
		minus_optind = true;
        if ( opt == 'p' )
		{
            setenv("SO_PATH", optarg, 1);
			setenv("LD_PRELOAD", "./sandbox.so", 1);
			minus_optind = false;
		}
		else if ( opt == 'd' )
		{
            setenv("BASE_PATH", optarg, 1);
			minus_optind = false;
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
	char cmd[100];
	strcpy(cmd, argv[optind++]);
	std::cout << "cmd :" << cmd << std::endl;
	// char* cmd_args[100];
	// int i = 0;
	// while ( argv[optind] != NULL )
	// {
	// 	cmd_args[i++] = argv[optind++];
	// }


	// if ( strcmp(cmd, "chdir") == 0 )
	// 	chdir(cmd_args[0]);
	if ( strcmp(cmd, "chmod") == 0 )
	{
		std::cout << "This is chmod" << std::endl;
		int i = 0;
		const char* pathname = argv[optind++];
		const char* str_mode = argv[optind++];
		mode_t mode = 0;
		mode |= strtol(str_mode, NULL, 8);
		// execve("chmod", "helloworld.cpp", 777, (char*)NULL, "LD_PRELOAD=./sandbox.so");
		chmod(pathname, mode);
	}
	// if ( strcmp(cmd, "chown") == 0 )
	// if ( strcmp(cmd, "creat") == 0 )
	// if ( strcmp(cmd, "fopen") == 0 )
	// if ( strcmp(cmd, "link") == 0 )
	// if ( strcmp(cmd, "mkdir") == 0 )
	// if ( strcmp(cmd, "open") == 0 )
	// if ( strcmp(cmd, "openat") == 0 )
	// if ( strcmp(cmd, "opendir") == 0 )
	// if ( strcmp(cmd, "readlink") == 0 )
	// if ( strcmp(cmd, "remove") == 0 )
	// if ( strcmp(cmd, "rename") == 0 )
	// if ( strcmp(cmd, "rmdir") == 0 )
	// if ( strcmp(cmd, "stat") == 0 )
	// if ( strcmp(cmd, "symlink") == 0 )
	// if ( strcmp(cmd, "unlink") == 0 )
	// if ( strcmp(cmd, "execl") == 0 )
	// if ( strcmp(cmd, "execle") == 0 )
	// if ( strcmp(cmd, "execlp") == 0 )
	// if ( strcmp(cmd, "execv") == 0 )
	// if ( strcmp(cmd, "execve") == 0 )
	// if ( strcmp(cmd, "execvp") == 0 )
	// if ( strcmp(cmd, "system") == 0 )

	// char *argv[] = { FILE_A, NULL };

	// chdir("~");
	// creat(FILE_A, O_RDWR);
	// chmod("..", 644);
	// chown(FILE_A, 0, 0);
	// // fopen(FILE_A, "r+");
	// link(FILE_A, "link.txt");
	// mkdir(DIR_A, 0755);
	// open(FILE_A, O_RDONLY);
	// openat(AT_FDCWD, FILE_A, 0755);
	// opendir(DIR_A);

	// char buf[128];
	// readlink(link.txt, buf, sizeof(buf));
	
	/// @brief checked
	// remove(FILE_A);

	/// @brief checked
	// rename(FILE_A, "rename.txt");

	/// @brief checked
	// rmdir(DIR_A); 
	
	// struct stat st;
	// __xstat(3, FILE_A, &st);

	/// @brief checked
	// symlink(FILE_A, "symlink.txt"); 

	/// @brief checked
	// unlink(FILE_A);	
	
	// int error_code = execl(FILE_A, FILE_A, NULL);
	// std::cout << "ERROR_CODE: " << error_code << std::endl;
	// execle(FILE_A, FILE_A, NULL, NULL);
	// execlp(FILE_A, FILE_A, NULL);
	// execv(FILE_A, argv);
	// execvp(FILE_A, argv);
	// execve(FILE_A, argv, NULL);

	/// @brief checked
	// system("echo $PATH");
	
	
	return 0;
}

/// @brief command line 選項-- 
/// @brief 檢查 -d 給的path  (DONE)
/// @brief exec系列, system()實作輸出error message要擋
/// @brief exec系列, variadic參數要parse