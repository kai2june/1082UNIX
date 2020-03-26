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

int main(int argc, char* argv[])
{
    std::vector<std::string> proc_fd;
    DIR* dir;
    struct dirent* ent;
    struct stat sb;
    struct stat sb_socket;
    char* linkname;
    ssize_t r;

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
        return EXIT_FAILURE;
    }
    
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



                // std::cout << sb.st_size << std::endl;
                // std::cout << "sb : " << sb.st_mode << std::endl << "S_IFMT : " << S_IFMT <<std::endl;
                // std::cout << "sb.st_mode & S_IFMT : " << (sb.st_mode & S_IFMT) << std::endl;
                // std::cout << "S_IFLNK : " << S_IFLNK << std::endl;
                switch (sb.st_mode & S_IFMT) {
                    case S_IFBLK:  printf("block device\n");            break;
                    case S_IFCHR:  printf("character device\n");        break;
                    case S_IFDIR:  printf("directory\n");               break;
                    case S_IFIFO:  printf("FIFO/pipe\n");               break;
                    case S_IFLNK:  printf("symlink\n");                 break;
                    case S_IFREG:  printf("regular file\n");            break;
                    case S_IFSOCK: printf("socket\n");                  break;
                    default:       printf("unknown?\n");                break;
                }


                if ( (sb.st_mode & S_IFMT) == S_IFLNK )
                {
                    std::cout << "This is a socket: " << str << std::endl;
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
                    printf("'%s' points to '%s'\n", str.c_str(), linkname);
                    // exit(EXIT_SUCCESS);
                }
            }
        }
        std::cout << std::endl;
    }

    return 0;
}