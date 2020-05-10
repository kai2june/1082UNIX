#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>

#include <map>
#include <string>
#include <type_traits>
#include <dlfcn.h>
#include <stdarg.h>
#include <iostream>
#include <vector>

#define DECLARE_TYPE(name, prototype)\
        using name##_type = prototype;\
        name##_type old_##name = nullptr;

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

DECLARE_TYPE(chdir, int (*)(const char *path));
DECLARE_TYPE(chmod, int (*)(const char *pathname, mode_t mode));
DECLARE_TYPE(chown, int (*)(const char *pathname, uid_t owner, gid_t group));
DECLARE_TYPE(creat, int (*)(const char *pathname, mode_t mode));
DECLARE_TYPE(fopen, FILE* (*)(const char *pathname, const char *mode));
// DECLARE_TYPE(fdopen, FILE* (*)(int fd, const char *mode));
DECLARE_TYPE(link, int (*)(const char *oldpath, const char *newpath));
DECLARE_TYPE(mkdir, int (*)(const char *pathname, mode_t mode));
DECLARE_TYPE(open, int (*)(const char *pathname, int flags, mode_t mode));
DECLARE_TYPE(openat, int (*)(int dirfd, const char *pathname, int flags, mode_t mode));
DECLARE_TYPE(opendir, DIR* (*)(const char *name));
DECLARE_TYPE(readlink, ssize_t (*)(const char *pathname, char *buf, size_t bufsiz));
DECLARE_TYPE(remove, int (*)(const char *pathname));
DECLARE_TYPE(rename, int (*)(const char *oldpath, const char *newpath));
DECLARE_TYPE(rmdir, int (*)(const char *pathname));
DECLARE_TYPE(__xstat, int (*)(int ver, const char *pathname, struct stat *statbuf));
DECLARE_TYPE(symlink, int (*)(const char *target, const char *linkpath));
DECLARE_TYPE(unlink, int (*)(const char *pathname));
DECLARE_TYPE(execl,  int (*)(const char *pathname, const char *arg, .../* (char  *) NULL */));
/// @brief execle()'s parameter envp moved leftward
/// @brief because variadic parameter(a.k.a. ...) should appear as the last parameter
// DECLARE_TYPE(execle, int (*)(const char *pathname, const char *arg, char *const envp[], .../*, (char *) NULL */));
DECLARE_TYPE(execlp, int (*)(const char *file, const char *arg, .../* (char  *) NULL */));
DECLARE_TYPE(execv, int (*)(const char *pathname, char *const argv[]));
DECLARE_TYPE(execve, int (*)(const char *pathname, char *const argv[], char *const envp[]));
DECLARE_TYPE(execvp, int (*)(const char *file, char *const argv[]));
DECLARE_TYPE(system, int (*)(const char *command));

int stderr_fd = 2;

// __attribute__((constructor)) void init()
// {
//     printf("\n%d pre_proc_1\n", __LINE__);
//     // std::cout << __LINE__ << std::endl;
//     setenv("LD_PRELOAD", "./sandbox.so", 1);
//     setenv("BASE_PATH", ".", 1);
// }

