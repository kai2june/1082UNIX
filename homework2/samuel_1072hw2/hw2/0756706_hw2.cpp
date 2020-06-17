#include <map>
#include <string>
#include <type_traits>
#include <dlfcn.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define S_IUID 0007000

#define DECLARE_TYPE(name, prototype)\
    using name##_type = prototype;\
    name##_type old_##name = nullptr

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


DECLARE_TYPE(opendir, DIR* (*)(const char*));
DECLARE_TYPE(readdir, struct dirent* (*)(DIR*));
DECLARE_TYPE(closedir, int (*)(DIR*));
DECLARE_TYPE(creat, int (*)(const char*, mode_t));
DECLARE_TYPE(open, int (*)(const char*, int, ...));
DECLARE_TYPE(read, ssize_t (*)(int, void*, size_t));
DECLARE_TYPE(write, ssize_t (*)(int, const void*, size_t));
DECLARE_TYPE(close, int (*)(int));
DECLARE_TYPE(dup, int (*)(int));
DECLARE_TYPE(dup2, int (*)(int, int));
DECLARE_TYPE(__xstat, int (*)(int, const char*, struct stat*));
DECLARE_TYPE(__lxstat, int (*)(int, const char*, struct stat*));
DECLARE_TYPE(pwrite, ssize_t (*)(int, const void*, size_t, off_t));
DECLARE_TYPE(fopen, FILE* (*)(const char*, const char*));
DECLARE_TYPE(fdopen, FILE* (*)(int, const char*));
DECLARE_TYPE(fclose, int (*)(FILE*));
DECLARE_TYPE(fread, size_t (*)(void*, size_t, size_t, FILE*));
DECLARE_TYPE(fwrite, size_t (*)(const void*, size_t, size_t, FILE*));
DECLARE_TYPE(fgetc, int (*)(FILE*));
DECLARE_TYPE(fgets, char* (*)(char*, int, FILE*));
DECLARE_TYPE(fprintf, int (*)(FILE*, const char*, ...));
DECLARE_TYPE(chdir, int (*)(const char*));
DECLARE_TYPE(chown, int (*)(const char*, uid_t, gid_t));
DECLARE_TYPE(chmod, int (*)(const char*, mode_t));
DECLARE_TYPE(remove, int (*)(const char*));
DECLARE_TYPE(rename, int (*)(const char*, const char*));
DECLARE_TYPE(link, int (*)(const char*, const char*));
DECLARE_TYPE(unlink, int (*)(const char*));
DECLARE_TYPE(readlink, ssize_t (*)(const char*, char*, size_t));
DECLARE_TYPE(symlink, int (*)(const char*, const char*));
DECLARE_TYPE(mkdir, int (*)(const char*, mode_t));
DECLARE_TYPE(rmdir, int (*)(const char*));

// addtional functions
DECLARE_TYPE(pread, ssize_t (*)(int, void*, size_t, off_t));
DECLARE_TYPE(fflush, int(*)(FILE*));
DECLARE_TYPE(fputs_unlocked, int (*)(const char*, FILE*));
DECLARE_TYPE(fwrite_unlocked, size_t (*)(const void*, size_t, size_t, FILE*));

int stderr_fd = 2;
std::map<DIR*, std::string> open_list;
const size_t FILENAME_LEN = 256;

void init() __attribute__((constructor));
void init()
{
    char* output = getenv("MONITOR_OUTPUT");
    if (output == nullptr)
    {
        LOAD_LIB(dup);
        stderr_fd = old_dup(2);
    }
    else
    {
        LOAD_LIB(open);
        stderr_fd = old_open(output, O_RDWR | O_CREAT | O_TRUNC, 00644);
    }
}

template <class T>
std::string strcat(T val)
{
    if constexpr (std::is_same<const char*, T>::value)
        return std::string(val);
    return std::to_string(val);
}

