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

#define	FILE_A	"./trash.txt"
#define DIR_A "dirrr"
// #define	FILE_NULL	"/dev/null"


int main(int argc, char* argv[])
{
	if( argc == 1)
		std::cout << "no command given" << std::endl; 

    int opt;
    char optstring[] = "p:d:";

	char* SO_PATH = getenv("SO_PATH");
	char* BASE_PATH = getenv("BASE_PATH");

    while ((opt = getopt(argc, argv, optstring)) != -1)
    {
        if ( opt == 'p' )
            setenv("SO_PATH", optarg, 1);
        else if ( opt == 'd' )
            setenv("BASE_PATH", optarg, 1);
		else
			std::cout << "usage: ./sandbox [-p sopath] [-d basedir] [--] cmd [cmd args ...]\n-p: set the path to sandbox.so, default = ./sandbox.so\n-d: the base directory that is allowed to access, default = .\n--: seperate the arguments for sandbox and for the executed command\n" << std::endl;
        // printf("opt = %c\n", opt);
        // printf("optarg = %s\n", optarg);
        // printf("optind = %d\n", optind);
        // printf("argv[optind - 1] = %s\n\n",  argv[optind - 1]);
    }
	std::cout << SO_PATH << ", " << BASE_PATH << std::endl;

    // if ( argv[optind] == "--")
    //     cmd = argv[++optind];



	// char *argv[] = { FILE_A, NULL };

	// chdir("~");
	creat(FILE_A, O_RDWR);
	chmod(FILE_A, 644);
	chown(FILE_A, 0, 0);
	// fopen(FILE_A, "r+");
	link(FILE_A, "link.txt");
	mkdir(DIR_A, 0755);
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
	
	struct stat st;
	__xstat(3, FILE_A, &st);

	/// @brief checked
	// symlink(FILE_A, "symlink.txt"); 

	/// @brief checked
	// unlink(FILE_A);	
	
	// execl(FILE_A, FILE_A, NULL);
	// execle(FILE_A, FILE_A, NULL, NULL);
	// execlp(FILE_A, FILE_A, NULL);
	// execv(FILE_A, argv);
	// execvp(FILE_A, argv);
	// execve(FILE_A, argv, NULL);

	/// @brief checked
	// system("echo $PATH");
	
	// return -1;

	return 0;
}

/// @brief command line 選項-- 
/// @brief 檢查 -d 給的path
/// @brief exec系列, system()實作輸出error message要擋
/// @brief exec系列, variadic參數要parse