bool is_sub_directory(const char* pathname)
{
    if ( strlen(pathname) == 0 )
    {
        puts("\nin_sub_director(pathname)=>pathname should not be empty.");
        exit(-3);
    }
    /// @brief if only filename, then add "./" prefix
    char* my_path = new char[200];
    if(pathname[0]!='/' && pathname[0]!='~' && pathname[0]!='.')
    {
        strcpy(my_path, "./");
        strcat(my_path, pathname);
    }
    else
        strcpy(my_path, pathname);

    /// @brief get parent path
    char* last_slash = strrchr(getenv("PWD"), '/');
    char* parent = new char[200];
    strncpy(parent, getenv("PWD"), last_slash - getenv("PWD") );

    /// @brief argument pathname ( relative path -> absolute path )
    std::vector<char*> vec;
    const char *delim = "/";
    char * pch = strtok(my_path, delim);
    if ( pch == NULL )
    {
        char* root_tmp = new char[2];
        strcpy(root_tmp, "/");
        vec.emplace_back(root_tmp);
    }
    while (pch != NULL)
    {
        if ( strcmp(pch, ".") == 0 )
            vec.emplace_back(getenv("PWD"));
        else if ( strcmp(pch, "..") == 0 )
            vec.emplace_back(parent);
        else if( strcmp(pch, "~") == 0 )
            vec.emplace_back(getenv("HOME"));
        else
        {
            char* slash = new char[200];
            strcpy(slash, "/");
            strcat(slash, pch);
            vec.emplace_back(slash);
        }
        pch = strtok (NULL, delim);
    }
    char* absolute_path = new char[200];
    for (auto it = std::begin(vec); it!=std::end(vec); ++it)
    {
        strcat(absolute_path, *it);
    }

    /// @brief BASE_PATH (relative path -> absolute path)
    std::vector<char*> vec_base;
    char* pch_base = strtok(getenv("BASE_PATH"), delim);
    if ( pch_base == NULL )
    {
        char* root_tmp = new char[2];
        strcpy(root_tmp, "/");
        vec_base.emplace_back(root_tmp);
    }
    while (pch_base != NULL)
    {
        if ( strcmp(pch_base, ".") == 0 )
            vec_base.emplace_back(getenv("PWD"));
        else if ( strcmp(pch_base, "..") == 0 )
            vec_base.emplace_back(parent);
        else if( strcmp(pch_base, "~") == 0 )
            vec_base.emplace_back(getenv("HOME"));
        else
        {
            char* slash = new char[200];
            strcpy(slash, "/");
            strcat(slash, pch_base);
            vec_base.emplace_back(slash);
        }
        pch_base = strtok (NULL, delim);
    }
    char* absolute_path_base = new char[200];
    for (auto it = std::begin(vec_base); it!=std::end(vec_base); ++it)
    {
        strcat(absolute_path_base, *it);
    }

    std::cout << "\nBASE_PATH=>" << absolute_path_base << std::endl;
    std::cout << "\nABSOLUTE_PATH=>" << absolute_path << std::endl;

    return !(strstr(absolute_path, absolute_path_base) == nullptr);
}