template <class T, class... Us>
std::string strcat(T val, Us... vals)
{
    if constexpr (std::is_same<const char*, T>::value)
        return strcat<Us...>(vals...).insert(0, std::string(val));
    else
        return strcat<Us...>(vals...).insert(0, std::to_string(val));
}

std::string read_fd(int fd)
{
    if (fd == 0)
        return "<STDIN>";
    else if (fd == 1)
        return "<STDOUT>";
    else if (fd == 2)
        return "<STDERR>";
    else;
    char file_des[FILENAME_LEN];
    LOAD_LIB(readlink);
    size_t len = old_readlink(strcat<const char*, int>("/proc/self/fd/", fd).c_str(), file_des, FILENAME_LEN);
    if (len == -1)
        return std::string();
    return std::string(file_des, len);
}

std::string file_type(mode_t m)
{
    if (S_ISREG(m))
        return "Regular";
    else if (S_ISDIR(m))
        return "Directory";
    else if (S_ISCHR(m))
        return "Character_dev";
    else if (S_ISBLK(m))
        return "Block_dev";
    else if (S_ISFIFO(m))
        return "FIFO";
    else if (S_ISLNK(m))
        return "Link";
    else if (S_ISSOCK(m))
        return "Socket";
    return "Unknown";
}

DIR* opendir(const char* name)
{
    LOAD_LIB(opendir);
    DIR* rval = old_opendir(name);
    dprintf(stderr_fd, "# opendir(\"%s\") = [DIR*] %p\n", name, rval);
    open_list[rval] = std::string(name);
    return rval;
}

struct dirent* readdir(DIR* dirp)
{
    LOAD_LIB(readdir);
    struct dirent* rval = old_readdir(dirp);
    if (rval == nullptr)
        dprintf(stderr_fd, "# readdir(\"%s\") = [dirent*] 0x0 (EOF)\n", open_list[dirp].c_str());
    else
        dprintf(stderr_fd, "# readdir(\"%s\") = [dirent*] %s\n", open_list[dirp].c_str(), rval->d_name);
    return rval;
}

int closedir(DIR *dirp)
{
    LOAD_LIB(closedir);
    int rval = old_closedir(dirp);
    dprintf(stderr_fd, "# closedir(\"%s\") = [int] %i\n", open_list[dirp].c_str(), rval);
    open_list.erase(dirp);
    return rval;
}

int open(const char* pathname, int flags, ... )
{
    LOAD_LIB(open);
    va_list args;
    va_start(args, flags);
    mode_t mode = va_arg(args, mode_t);
    int mode_num = ((mode & S_IUID) >> 9) * 1000 + ((mode & S_IRWXU) >> 6) * 100 + ((mode & S_IRWXG) >> 3) * 10 + (mode & S_IRWXO);
    int rval = old_open(pathname, flags, mode);
    /*
    if ((flags & O_CREAT) == O_CREAT || (flags & O_TMPFILE) == O_TMPFILE)
        dprintf(stderr_fd, "# open(\"%s\", %i, %i) = [fd] %i (%s)\n", pathname, flags, mode_num, rval, read_fd(rval).c_str());
    else
        dprintf(stderr_fd, "# open(\"%s\", %i) = [fd] %i (%s)\n", pathname, flags, rval, read_fd(rval).c_str());
*/    
    va_end(args);
    return rval;
}

int creat(const char* pathname, mode_t mode)
{
    LOAD_LIB(creat);
    int rval = old_creat(pathname, mode);
    int mode_num = ((mode & S_IUID) >> 9) * 1000 + ((mode & S_IRWXU) >> 6) * 100 + ((mode & S_IRWXG) >> 3) * 10 + (mode & S_IRWXO);
    dprintf(stderr_fd, "# creat(\"%s\", %i) = [fd] %i (%s)\n", pathname, mode_num, rval, read_fd(rval).c_str());
    return rval;
}

ssize_t read(int fd, void* buf, size_t count)
{
    LOAD_LIB(read);
    ssize_t rval = old_read(fd, buf, count);
    dprintf(stderr_fd, "# read(\"%s\", %p, %lu) = [Byte] %lu\n", read_fd(fd).c_str(), buf, count, rval);
    return rval;
}

