#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include <cctype>
#include <algorithm>
#include <vector>
#include <string>
#include <sys/stat.h>
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
                std::cout << str << std::endl;
                if ( stat(str.c_str(), &sb) == -1 )
                {
                    perror("stat");
                    exit(EXIT_FAILURE);
                }
                // std::cout << "sb : " << sb.st_mode << std::endl << "S_IFMT : " << S_IFMT <<std::endl;
                // std::cout << "sb.st_mode & S_IFMT : " << (sb.st_mode & S_IFMT) << std::endl;
                // std::cout << "S_IFLNK : " << S_IFLNK << std::endl;
                if ( (sb.st_mode & S_IFMT) == S_IFSOCK )
                    std::cout << "this is a socket." << std::endl;
            }
        }
        std::cout << std::endl;
    }


    // struct stat sb;
    // for i : b 
    //     if sb.st_mode & SIFMT == S_IFLNK
    //          read_link()


    return 0;
}