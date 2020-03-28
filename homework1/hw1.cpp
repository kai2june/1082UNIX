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
#include <arpa/inet.h>
#include <netinet/in.h>

#define CHAR_ARRAY_MAX_SIZE 1000

enum class CONNECTION
{
    TCP
  , UDP
  , TCP6
  , UDP6
};

enum class ADDRESS
{
    LOCAL
  , REMOTE
};

enum class TCP_UDP_FIELD
{
    SL
  , LOCAL_ADDRESS
  , REMOTE_ADDRESS
  , ST
  , TX_QUEUE_RX_QUEUE
  , TR_TM_WHEN
  , RETRNSMT
  , UID
  , TIMEOUT
  , INODE
  , REF
  , POINTER
  , DROPS
};

std::vector<std::pair<char*, char*>> split_colon(const std::vector<char*>& address)
{
    std::vector<std::pair<char*, char*>> ip_port;
    std::pair<char*, char*> ip_port_oneline;
    for( std::vector<char*>::const_iterator it = std::begin(address); it != std::end(address); ++it )
    {
        char* pch;
        pch = strchr(*it, ':');
        std::cout << "found at "<< pch-(*it) << std::endl;
        
        // char* ip = new char[pch-(*it)+1];
        char* ip = new char[CHAR_ARRAY_MAX_SIZE];
        strncpy(ip, *it, pch-(*it));
        std::cout << "ip: " << ip << std::endl;
        // char* port = new char[ sizeof(*it) - (pch-(*it)) + 1];
        char* port = new char[CHAR_ARRAY_MAX_SIZE];
        strncpy(port, (*it) + (pch-(*it)) + 1, strlen(*it) - (pch-(*it)) - 1);
        // std::cout << "port: " << port << std::endl;
        ip_port_oneline.first = ip;
        ip_port_oneline.second = port;
        ip_port.emplace_back(ip_port_oneline);
    }
    return ip_port;
}


std::vector<char*> parse_TCPUDPFIELD(const CONNECTION& connection_type, const TCP_UDP_FIELD& field)
{
    std::FILE* fp;
    std::vector<char*> inodes;
    if ( connection_type == CONNECTION::TCP )
    {
        fp = fopen("/proc/net/tcp", "r"); 
    } else if ( connection_type == CONNECTION::UDP ) {
        fp = fopen("/proc/net/udp", "r");
    } else if ( connection_type == CONNECTION::TCP6 ) {
        fp = fopen("/proc/net/tcp6", "r");
    } else if ( connection_type == CONNECTION::UDP6 ) {
        fp = fopen("/proc/net/udp6", "r");
    }

    char buf[CHAR_ARRAY_MAX_SIZE];
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
            if ( cnt == static_cast<int>(field) )
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
    // std::cout << "sb.st_size: " << file_mode << std::endl;
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
            // printf("'%s' points to '%s'\n", path_to_symboliclink, linkname);
            int socket_inode = atoi(linkname+8);
            // std::cout << "Yes, it's socket. socket inode : " << socket_inode << std::endl;
            return socket_inode;
        }
    } // if ISLNK
    return -1;
}


bool is_tcpsocket(const std::vector<char*>& inodes, const int& socket_inode)
{
    char s[20];
    int cx = snprintf(s, 20, "%d", socket_inode);
    // std::cout << s << std::endl;
    for ( std::vector<char*>::const_iterator it = std::begin(inodes); it != std::end(inodes); ++it )
    {
        // std::cout << *it <<std::endl;
        if ( strcmp(*it, s) == 0 )
            return true;
    }
    return false;
}


std::vector<char*> search_tcppid(const std::vector<char*>& proc_pid, const std::vector<char*>& inodes)
{
    DIR* dir;
    struct dirent* ent;
    struct stat sb;
    std::vector<char*> tcp_pid;
    for( std::vector<char*>::const_iterator it = std::begin(proc_pid); it!=std::end(proc_pid); ++it)
    {
        char path_to_pid[CHAR_ARRAY_MAX_SIZE] = "/proc/";
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
                // std::cout << path_to_symboliclink << std::endl;

                if ( lstat(path_to_symboliclink, &sb) == -1 )
                {
                    perror("lstat");
                    exit(EXIT_FAILURE);
                }
                print_filetype(sb);

                int socket_inode = search_socketinode(sb.st_mode, path_to_symboliclink);
                bool tcp_socket = false;
                if ( socket_inode > 0 )
                {
                    tcp_socket = is_tcpsocket(inodes, socket_inode);
                }
                // std::cout << "is this a tcp socket? " << tcp_socket << std::endl;
                if ( tcp_socket )
                {
                    tcp_pid.emplace_back(*it);
                }
            } // while readdir
        } // if opendir
        std::cout << std::endl;
    } // for proc_pid
    return tcp_pid;
}