ssize_t write(int fd, const void* buf, size_t count)
{
    LOAD_LIB(write);
    ssize_t rval = old_write(fd, buf, count);
    dprintf(stderr_fd, "# write(\"%s\", %p, %lu) = [Byte] %lu\n", read_fd(fd).c_str(), buf, count, rval);
    return rval;
}

int close(int fd)
{
    LOAD_LIB(close);
    dprintf(stderr_fd, "# close(\"%s", read_fd(fd).c_str());
    ssize_t rval = old_close(fd);
    dprintf(stderr_fd, "\") = [int] %lu\n", rval);
    return rval;
}

int dup(int oldfd)
{
    LOAD_LIB(dup);
    dprintf(stderr_fd, "# dup(\"%s", read_fd(oldfd).c_str());
    int rval = old_dup(oldfd);
    dprintf(stderr_fd, "\") = [fd] %i (%s)\n", rval, read_fd(rval).c_str());
    return rval;
}

int dup2(int oldfd, int newfd)
{
    LOAD_LIB(dup2);
    dprintf(stderr_fd, "# dup2(\"%s\", %i (%s) ", read_fd(oldfd).c_str(), newfd, read_fd(newfd).c_str());
    int rval = old_dup2(oldfd, newfd);
    dprintf(stderr_fd, ") = [fd] %i (%s)\n", rval, read_fd(rval).c_str());
    return rval;
}

int __xstat(int ver, const char* path, struct stat* buf)
{
    LOAD_LIB(__xstat);
    int rval = old___xstat(ver, path, buf);
    mode_t mode = buf->st_mode;
    int mode_num = ((mode & S_IUID) >> 9) * 1000 + ((mode & S_IRWXU) >> 6) * 100 + ((mode & S_IRWXG) >> 3) * 10 + (mode & S_IRWXO);
    dprintf(stderr_fd, "# stat(\"%s\", %p", path, buf);
    dprintf(stderr_fd, " { type=%s, mode=%i, uid=%u, size=%lu }) = [int] %i\n", file_type(buf->st_mode).c_str(), mode_num, buf->st_uid, buf->st_size, rval);
    return rval;
}


int __lxstat(int ver, const char* path, struct stat* buf)
{
    LOAD_LIB(__lxstat);
    int rval = old___lxstat(ver, path, buf);
    mode_t mode = buf->st_mode;
    int mode_num = ((mode & S_IUID) >> 9) * 1000 + ((mode & S_IRWXU) >> 6) * 100 + ((mode & S_IRWXG) >> 3) * 10 + (mode & S_IRWXO);
    dprintf(stderr_fd, "# lstat(\"%s\", %p", path, buf);
    dprintf(stderr_fd, " { type=%s, mode=%i, uid=%u, size=%lu }) = [int] %i\n", file_type(buf->st_mode).c_str(), mode_num, buf->st_uid, buf->st_size, rval);
    return rval;
}

ssize_t pwrite(int fd, const void* buf, size_t count, off_t offset)
{
    LOAD_LIB(pwrite);
    ssize_t rval = old_pwrite(fd, buf, count, offset);
    dprintf(stderr_fd, "# pwrite(\"%s\", %p, %lu, %lu) - [Bytes] %lu\n", read_fd(fd).c_str(), buf, count, offset, rval);
    return rval;
}

FILE* fopen(const char* pathname, const char* mode)
{
    LOAD_LIB(fopen);
    FILE* rval = old_fopen(pathname, mode);
    dprintf(stderr_fd, "# fopen(\"%s\", \"%s\") = [FILE*] %p (%s)\n", pathname, mode, rval, read_fd(fileno(rval)).c_str());
    return rval;
}

