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
#include <sys/socket.h>
#include <getopt.h>

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

void port_hex_to_dec(const char* input, char* output, size_t outlen=CHAR_ARRAY_MAX_SIZE)
{
    long int li;
    li = strtol(input, nullptr, 16);
    if ( li == 0 )
        snprintf(output, outlen, "%c", '*');
    else
        snprintf(output, outlen, "%ld", li);
    // printf("%s\n", output);
    return;
}   


int ip_hex_to_dquad(
    const char* input
  , char* output
  , size_t outlen=CHAR_ARRAY_MAX_SIZE
  , const CONNECTION& connection_type=CONNECTION::TCP
)
{
    if ( connection_type == CONNECTION::TCP || connection_type == CONNECTION::UDP )
    {
        unsigned int a, b, c, d;
        if (sscanf(input, "%2x%2x%2x%2x", &a, &b, &c, &d) != 4)
            return -1;
        snprintf(output, outlen, "%u.%u.%u.%u", d, c, b, a);
    } else if ( connection_type == CONNECTION::TCP6 || connection_type == CONNECTION::UDP6 ) {
        // unsigned int a, b, c, d, e, f, g, h, i, j ,k, l, m, n, o, p;
        // if (sscanf(input, "%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x"
        //     , &a, &b, &c, &d, &e, &f, &g, &h, &i, &j, &k, &l, &m, &n, &o, &p) != 16)
        //     return -1;
        // snprintf(output, outlen, "%u.%u.%u.%u.%u.%u.%u.%u.%u.%u.%u.%u.%u.%u.%u.%u"
        //     , p, o, n, m, l, k, j, i, h, g, f, e, d, c, b, a);
        char a[5], b[5], c[5], d[5], e[5], f[5], g[5], h[5];
        if ( sscanf(input, "%4s%4s%4s%4s%4s%4s%4s%4s", a, b, c, d, e, f, g, h) != 8 )
            return -1;
        snprintf(output, outlen, "%s:%s:%s:%s:%s:%s:%s:%s", h, g, f, e, d, c, b, a);
    }
    return 0;
}


std::vector<std::pair<char*, char*>> split_colon(
    const std::vector<char*>& address
  , const CONNECTION& connection_type
)
{
    std::vector<std::pair<char*, char*>> ip_port;
    std::pair<char*, char*> ip_port_oneline;
    for( std::vector<char*>::const_iterator it = std::begin(address); it != std::end(address); ++it )
    {
        char* pch;
        pch = strchr(*it, ':');
        // std::cout << "found at "<< pch-(*it) << std::endl;
        

        struct in_addr s;
        // char* ip = new char[pch-(*it)+1];
        char* ip = new char[CHAR_ARRAY_MAX_SIZE];
        strncpy(ip, *it, pch-(*it));
        char* ip_decimal = new char[CHAR_ARRAY_MAX_SIZE];
        ip_hex_to_dquad(ip, ip_decimal, CHAR_ARRAY_MAX_SIZE, connection_type);

        // char* port = new char[ sizeof(*it) - (pch-(*it)) + 1];
        char* port = new char[CHAR_ARRAY_MAX_SIZE];
        strncpy(port, (*it) + (pch-(*it)) + 1, strlen(*it) - (pch-(*it)) - 1);
        char* port_decimal = new char[CHAR_ARRAY_MAX_SIZE];
        port_hex_to_dec(port, port_decimal, CHAR_ARRAY_MAX_SIZE);
        // std::cout << "port: " << port << std::endl;
        ip_port_oneline.first = ip_decimal;
        ip_port_oneline.second = port_decimal;
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
        // puts("end of file reached");
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
    std::vector<char*> tcpudp_pid;
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
                    tcpudp_pid.emplace_back(*it);
                }
            } // while readdir
        } // if opendir
        // std::cout << std::endl;
    } // for proc_pid
    return tcpudp_pid;
}


