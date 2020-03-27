#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <cctype>
#include <algorithm>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include "hw1.hpp"

bool is_number(const std::string& s)
{
    return !s.empty() && 
        std::find_if(
            s.begin(), 
            s.end(), 
            [](unsigned char c) 
            { 
                return !std::isdigit(c); 
            }
        ) == s.end();
}

std::vector<std::string> search_pid()
{
    std::vector<std::string> proc_fd;
    DIR* dir;
    struct dirent* ent;

    if ( (dir = opendir("/proc")) != nullptr )
    {
        while ( (ent = readdir(dir)) != nullptr )
        {
            if ( is_number(ent->d_name) )
            {
                std::string str("/proc/");
                str.append(ent->d_name).append("/fd");
                proc_fd.emplace_back(str);
            }
        }
        closedir(dir);
    } else {
        perror("");
        exit(EXIT_FAILURE);
    }
    return proc_fd;
}

int main(int argc, char* argv[])
{
    std::vector<std::string> proc_fd = search_pid();
    DIR* dir;
    struct dirent* ent;
    struct stat sb;
    struct stat sb_socket;
    char* linkname;
    ssize_t r;

    for( std::vector<std::string>::iterator it = proc_fd.begin(); it!=proc_fd.end(); ++it)
    {
        if ( (dir = opendir( (*it).c_str() ) ) != nullptr ) 
        {
            while ( (ent = readdir(dir)) != nullptr )
            {
                std::string str(*it);
                str.append("/").append(ent->d_name);

                // if ( stat(str.c_str(), &sb_socket) == -1 )
                // {
                //     perror("stat");
                //     exit(EXIT_FAILURE);
                // }
                if ( lstat(str.c_str(), &sb) == -1 )
                {
                    perror("lstat");
                    exit(EXIT_FAILURE);
                }

                // switch (sb.st_mode & S_IFMT) {
                //     case S_IFBLK:  printf("block device\n");            break;
                //     case S_IFCHR:  printf("character device\n");        break;
                //     case S_IFDIR:  printf("directory\n");               break;
                //     case S_IFIFO:  printf("FIFO/pipe\n");               break;
                //     case S_IFLNK:  printf("symlink\n");                 break;
                //     case S_IFREG:  printf("regular file\n");            break;
                //     case S_IFSOCK: printf("socket\n");                  break;
                //     default:       printf("unknown?\n");                break;
                // }


                if ( (sb.st_mode & S_IFMT) == S_IFLNK )
                {
                    // std::cout << "This is a socket: " << str << std::endl;
                    r = readlink(str.c_str(), linkname, sb.st_size + 100);
                    if (r < 0)
                    {
                        perror("stat");
                        exit(EXIT_FAILURE);
                    }
                    if (r > (sb.st_size+100))
                    {
                        fprintf(stderr, "symlink increased in size "
                        "between lstat() and readlink()\n");
                        exit(EXIT_FAILURE);
                    }
                    linkname[sb.st_size] = '\0';
                    if ( linkname[0] == 's' && 
                         linkname[1] == 'o' &&
                         linkname[2] == 'c' &&
                         linkname[3] == 'k' &&
                         linkname[4] == 'e' &&
                         linkname[5] == 't'
                    )
                        printf("'%s' points to '%s'\n", str.c_str(), linkname);
                }
            }
        }
        std::cout << std::endl;
    }

    return 0;
}

// 接下來
// cat /proc/net/tcp以及cat/proc/net/udp看inode欄位有哪些inode
// 與我目前撈到的socket:[xxxxxxxx]比對，若xxxxxxxx == inode則撈/proc/[pid]/comm(得到program name)以及/proc/[pid]/cmdline(執行時下的指令)
// cat /proc/net/tcp的local_address與rem_address轉成十進位 xxx.xxx.xxx.xxx:xxxx