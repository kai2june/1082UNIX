#include <iostream>
#include <fstream>
#include <cctype>
#include <algorithm>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>


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

bool match_inode(const std::vector<char*>& inodes, int& socket_inode)
{
    std::string s = std::to_string(socket_inode);
    // s = s.c_str();
    for( auto it = inodes.begin(); it!=inodes.end(); ++it )
    {
        std::cout << *it << std::endl;
        if ( strcmp(*it, s.c_str()) == 0 )
            return true;
    }
    // std::cout << "current: " << typeid(s.c_str()).name() << std::endl;
    return false;
}

int main(int argc, char* argv[])
{
    // crawl /proc/net/tcp
    std::FILE* fp = fopen("/proc/net/tcp", "r");
    char buf[1000];
    std::vector<char*> inodes;
    if(!fp)
    {
        perror("file opening failed");
        return EXIT_FAILURE;
    }
    fgets(buf, sizeof buf, fp);
    while(fgets(buf, sizeof buf, fp) != nullptr)
    {
        std::size_t cnt = 0;
        // std::cout << buf << std::endl;
        char* pch;
        pch = strtok(buf,  " ");
        while ( pch != nullptr )
        {
            // printf("%zu:\n%s\n", cnt, pch);
            if ( cnt == 9 )
            {
                char* tmp = new char[strlen(pch) + 1];
                memcpy(tmp, pch, strlen(pch) + 1);
                inodes.emplace_back(tmp);
            }
            pch = strtok(nullptr, " ");
            ++cnt;
        }
    }
    if(feof(fp))
    {
        // puts("end of file reached");
    }
    puts("inodes");
    for( std::vector<char*>::iterator it = inodes.begin(); it != inodes.end(); ++it )
        std::cout << *it << std::endl;

    std::vector<std::string> proc_fd = search_pid();
    DIR* dir;
    struct dirent* ent;
    struct stat sb;
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

                if ( lstat(str.c_str(), &sb) == -1 )
                {
                    perror("lstat");
                    exit(EXIT_FAILURE);
                }

                if ( (sb.st_mode & S_IFMT) == S_IFLNK )
                {
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
                    { 
                        printf("'%s' points to '%s'\n", str.c_str(), linkname);
                        int socket_inode = atoi(linkname+8);
                        std::cout << "inode : " << socket_inode << std::endl;
                        std::cout << match_inode(inodes, socket_inode) << std::endl;
                    }
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