FILE* fdopen(int fd, const char* mode)
{
    LOAD_LIB(fdopen);
    FILE* rval = old_fdopen(fd, mode);
    dprintf(stderr_fd, "# fdopen(\"%s\", \"%s\") = [FILE*] %p (%s)\n", read_fd(fd).c_str(), mode, rval, read_fd(fileno(rval)).c_str());
    return rval;
}

int fclose(FILE* stream)
{
    LOAD_LIB(fclose);
    dprintf(stderr_fd, "# fclose(\"%s\") = [int] ", read_fd(fileno(stream)).c_str());
    int rval = old_fclose(stream);
    dprintf(stderr_fd, "%i\n", rval);
    return rval;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE* stream)
{
    LOAD_LIB(fread);
    int rval = old_fread(ptr, size, nmemb, stream);
    dprintf(stderr_fd, "# fread(%p, %lu, %lu, \"%s\") = [size_t] %u\n", ptr, size, nmemb, read_fd(fileno(stream)).c_str(), rval);
    return rval;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE* stream)
{
    LOAD_LIB(fwrite);
    int rval = old_fwrite(ptr, size, nmemb, stream);
    dprintf(stderr_fd, "# fwrite(%p, %lu, %lu, \"%s\") = [size_t] %u\n", ptr, size, nmemb, read_fd(fileno(stream)).c_str(), rval);
    return rval;
}

int fgetc(FILE* stream)
{
    LOAD_LIB(fgetc);
    int rval = old_fgetc(stream);
    if (rval == EOF)
        dprintf(stderr_fd, "# fgetc(\"%s\") = [char] -1 (EOF)\n", read_fd(fileno(stream)).c_str());
    else
        dprintf(stderr_fd, "# fgetc(\"%s\") = [char] %c\n", read_fd(fileno(stream)).c_str(), (char)rval);
    return rval;
}

char* fgets(char* s, int size, FILE* stream)
{
    LOAD_LIB(fgets);
    char* rval = old_fgets(s, size, stream);
    if (rval == nullptr)
        dprintf(stderr_fd, "# fgets(%p, %i, \"%s\") = [char*] NULL (EOF or Error)\n", s, size, read_fd(fileno(stream)).c_str());
    else
        dprintf(stderr_fd, "# fgets(%p, %i, \"%s\") = [char*] %s\n", s, size, read_fd(fileno(stream)).c_str(), std::string(rval, size).c_str());
    return rval;
}

int fscanf(FILE* stream, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int rval = vfscanf(stream, format, args);
    dprintf(stderr_fd, "# fscanf(\"%s\", \"%s\", ...) = [int] %i\n", read_fd(fileno(stream)).c_str(), format, rval);
    va_end(args);
    return rval;
}

int fprintf(FILE* stream, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int rval = vfprintf(stream, format, args);
    dprintf(stderr_fd, "# fprintf(\"%s\", \"%s\", ...) = [int] %i\n", read_fd(fileno(stream)).c_str(), format, rval);
    va_end(args);
    return rval;
}

// don't use std::cerr, it will call back to fwrite

int chdir(const char* path)
{
    LOAD_LIB(chdir);
    int rval = old_chdir(path);
    dprintf(stderr_fd, "# chdir(\"%s\") = [int] %i\n", path, rval);
    return rval;
}

int chown(const char* pathname, uid_t owner, gid_t group)
{
    LOAD_LIB(chown);
    int rval = old_chown(pathname, owner, group);
    dprintf(stderr_fd, "# chown(\"%s\", %u, %u) = [int] %i\n", pathname, owner, group, rval);
    return rval;
}

int chmod(const char* pathname, mode_t mode)
{
    LOAD_LIB(chmod);
    int rval = old_chmod(pathname, mode);
    int mode_num = ((mode & S_IUID) >> 9) * 1000 + ((mode & S_IRWXU) >> 6) * 100 + ((mode & S_IRWXG) >> 3) * 10 + (mode & S_IRWXO);
    dprintf(stderr_fd, "# chmod(\"%s\", %u) = [int] %i\n", pathname, mode_num, rval);
    return rval;
}