int chdir(const char* path)
{
    puts("MY OWN CHDIR.");
    if ( !is_sub_directory(path) )
    {
        std::cerr << "access to " << path << " is not allowd." << std::endl;
        return -1;
    }
    LOAD_LIB(chdir);
    int rtn = old_chdir(path);
    return rtn;
}
int chmod(const char *pathname, mode_t mode)
{
    puts("MY OWN CHMOD.");
    if ( !is_sub_directory(pathname) )
    {
        std::cerr << "access to " << pathname << " is not allowd." << std::endl;
        return -1;
    }
    LOAD_LIB(chmod);
    int rtn = old_chmod(pathname, mode);
    return rtn;
}
int chown(const char *pathname, uid_t owner, gid_t group)
{
    puts("MY OWN CHOWN.");
    if ( !is_sub_directory(pathname) )
    {
        std::cerr << "access to " << pathname << " is not allowd." << std::endl;
        return -1;
    }
    LOAD_LIB(chown);
    int rtn = old_chown(pathname, owner, group);
    return rtn;
}
int creat(const char *pathname, mode_t mode)
{
    puts("MY OWN CREAT.");
    if ( !is_sub_directory(pathname) )
    {
        std::cerr << "access to " << pathname << " is not allowd." << std::endl;
        return -1;
    }
    LOAD_LIB(creat);
    int rtn = old_creat(pathname, mode);
    return rtn;
}
FILE* fopen(const char *pathname, const char *mode)
{
    puts("MY OWN FOPEN.");
    if ( !is_sub_directory(pathname) )
    {
        std::cerr << "access to " << pathname << " is not allowd." << std::endl;
        return nullptr;
    }
    LOAD_LIB(fopen);
    FILE* rtn = old_fopen(pathname, mode);
    return rtn;
}
// FILE* fdopen(int fd, const char *mode)
// {
//     puts("MY OWN FDOPEN");
//     if ( !is_sub_directory(pathname) )
//     {
//         std::cerr << "access to " << pathname << " is not allowd." << std::endl;
//         return nullptr;
//     }
//     LOAD_LIB(fopen);
//     FILE* rtn = old_fopen(pathname, mode);
//     return rtn;
// }
int link(const char *oldpath, const char *newpath)
{
    puts("MY OWN LINK.");
    if ( !is_sub_directory(oldpath) )
    {
        std::cerr << "access to " << oldpath << " is not allowd." << std::endl;
        return -1;
    }
    if ( !is_sub_directory(newpath) )
    {
        std::cerr << "access to " << newpath << " is not allowd." << std::endl;
        return -1;
    }
    LOAD_LIB(link);
    int rtn = old_link(oldpath, newpath);
    return rtn;
}
int mkdir(const char *pathname, mode_t mode)
{
    puts("MY OWN MKDIR.");
    if ( !is_sub_directory(pathname) )
    {
        std::cerr << "access to " << pathname << " is not allowd." << std::endl;
        return -1;
    }
    LOAD_LIB(mkdir);
    int rtn = old_mkdir(pathname, mode);
    return rtn;
}
int open(const char *pathname, int flags, mode_t mode)
{
    puts("MY OWN OPEN.");
    if ( !is_sub_directory(pathname) )
    {
        std::cerr << "access to " << pathname << " is not allowd." << std::endl;
        return -1;
    }
    LOAD_LIB(open);
    int rtn = old_open(pathname, flags, mode);
    return rtn;
}
int openat(int dirfd, const char *pathname, int flags, mode_t mode)
{
    puts("MY OWN OPENAT.");
    if ( !is_sub_directory(pathname) )
    {
        std::cerr << "access to " << pathname << " is not allowd." << std::endl;
        return -1;
    }
    LOAD_LIB(openat);
    int rtn = old_openat(dirfd, pathname, flags, mode);
    return rtn;
}
DIR* opendir(const char *name)
{
    puts("MY OWN OPENDIR.");
    if ( !is_sub_directory(name) )
    {
        std::cerr << "access to " << name << " is not allowd." << std::endl;
        return nullptr;
    }
    LOAD_LIB(opendir);
    DIR* rtn = old_opendir(name);
    return rtn;
}
ssize_t readlink(const char *pathname, char *buf, size_t bufsiz)
{
    puts("MY OWN READLINK.");
    if ( !is_sub_directory(pathname) )
    {
        std::cerr << "access to " << pathname << " is not allowd." << std::endl;
        return -1;
    }
    LOAD_LIB(readlink);
    ssize_t rtn = old_readlink(pathname, buf, bufsiz);
    return rtn;
}
int remove(const char *pathname)
{
    puts("MY OWN REMOVE.");
    if ( !is_sub_directory(pathname) )
    {
        std::cerr << "access to " << pathname << " is not allowd." << std::endl;
        return -1;
    }
    LOAD_LIB(remove);
    int rtn = old_remove(pathname);
    return rtn;
}
int rename(const char *oldpath, const char *newpath)
{
    puts("MY OWN RENAME.");
    if ( !is_sub_directory(oldpath) )
    {
        std::cerr << "access to " << oldpath << " is not allowd." << std::endl;
        return -1;
    }
    if ( !is_sub_directory(newpath) )
    {
        std::cerr << "access to " << newpath << " is not allowd." << std::endl;
        return -1;
    }
    LOAD_LIB(rename);
    int rtn = old_rename(oldpath, newpath);
    return rtn;
}
int rmdir(const char *pathname)
{
    puts("MY OWN RMDIR.");
    if ( !is_sub_directory(pathname) )
    {
        std::cerr << "access to " << pathname << " is not allowd." << std::endl;
        return -1;
    }
    LOAD_LIB(rmdir);
    int rtn = old_rmdir(pathname);
    return rtn;
}
int __xstat(int ver, const char *pathname, struct stat *statbuf)
{
    puts("MY OWN __XSTAT.");
    if ( !is_sub_directory(pathname) )
    {
        std::cerr << "access to " << pathname << " is not allowd." << std::endl;
        return -1;
    }
    LOAD_LIB(__xstat);
    int rtn = old___xstat(ver, pathname, statbuf);
    return rtn;
}
int symlink(const char *target, const char *linkpath)
{
    puts("MY OWN SYMLINK.");
    if ( !is_sub_directory(target) )
    {
        std::cerr << "access to " << target << " is not allowd." << std::endl;
        return -1;
    }
    if ( !is_sub_directory(linkpath) )
    {
        std::cerr << "access to " << linkpath << " is not allowd." << std::endl;
        return -1;
    }
    LOAD_LIB(symlink);
    int rtn = old_symlink(target, linkpath);
    return rtn;
}
int unlink(const char *pathname)
{
    puts("MY OWN UNLINK.");
    if ( !is_sub_directory(pathname) )
    {
        std::cerr << "access to " << pathname << " is not allowd." << std::endl;
        return -1;
    }
    LOAD_LIB(unlink);
    int rtn = old_unlink(pathname);
    return rtn;
}
int execl(const char *pathname, const char *arg, .../* (char  *) NULL */)
{
    puts("MY OWN EXECL.");
    return -82193;
    if ( !is_sub_directory(pathname) )
    {
        std::cerr << "access to " << pathname << " is not allowd." << std::endl;
        return -1;
    }
    /// @brief cannot pass ... as argument!~~~~~~~~~~~~~~
    /// @brief use va_list
    int count = 0;
    va_list args;
    va_start(args, arg);
    char* tmp;
    while( (tmp = va_arg(args, char*)) != (char*)NULL)
    {
        ++count;
        printf("count:%d, %s\n", count, tmp);
    }
    LOAD_LIB(execl);
    // int rtn = old_execl(pathname, arg, ...);
    int rtn = old_execl(pathname, arg);
    return rtn;
}
// int execle(const char *pathname, const char *arg, char *const envp[], .../*, (char *) NULL */)
// {
    // puts("MY OWN EXECLE.");
