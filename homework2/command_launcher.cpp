#include <unistd.h>
#include <getopt.h>
#include <iostream>
#include <stdio.h>

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


#define	FILE_A	"trash.txt"
#define DIR_A "dirrr"
// #define	FILE_NULL	"/dev/null"


#define DECLARE_TYPE(name, prototype)\
        using name##_type = prototype;\
        static name##_type old_##name = nullptr;
#define LOAD_LIB(name)\
        old_##name == nullptr ? \
        old_##name = (name##_type)dlsym(RTLD_NEXT, #name),\
        old_##name == nullptr ? \
            dprintf(stderr_fd, #name "() not found.\n"),\
            exit(EXIT_FAILURE)\
            :\
            void()\
        :\
        void()
DECLARE_TYPE(execve, int (*)(const char *pathname, char *const argv[], char *const envp[]));
int stderr_fd = 2;
int execve(const char *pathname, char *const argv[], char *const envp[])
{
    std::cout << "MY OWN EXECVE:" << std::endl;
    // if ( !is_sub_directory(pathname) )
    // {
    //     std::cerr << "access to " << pathname << " is not allowd." << std::endl;
    //     return -1;
    // }
	if ( strcmp(pathname, "chmod") == 0)
	{
		std::cout << "ITS CHMOD!!" << std::endl;
		int rtn = 59487;
		rtn = chmod("helloworld.cpp", 123);
		std::cout << "LAUNCHER RTN:" << rtn << std::endl;
		
	}
	std::cout << "BEFORE LOAD_LIB:" << std::endl;
	LOAD_LIB(execve);
    int rtn = old_execve(pathname, argv, envp);
	std::cout << "AFTER LOAD_LIB:" << std::endl; 
    return rtn;
}




int main(int argc, char* argv[])
{
	// setenv("LD_PRELOAD", "./sandbox.so", 1);
	// setenv("SO_PATH", "./sandbox.so", 1);
    // setenv("BASE_PATH", ".", 1);
	if( argc == 1)
	{
		std::cout << "no command given" << std::endl; 
		return 0;
	}
    int opt;
    char optstring[] = "p:d:";

	char* SO_PATH = new char[100];
	char* BASE_PATH = new char[100];

	bool minus_optind = false;
    while ((opt = getopt(argc, argv, optstring)) != -1)
    {
		minus_optind = true;
        if ( opt == 'p' )
		{
            strcpy(SO_PATH, optarg);
			// setenv("LD_PRELOAD", "./sandbox.so", 1);
			minus_optind = false;
		}
		else if ( opt == 'd' )
		{
            strcpy(BASE_PATH, optarg);
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
	char* cmd = new char[100];
	if (argv[optind] != NULL)
		strcpy(cmd, argv[optind++]);
	std::cout << "cmd :" << cmd << std::endl;

	// char* cmd_args[100];
	// int i = 0;
	// while ( argv[optind] != NULL )
	// {
	// 	cmd_args[i++] = argv[optind++];
	// }


	// char* args[] = {"chmod", "775", "./helloworld.cpp", (char*)0};
	// char* tmp_env = new char[100];
	// strcpy(tmp_env, "LD_PRELOAD=");
	// strcat(tmp_env, SO_PATH);
	// std::cout << "tmp_env :" << tmp_env << std::endl;
	// char* envp[] = {tmp_env, NULL};
	// execve(SO_PATH, args, envp);

	// char *args[ ]={"ls", "-al", "/etc/passwd", NULL};   
	// char *envp[ ]={"PATH=/bin", NULL};
	// execve("/bin/ls", args, envp);

	char *args2[ ]={"chmod", "000", "helloworld.cpp", (char*)NULL};   
	char *envp2[ ]={"LD_PRELOAD=./sandbox.so", NULL};
	execve("/bin/chmod", args2, envp2);

	// if ( strcmp(cmd, "chdir") == 0 )
	// {
	// 	char* pathname = argv[optind++];
	// 	chdir(pathname);
	// }
	// if ( strcmp(cmd, "chmod") == 0 )
	// {
	// 	std::cout << "This is chmod" << std::endl;
	// 	// char* args[100];  
	// 	// int cnt_args = 0;
	// 	// while (argv[optind] != NULL)
	// 	// {
	// 	// 	std::cout << "argv[optind] = " << argv[optind] << std::endl;
	// 	// 	args[cnt_args++] = argv[optind++];
	// 	// }
	// 	char* pathname = argv[optind++];
	// 	char* str_mode = argv[optind++];
	// 	mode_t mode = 0;
	// 	mode |= strtol(str_mode, NULL, 8);
	// 	chmod(pathname, mode);
	// 	// execve("chmod", "helloworld.cpp", 777, (char*)NULL, "LD_PRELOAD=./sandbox.so");
	// }
	// if ( strcmp(cmd, "chown") == 0 )
	// {
	// 	char* pathname = argv[optind++];
	// 	uid_t uid = strtol(argv[optind++], NULL, 10);
	// 	gid_t gid = strtol(argv[optind++], NULL, 10);
	// 	std::cout << "minus3: "<< argv[optind-3] << "path: " << pathname << "uid: " << uid << "gid: " << gid << std::endl;
	// 	chown(pathname, uid, gid);
	// }
	// if ( strcmp(cmd, "creat") == 0 )
	// {
	// 	char* pathname = argv[optind++];
	// 	char* str_mode = argv[optind++];
	// 	mode_t mode = 0;
	// 	mode |= strtol(str_mode, NULL, 8);
	// 	creat(pathname, mode);
	// }
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
	// system("LD_PRELOAD=./sandbox.so ./sandbox");
	
	
	return 0;
}

/// @brief command line 選項-- 
/// @brief 檢查 -d 給的path  (DONE)
/// @brief exec系列, system()實作輸出error message要擋
/// @brief exec系列, variadic參數要parse