int remove(const char* pathname)
{
    LOAD_LIB(remove);
    int rval = old_remove(pathname);
    dprintf(stderr_fd, "# remove(\"%s\") = [int] %i\n", pathname, rval);
    return rval;
}

int rename(const char* oldpath, const char* newpath)
{
    LOAD_LIB(rename);
    int rval = old_rename(oldpath, newpath);
    dprintf(stderr_fd, "# rename(\"%s\", \"%s\") = [int] %i\n", oldpath, newpath, rval);
    return rval;
}

int link(const char* oldpath, const char* newpath)
{
    LOAD_LIB(link);
    int rval = old_link(oldpath, newpath);
    dprintf(stderr_fd, "# link(\"%s\", \"%s\") = [int] %i\n", oldpath, newpath, rval);
    return rval;
}

int unlink(const char* pathname)
{
    LOAD_LIB(unlink);
    int rval = old_unlink(pathname);
    dprintf(stderr_fd, "# unlink(\"%s\") = [int] %i\n", pathname, rval);
    return rval;
}

ssize_t readlink(const char* pathname, char *buf, size_t bufsiz)
{
    LOAD_LIB(readlink);
    ssize_t rval = old_readlink(pathname, buf, bufsiz);
    dprintf(stderr_fd, "# readlink(\"%s\", %p, %lu) = [Bytes] %lu\n", pathname, buf, bufsiz, rval);
    return rval;
}

int symlink(const char* target, const char* linkpath)
{
    LOAD_LIB(symlink);
    int rval = old_symlink(target, linkpath);
    dprintf(stderr_fd, "# symlink(\"%s\", \"%s\") = [int] %i\n", target, linkpath, rval);
    return rval;
}

int mkdir(const char* pathname, mode_t mode)
{
    LOAD_LIB(mkdir);
    int rval = old_mkdir(pathname, mode);
    int mode_num = ((mode & S_IUID) >> 9) * 1000 + ((mode & S_IRWXU) >> 6) * 100 + ((mode & S_IRWXG) >> 3) * 10 + (mode & S_IRWXO);
    dprintf(stderr_fd, "# mkdir(\"%s\", %i) = [int] %i\n", pathname, mode_num, rval);
    return rval;
}

int rmdir(const char* pathname)
{
    LOAD_LIB(rmdir);
    int rval = old_rmdir(pathname);
    dprintf(stderr_fd, "# rmdir(\"%s\") = [int] %i\n", pathname, rval);
    return rval;
}

ssize_t pread(int fd, void* buf, size_t count, off_t offset)
{
    LOAD_LIB(pread);
    ssize_t rval = old_pread(fd, buf, count, offset);
    dprintf(stderr_fd, "# pread(\"%s\", %p, %lu, %lu) = [Bytes] %lu\n", read_fd(fd).c_str(), buf, count, offset, rval);
    return rval;
}

int fflush(FILE* stream)
{
    LOAD_LIB(fflush);
    int rval = old_fflush(stream);
    dprintf(stderr_fd, "# fflush(\"%s\") = [int] %i\n", read_fd(fileno(stream)).c_str(), rval);
    return rval;
}

int fputs_unlocked(const char* s, FILE* stream)
{
    LOAD_LIB(fputs_unlocked);
    int rval = old_fputs_unlocked(s, stream);
    dprintf(stderr_fd, "# fputs_unlocked(\"%s\", \"%s\") = [int] %i\n", s, read_fd(fileno(stream)).c_str(), rval);
    return rval;
}

size_t fwrite_unlocked(const void* ptr, size_t size, size_t n, FILE* stream)
{
    LOAD_LIB(fwrite_unlocked);
    int rval = old_fwrite_unlocked(ptr, size, n, stream);
    dprintf(stderr_fd, "# fwrite_unlocked(%p, %lu, %lu, \"%s\") = [int] %i\n", ptr, size, n, read_fd(fileno(stream)).c_str(), rval);
    return rval;
}
