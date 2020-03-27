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


enum class CONNECTION
{
    TCP
  , UDP
};


std::vector<char*> parse_inodes(const CONNECTION& connection_type)
{
    std::FILE* fp;
    std::vector<char*> inodes;
    if ( connection_type == CONNECTION::TCP )
    {
        fp = fopen("/proc/net/tcp", "r");
    }
    char buf[1000];
    if(!fp)
    {
        perror("file opening failed");
        exit(EXIT_FAILURE);
    }
    fgets(buf, sizeof buf, fp);
    while(fgets(buf, sizeof buf, fp) != nullptr)
    {
        int cnt = 0;
        // std::cout << buf << std::endl;
        char* pch;
        pch = strtok(buf,  " ");
        while ( pch != nullptr )
        {
            // printf("%zu:\n%s\n", cnt, pch);
            if ( cnt == 9 )
            {
                char* inode = new char[strlen(pch) + 1];
                memcpy(inode, pch, strlen(pch) + 1);
                inodes.emplace_back(inode);
            }
            pch = strtok(nullptr, " ");
            ++cnt;
        }
    }
    if(feof(fp))
    {
        puts("end of file reached");
    }
    return inodes;
}


bool is_number(const char* s)
{
    for(int i=0; i<strlen(s); ++i)
    {
        if (!std::isdigit(s[i]))
            return false;
    }
    return true;
}


std::vector<char*> search_pid()
{
    std::vector<char*> proc_pid;
    DIR* dir;
    struct dirent* ent;

    if ( (dir = opendir("/proc")) != nullptr )
    {
        while ( (ent = readdir(dir)) != nullptr )
        {
            if ( is_number(ent->d_name) )
            {
                char* pid = new char[strlen(ent->d_name) + 1];
                memcpy(pid, ent->d_name, strlen(ent->d_name) + 1);
                proc_pid.emplace_back(pid);
            }
        }
        closedir(dir);
    } else {
        perror("");
        exit(EXIT_FAILURE);
    }
    return proc_pid;
}


void print_filetype(const struct stat& sb)
{
    switch (sb.st_mode & S_IFMT) {
        // case S_IFBLK:  printf("block device\n");            break;
        // case S_IFCHR:  printf("character device\n");        break;
        // case S_IFDIR:  printf("directory\n");               break;
        // case S_IFIFO:  printf("FIFO/pipe\n");               break;
        // case S_IFLNK:  printf("symlink\n");                 break;
        // case S_IFREG:  printf("regular file\n");            break;
        case S_IFSOCK: printf("socket\n");                  break;
        // default:       printf("unknown?\n");                break;
    }
}


int search_socketinode(const mode_t& file_mode, const char* path_to_symboliclink)
{
    std::cout << "sb.st_size: " << file_mode << std::endl;
    char* linkname;
    ssize_t r;
    if ( (file_mode & S_IFMT) == S_IFLNK )
    {
        linkname  = static_cast<char*>(malloc(file_mode + 100));
        if (linkname == NULL) {
            fprintf(stderr, "insufficient memory\n");
            exit(EXIT_FAILURE);
        }

        r = readlink(path_to_symboliclink, linkname, file_mode + 1);
        if (r < 0)
        {
            perror("stat");
            exit(EXIT_FAILURE);
        }
        if (r > file_mode+1 ) // this line should be if (r>file_mode) {...}, but it'll exit
        {
            fprintf(stderr, "symlink increased in size "
            "between lstat() and readlink()\n");
            exit(EXIT_FAILURE);
        }
        linkname[file_mode] = '\0';
        char linktype[7];
        strncpy(linktype, linkname, 6);
        linktype[strlen(linktype)] = '\0';
        if ( strcmp(linktype, "socket") == 0)
        { 
            printf("'%s' points to '%s'\n", path_to_symboliclink, linkname);
            int socket_inode = atoi(linkname+8);
            std::cout << "Yes, it's socket. socket inode : " << socket_inode << std::endl;
            return socket_inode;
    //         if ( match_inode(inodes, socket_inode) )
    //         {
    //             std::cout << "true" << std::endl;
    //             // (*it).erase((*it).end() - 3);
    //             // tcp_pid.emplace_back(*it);
    //         }
        }
    } // if ISLNK
    return -1;
}


bool is_tcpsocket()
{
    
}


// bool match_inode(const std::vector<char*>& inodes, const int& socket_inode)
// {
//     char s[20];
//     int cx = snprintf(s, 20, "%d", socket_inode);
//     // std::cout << s << std::endl;
//     for( auto it = inodes.begin(); it!=inodes.end(); ++it )
//     {
//         std::cout << *it << std::endl;
//         if ( strcmp(*it, s) == 0 )
//             return true;
//     }
//     // std::cout << "current: " << typeid(s).name() << std::endl;
//     return false;
// }


int main(int argc, char* argv[])
{
    //// crawl /proc/net/tcp
    std::vector<char*> inodes = parse_inodes(CONNECTION::TCP);
    puts("inodes");
    for( std::vector<char*>::iterator it = inodes.begin(); it != inodes.end(); ++it )
        std::cout << *it << std::endl;

    //// do something with pid socket
    std::vector<char*> proc_pid = search_pid();
    // for( std::vector<char*>::iterator it=proc_pid.begin(); it!=proc_pid.end(); ++it)
    //     std::cout << *it << std::endl;

    //// search socket_pid
    DIR* dir;
    struct dirent* ent;
    struct stat sb;
    std::vector<char*> tcp_pid;
    for( std::vector<char*>::iterator it = proc_pid.begin(); it!=proc_pid.end(); ++it)
    {
        char path_to_pid[300] = "/proc/";
        strcat(path_to_pid, *it);
        strcat(path_to_pid, "/fd/");
        // std::cout << path_to_pid << std::endl;
        if ( ( dir = opendir( path_to_pid ) ) != nullptr ) 
        {
            while ( (ent = readdir(dir)) != nullptr )
            {
                char* path_to_symboliclink = new char[strlen(path_to_pid) + strlen(ent->d_name) + 1];
                strcpy(path_to_symboliclink, path_to_pid);
                strcat(path_to_symboliclink, ent->d_name);
                std::cout << path_to_symboliclink << std::endl;

                if ( lstat(path_to_symboliclink, &sb) == -1 )
                {
                    perror("lstat");
                    exit(EXIT_FAILURE);
                }
                print_filetype(sb);

                int socket_inode = search_socketinode(sb.st_mode, path_to_symboliclink);
                bool tcp_socket;
                if ( socket > 0 )
                    tcp_socket = is_tcpsocket();
            } // while readdir
        } // if opendir
        std::cout << std::endl;
    }

    return 0;
}

// 接下來
// cat /proc/net/tcp以及cat/proc/net/udp看inode欄位有哪些inode
// 與我目前撈到的socket:[xxxxxxxx]比對，若xxxxxxxx == inode則撈/proc/[pid]/comm(得到program name)以及/proc/[pid]/cmdline(執行時下的指令)
// cat /proc/net/tcp的local_address與rem_address轉成十進位 xxx.xxx.xxx.xxx:xxxx