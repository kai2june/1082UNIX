#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#include <map>
#include <string>
#include <type_traits>
#include <dlfcn.h>
#include <stdarg.h>
#include <iostream>

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

void init() __attribute__((constructor));
void init()
{
    setenv("SO_PATH", "./sandbox.so", 1);
    setenv("BASE_PATH", ".", 1);
}

int chdir(const char* path)
{
    LOAD_LIB(chdir);
    int rtn = old_chdir(path);
    return rtn;
}
int chmod(const char *pathname, mode_t mode)
{
    // std::cout << getenv("SO_PATH") << ", " << getenv("BASE_PATH") << std::endl;
    LOAD_LIB(chmod);
    int rtn = old_chmod(pathname, mode);
    return rtn;
}
int chown(const char *pathname, uid_t owner, gid_t group)
{
    LOAD_LIB(chown);
    int rtn = old_chown(pathname, owner, group);
    return rtn;
}
int creat(const char *pathname, mode_t mode)
{
    LOAD_LIB(creat);
    int rtn = old_creat(pathname, mode);
    return rtn;
}
FILE* fopen(const char *pathname, const char *mode)
{
    LOAD_LIB(fopen);
    FILE* rtn = old_fopen(pathname, mode);
    return rtn;
}
int link(const char *oldpath, const char *newpath)
{
    LOAD_LIB(link);
    int rtn = old_link(oldpath, newpath);
    return rtn;
}
int mkdir(const char *pathname, mode_t mode)
{
    LOAD_LIB(mkdir);
    int rtn = old_mkdir(pathname, mode);
    return rtn;
}
int open(const char *pathname, int flags, mode_t mode)
{
    LOAD_LIB(open);
    int rtn = old_open(pathname, flags, mode);
    return rtn;
}
int openat(int dirfd, const char *pathname, int flags, mode_t mode)
{
    LOAD_LIB(openat);
    int rtn = old_openat(dirfd, pathname, flags, mode);
    return rtn;
}
DIR* opendir(const char *name)
{
    LOAD_LIB(opendir);
    DIR* rtn = old_opendir(name);
    return rtn;
}
ssize_t readlink(const char *pathname, char *buf, size_t bufsiz)
{
    LOAD_LIB(readlink);
    ssize_t rtn = old_readlink(pathname, buf, bufsiz);
    return rtn;
}
int remove(const char *pathname)
{
    LOAD_LIB(remove);
    int rtn = old_remove(pathname);
    return rtn;
}
int rename(const char *oldpath, const char *newpath)
{
    LOAD_LIB(rename);
    int rtn = old_rename(oldpath, newpath);
    return rtn;
}
int rmdir(const char *pathname)
{
    LOAD_LIB(rmdir);
    int rtn = old_rmdir(pathname);
    return rtn;
}
int __xstat(int ver, const char *pathname, struct stat *statbuf)
{
    LOAD_LIB(__xstat);
    int rtn = old___xstat(ver, pathname, statbuf);
    return rtn;
}
int symlink(const char *target, const char *linkpath)
{
    LOAD_LIB(symlink);
    int rtn = old_symlink(target, linkpath);
    return rtn;
}
int unlink(const char *pathname)
{
    LOAD_LIB(unlink);
    int rtn = old_unlink(pathname);
    return rtn;
}
int execl(const char *pathname, const char *arg, .../* (char  *) NULL */)
{
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
    /// @brief cannot pass ... as argument!~~~~~~~~~~~~~~
    /// @brief use va_list
    LOAD_LIB(execlp);
    // int rtn = old_execlp(file, arg, ...);
    int rtn = old_execlp(file, arg);
    return rtn;
}
int execv(const char *pathname, char *const argv[])
{
    LOAD_LIB(execv);
    int rtn = old_execv(pathname, argv);
    return rtn;
}
int execve(const char *pathname, char *const argv[], char *const envp[])
{
    LOAD_LIB(execve);
    int rtn = old_execve(pathname, argv, envp);
    return rtn;
}
int execvp(const char *file, char *const argv[])
{
    LOAD_LIB(execvp);
    int rtn = old_execvp(file, argv);
    return rtn;
}
int system(const char *command)
{
    LOAD_LIB(system);
    int rtn = old_system(command);
    return rtn;
}