std::vector<char*> search_cmdline(const std::vector<char*>& tcp_pid)
{
    std::vector<char*> proc_cmdline;
    std::FILE* cmd_file;
    for ( std::vector<char*>::const_iterator it = std::begin(tcp_pid); it!=std::end(tcp_pid); ++it )
    {
        std::cout << "tcp_pid: " << *it << std::endl;
        char path_to_pid[CHAR_ARRAY_MAX_SIZE] = "/proc/";
        strcat(path_to_pid, *it);
        strcat(path_to_pid, "/cmdline");

        char tmp[CHAR_ARRAY_MAX_SIZE];
        cmd_file = fopen(path_to_pid, "r");
        if ( fgets(tmp, sizeof tmp, cmd_file) == nullptr )
        {
            perror("cmdline file empty");
            exit(EXIT_FAILURE);
        }
        char* cmd_line = new char[strlen(tmp)];
        memcpy(cmd_line, tmp, strlen(tmp) + 1);
        proc_cmdline.emplace_back(cmd_line);
        fclose(cmd_file);
    }
    return proc_cmdline;
}

int main(int argc, char* argv[])
{
    ////// crawl /proc/net/tcp
    std::vector<char*> inodes = parse_TCPUDPFIELD(CONNECTION::TCP, TCP_UDP_FIELD::INODE);
    std::cout << "inodes: " << std::endl;
    for( std::vector<char*>::iterator it = std::begin(inodes); it != std::end(inodes); ++it )
        std::cout << *it << std::endl;

    std::vector<char*> local_address = parse_TCPUDPFIELD(CONNECTION::TCP, TCP_UDP_FIELD::LOCAL_ADDRESS);
    std::vector<std::pair<char*, char*>> local_ip_port = split_colon(local_address);
    for ( std::vector<char*>::iterator it=std::begin(local_address); it!=std::end(local_address); ++it)
        std::cout << "this is local_address: " << *it << std::endl;
    for ( std::vector<std::pair<char*, char*>>::iterator it=std::begin(local_ip_port); it!=std::end(local_ip_port); ++it)
        std::cout << "this is local_address: " << (*it).first << " : " << (*it).second << std::endl;

    std::vector<char*> remote_address = parse_TCPUDPFIELD(CONNECTION::TCP, TCP_UDP_FIELD::REMOTE_ADDRESS);
    std::vector<std::pair<char*, char*>> remote_ip_port = split_colon(remote_address);
    for ( std::vector<char*>::iterator it=std::begin(remote_address); it!=std::end(remote_address); ++it)
        std::cout << "this is remote_address: " << *it << std::endl;
    for ( std::vector<std::pair<char*, char*>>::iterator it=std::begin(remote_ip_port); it!=std::end(remote_ip_port); ++it)
        std::cout << "this is local_address: " << (*it).first << " : " << (*it).second << std::endl;
    


    ////// do something with pid socket
    std::vector<char*> proc_pid = search_pid();

    ////// search tcp_pid among socket_pid
    std::vector<char*> tcp_pid = search_tcppid(proc_pid, inodes);
    
    ////// crawl /proc/[pid]/cmdline
    std::vector<char*> proc_cmdline = search_cmdline(tcp_pid);
    for ( std::vector<char*>::iterator it=std::begin(proc_cmdline); it!=std::end(proc_cmdline); ++it)
        std::cout << "this is command line: " << *it << std::endl;

    return 0;
}

// 接下來
// cat /proc/net/tcp以及cat/proc/net/udp看inode欄位有哪些inode
// 與我目前撈到的socket:[xxxxxxxx]比對，若xxxxxxxx == inode則撈/proc/[pid]/comm(得到program name)以及/proc/[pid]/cmdline(執行時下的指令)
// cat /proc/net/tcp的local_address與rem_address轉成十進位 xxx.xxx.xxx.xxx:xxxx