//     return -1;
//     if ( !is_sub_directory(pathname) )
//     {
//     std::cerr << "access to " << pathname << " is not allowd." << std::endl;
//     return -1;
//     }
//     /// @brief cannot pass ... as argument!~~~~~~~~~~~~~~
//     /// @brief use va_list
//     int count = 0;  
//     va_list args;
//     va_start(args, envp);
//     char* tmp;
//     while ( (tmp = va_arg(args, char*)) != (char*)NULL )
//     {
//         ++count;
//         printf("count:%d, %s\n", count, tmp);
//     }

//     LOAD_LIB(execle);
//     int rtn = old_execle(pathname, arg, (char*)NULL, envp);
//     return rtn;
// }
int execlp(const char *file, const char *arg, .../* (char  *) NULL */)
{
    puts("MY OWN EXECLP.");
    return -1;
    if ( !is_sub_directory(file) )
    {
        std::cerr << "access to " << file << " is not allowd." << std::endl;
        return -1;
    }
    /// @brief cannot pass ... as argument!~~~~~~~~~~~~~~
    /// @brief use va_list
    LOAD_LIB(execlp);
    // int rtn = old_execlp(file, arg, ...);
    int rtn = old_execlp(file, arg);
    return rtn;
}
int execv(const char *pathname, char *const argv[])
{
    puts("MY OWN EXECV.");
    return -1;
    if ( !is_sub_directory(pathname) )
    {
        std::cerr << "access to " << pathname << " is not allowd." << std::endl;
        return -1;
    }
    LOAD_LIB(execv);
    int rtn = old_execv(pathname, argv);
    return rtn;
}
// int execve(const char *pathname, char *const argv[], char *const envp[])
// {
    // puts("MY OWN EXECVE.");
//     if ( !is_sub_directory(pathname) )
//     {
//         std::cerr << "access to " << pathname << " is not allowd." << std::endl;
//         return -1;
//     }
//     LOAD_LIB(execve);
//     int rtn = old_execve(pathname, argv, envp);
//     return rtn;
// }
int execvp(const char *file, char *const argv[])
{
    puts("MY OWN EXECVP.");
    if ( !is_sub_directory(file) )
    {
        std::cerr << "access to " << file << " is not allowd." << std::endl;
        return -1;
    }
    LOAD_LIB(execvp);
    int rtn = old_execvp(file, argv);
    return rtn;
}
int system(const char *command)
{
    puts("MY OWN SYSTEM.");
    LOAD_LIB(system);
    int rtn = old_system(command);
    return rtn;
}