std::vector<char*> search_cmdline(const std::vector<char*>& tcpudp_pid)
{
    std::vector<char*> proc_cmdline;
    std::FILE* cmd_file;
    for ( std::vector<char*>::const_iterator it = std::begin(tcpudp_pid); it!=std::end(tcpudp_pid); ++it )
    {
        // std::cout << "tcpudp_pid: " << *it << std::endl;
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


void print_table(
    const CONNECTION& connection_type
  , const std::vector<std::pair<char*, char*>>& local_ip_port
  , const std::vector<std::pair<char*, char*>>& remote_ip_port
  , const std::vector<char*>& tcpudp_pid
  , const std::vector<char*>& proc_cmdline
  , const char* filter_string=""
)
{
    // std::cout << local_ip_port.size() << "  " << remote_ip_port.size() << "  " << tcpudp_pid.size()
    //     << "  " << proc_cmdline.size() <<std::endl;
    // std::cout << "\n" 
    //     << local_ip_port.size() 
    //     << "\n"
    //     << remote_ip_port.size() 
    //     << "\n" 
    //     << tcpudp_pid.size()
    //     << "\n"
    //     << proc_cmdline.size() 
    //     << std::endl;
    for( std::size_t i = 0; i < local_ip_port.size(); ++i)
    {
        if ( strstr(proc_cmdline.at(i), filter_string) == nullptr )
        {
            continue;
        }
        if ( connection_type == CONNECTION::TCP )
            printf("%-6s", "tcp");
        else if ( connection_type == CONNECTION::UDP )
            printf("%-6s", "udp");
        else if ( connection_type == CONNECTION::TCP6 )
            printf("%-6s", "tcp6");
        else if ( connection_type == CONNECTION::UDP6 )
            printf("%-6s", "udp6");
        printf("%s:%s\t\t\t%s:%s\t\t\t%s/%s\n"
          , local_ip_port.at(i).first
          , local_ip_port.at(i).second
          , remote_ip_port.at(i).first
          , remote_ip_port.at(i).second
          , tcpudp_pid.at(i)
          , proc_cmdline.at(i)
        );
    }
}


void netstat(const CONNECTION& connection_type=CONNECTION::TCP, const char* filter_string="")
{
    ////// crawl /proc/net/tcp
    std::vector<char*> inodes = parse_TCPUDPFIELD(connection_type, TCP_UDP_FIELD::INODE);
    // std::cout << "inodes: " << std::endl;
    // for( std::vector<char*>::iterator it = std::begin(inodes); it != std::end(inodes); ++it )
    //     std::cout << *it << std::endl;

    std::vector<char*> local_address = parse_TCPUDPFIELD(connection_type, TCP_UDP_FIELD::LOCAL_ADDRESS);
    std::vector<std::pair<char*, char*>> local_ip_port = split_colon(local_address, connection_type);
    // for ( std::vector<char*>::iterator it=std::begin(local_address); it!=std::end(local_address); ++it)
    //     std::cout << "this is local_address: " << *it << std::endl;
    // for ( std::vector<std::pair<char*, char*>>::iterator it=std::begin(local_ip_port); it!=std::end(local_ip_port); ++it)
    //     std::cout << "this is (local_ip : local_port) " << (*it).first << " : " << (*it).second << std::endl;

    std::vector<char*> remote_address = parse_TCPUDPFIELD(connection_type, TCP_UDP_FIELD::REMOTE_ADDRESS);
    std::vector<std::pair<char*, char*>> remote_ip_port = split_colon(remote_address, connection_type);
    // for ( std::vector<char*>::iterator it=std::begin(remote_address); it!=std::end(remote_address); ++it)
    //     std::cout << "this is remote_address: " << *it << std::endl;
    // for ( std::vector<std::pair<char*, char*>>::iterator it=std::begin(remote_ip_port); it!=std::end(remote_ip_port); ++it)
    //     std::cout << "this is (remote_ip : remote_port) " << (*it).first << " : " << (*it).second << std::endl;

    ////// do something with pid socket
    std::vector<char*> proc_pid = search_pid();

    ////// search tcpudp_pid among socket_pid
    std::vector<char*> tcpudp_pid = search_tcppid(proc_pid, inodes);
    
    ////// crawl /proc/[pid]/cmdline
    std::vector<char*> proc_cmdline = search_cmdline(tcpudp_pid);
    // for ( std::vector<char*>::iterator it=std::begin(proc_cmdline); it!=std::end(proc_cmdline); ++it)
    //     std::cout << "this is command line: " << *it << std::endl;
    
    print_table(connection_type, local_ip_port, remote_ip_port, tcpudp_pid, proc_cmdline, filter_string);
}


int main(int argc, char* argv[])
{
    bool tcp_show = true, udp_show = true;
    int opt;
    int option_index = 0;
    const char *optstring = "tu";
    static struct option long_options[] = {
        {"tcp", no_argument, nullptr, 't'}
      , {"udp", no_argument, nullptr, 'u'}
      , {0, 0, 0, 0}
    };

    char* filter_string = new char[CHAR_ARRAY_MAX_SIZE];
    while ( ( opt = getopt_long(argc, argv, optstring, long_options, &option_index) ) != -1 )
    {
        if ( strcmp(argv[optind - 1], "-t") == 0 || strcmp(argv[optind - 1], "--tcp") == 0 )
            udp_show = false;
        if ( strcmp(argv[optind - 1], "-u") == 0 || strcmp(argv[optind - 1], "--udp") == 0 )
            tcp_show = false;
        // printf("opt = %c\n", opt);
        // printf("optarg = %s\n", optarg);
        // printf("optind = %d\n", optind);
        // printf("argv[optind - 1] = %s\n",  argv[optind - 1]);
        // printf("option_index = %d\n", option_index);
    }
    if ( tcp_show == false && udp_show == false )
    {
        tcp_show = true;
        udp_show = true;
    }
    for ( int index = optind; index < argc; ++index)
    {
        // printf("\nindex = %d\n", index);
        // printf("argv[index - 1] = %s\n",  argv[index]);
        strcat(filter_string, argv[index]);
        if ( index != argc -1 )
            strcat(filter_string, " ");
    }
    // printf("filter string: %s\n\n", filter_string);


    if ( tcp_show == true )
    {
        printf("%s\n%s\t\t\t%s\t\t\t%s\n"
          , "List of TCP connections:"
          , "Proto Local Address"
          , "Foreign Address"
          , "PID/Program name and arguments"
        );
        netstat(CONNECTION::TCP, filter_string);
        netstat(CONNECTION::TCP6, filter_string);
    }
    if ( udp_show == true)
    {
        printf("\n%s\n%s\t\t\t%s\t\t\t%s\n"
          , "List of UDP connections:"
          , "Proto Local Address"
          , "Foreign Address"
          , "PID/Program name and arguments"
        );
        netstat(CONNECTION::UDP, filter_string);
        netstat(CONNECTION::UDP6, filter_string);
    }

    return 0;
}
