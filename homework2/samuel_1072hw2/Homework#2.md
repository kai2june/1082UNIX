### Homework #2

#### Advanced Programming in the UNIX Environment

#### Due: April 17, 2019

### Monitor File and Directory Activities of Dynamically Linked Programs

In this homework, we are going to practice library injection and API hijacking. Please implement a "library call monitor" (LCM) program that is able to show the activities of an arbitrary binary running on a Linux operating system. You have to implement your LCM as a shared library and inject the shared library into a process by using LD_PRELOAD. You have to dump the library calls as well as the passed parameters and the returned values. Please monitor file and directory relevant functions listed in the section "Minimum Requirements" below. The result should be output to either stderr or a filename, e.g., "**fsmon.log**". By default, the output is written to stderr. But you may write the output to a filename specified by an environment variable "**MONITOR_OUTPUT**".

You have to compile your source codes and generate a shared object. You don't have to implement any monitored program by yourself. Instead, you should work with those binaries already installed in the system, or the test cases provided by the instructor.

### Minimum Requirements

The minimum list of monitored library calls is shown below. It covers almost all the functions we have introduced in the class.

> closedir	opendir	readdir	creat	open	read	write	dup
> dup2	     close	     lstat	     stat	  pwrite	fopen	fclose	fread
> fwrite	    fgetc	     fgets	fscanf	fprintf	chdir	chown	chmod
> remove	rename	link	unlink	readlink	symlink	mkdir	rmdir

If you would like to monitor more, please read the function lists from the following manual pages: (ordered alphabetically)

1. [**dirent.h(P)**](http://man7.org/linux/man-pages/man0/dirent.h.0p.html)
2. [**fcntl.h(P)**](http://man7.org/linux/man-pages/man0/fcntl.h.0p.html)
3. [**stdio.h(P)**](http://man7.org/linux/man-pages/man0/stdio.h.0p.html)
4. [**stdlib.h(P)**](http://man7.org/linux/man-pages/man0/stdlib.h.0p.html)
5. [**sys_socket.h(7POSIX)**](http://man7.org/linux/man-pages/man0/sys_socket.h.0p.html) (network functions)
6. [**unistd.h(P)**](http://man7.org/linux/man-pages/man0/unistd.h.0p.html)
7. [**sys_stat.h(7POSIX)**](http://man7.org/linux/man-pages/man0/sys_stat.h.0p.html)

### Display Function Call Parameters and Return Values

You will get a basic score if you only print out the raw value of monitored function calls. For example, the primitive data types **char**, **int**, **short**, **long**, **long long**, **float**, and **double**. For pointers, you can also print out its raw values. If you would like to get higher scores, you should output comprehensible outputs for the user. Here are additional explanations for the comprehensible output format.

1. For **char \*** data type, you may optionally print it out as a string.
2. For file descriptors (passed as an **int**), **FILE\***, and **DIR\*** pointers, you can convert them to corresponding file names.
3. For **struct stat** or its pointer, retrieve meaningful information from the structure. For example, file type, file size, and permissions.



### Grading Policy

The tentative grading policy for this homework is listed below:

- [10%] A monitored executable can work as usual. Your program cannot break the functions of a monitored executable.
- [20%] Monitor functions listed in minimum requirements.
- [20%] Provide basic list for function call parameters and return values.
- [20%] Provide comprehensive list for function call parameters and return values.
- [20%] Output can be configured using **MONITOR_OUTPUT** environmental variable.
- [10%] Use Makefile to manage the building process of your program. We will not grade your program if we cannot use **make** command to build your program.



### Homework Submission

Please pack your files into a single ZIP archive and submit your homework via the E3 system. Please also provide a Makefile (used for compiling and linking your codes) and a README file (indicating what functions will be monitored).

### Sample Test Cases

You can download our sample test cases from here: [testcases.zip](https://people.cs.nctu.edu.tw/~chuang/courses/unixprog/resources/hw2_fsmon/testcases.zip). Please see the Makefile for the details.

### Hints

Some hints that may simplify your implementation:

1. You may need to define macros to simplify your implementation.
2. You may consider working with # and ## operators in macros.
3. For variable-length function parameters, consider working with [stdarg.h](http://man7.org/linux/man-pages/man0/stdarg.h.0p.html).
4. You may consider working with **__attribute__((constructor))**. If you don't know what is that, please google for it!
5. The implementation for some library functions may be different to its well-known prototypes. For example, the actual implementation for **stat** in GNU C library is **__xstat**. Therefore, you may not be able to find symbol **stat** in the library. In case that you are not sure about the *real* symbols used in C library, try to work with **readelf** or **nm** to get the symbol names.



### Running Examples

Suppose you have compiled your homework files into **fsmon.so**, three examples from our simple implementation are given below. In the first example, the output is stored into **fsmon.log** file.

```bash
$ MONITOR_OUTPUT=fsmon.log LD_PRELOAD=./fsmon.so head -n 1000 /etc/services > /dev/null
$ cat fsmon.log
# open("/etc/services", 0x0) = 4
# read("/etc/services", 0x7fff85d879b0, 8192) = 8192
# read("/etc/services", 0x7fff85d879b0, 8192) = 8192
# read("/etc/services", 0x7fff85d879b0, 8192) = 2799
# read("/etc/services", 0x7fff85d879b0, 8192) = 0
# close("/etc/services") = 0
# fflush("<STDOUT>") = 0 (0x0)
# fclose("<STDOUT>") = 0
# fflush("<STDERR>") = 0 (0x0)
# fclose("<STDERR>") = 0
$
```

In the second example, the output is written to **stderr**. The blue lines are those printed out by the monitor.

```bash
$ LD_PRELOAD=./fsmon.so ls -la
# fopen("/proc/filesystems", "re") = 0x55a5b0caa270
# fclose("/proc/filesystems") = 0
# opendir(".") = 0x55a5b0cb3ac0
# readdir(".") = tests
# lstat("tests", 0x55a5b0caec78 {mode=00775, size=4096}) = 0
# fopen("/etc/passwd", "rme") = 0x55a5b0caa270
# fclose("/etc/passwd") = 0
# fopen("/etc/group", "rme") = 0x55a5b0caa270
# fclose("/etc/group") = 0
# readdir(".") = ..
# lstat("..", 0x55a5b0caed40 {mode=00775, size=4096}) = 0
# readdir(".") = .
# lstat(".", 0x55a5b0caee08 {mode=00775, size=4096}) = 0
# readdir(".") = fsmon.so
# lstat("fsmon.so", 0x55a5b0caeed0 {mode=00775, size=33352}) = 0
# readdir(".") = Makefile
# lstat("Makefile", 0x55a5b0caef98 {mode=00664, size=390}) = 0
# readdir(".") = fsmon.c
# lstat("fsmon.c", 0x55a5b0caf060 {mode=00664, size=21370}) = 0
# readdir(".") = NULL
# closedir(".") = 0
# fputs_unlocked(0x55a5af0e0cf8, "<STDOUT>") = 1
# fputs_unlocked(0x7ffd733bf505, "<STDOUT>") = 1
total 76
# fputs_unlocked(0x7ffd733be2e0, "<STDOUT>") = 1
# fputs_unlocked(0x55a5b0cc1ce0, "<STDOUT>") = 1
# fputs_unlocked(0x55a5b0cc2300, "<STDOUT>") = 1
# fputs_unlocked(0x7ffd733be2e0, "<STDOUT>") = 1
# fwrite_unlocked(0x55a5af0e113c, 2, 1, "<STDOUT>") = 1
# fwrite_unlocked(0x55a5b0cac270, 1, 1, "<STDOUT>") = 1
# fwrite_unlocked(0x55a5af0e0d5e, 1, 1, "<STDOUT>") = 1
# fwrite_unlocked(0x55a5af0e113c, 2, 1, "<STDOUT>") = 1
# fwrite_unlocked(0x55a5b0cac271, 5, 1, "<STDOUT>") = 1
# fwrite_unlocked(0x55a5af0e0d5e, 1, 1, "<STDOUT>") = 1
# fwrite_unlocked(0x55a5b0cc2580, 1, 1, "<STDOUT>") = 1
# fwrite_unlocked(0x55a5af0e113c, 2, 1, "<STDOUT>") = 1
# fwrite_unlocked(0x55a5b0cac270, 1, 1, "<STDOUT>") = 1
# fwrite_unlocked(0x55a5af0e0d5e, 1, 1, "<STDOUT>") = 1
drwxrwxr-x  3 chuang chuang  4096 Apr  2 00:54 .
# fputs_unlocked(0x7ffd733be2e0, "<STDOUT>") = 1
# fputs_unlocked(0x55a5b0cc1ce0, "<STDOUT>") = 1
# fputs_unlocked(0x55a5b0cc2300, "<STDOUT>") = 1
# fputs_unlocked(0x7ffd733be2e0, "<STDOUT>") = 1
# fwrite_unlocked(0x55a5af0e113c, 2, 1, "<STDOUT>") = 1
# fwrite_unlocked(0x55a5b0cac271, 5, 1, "<STDOUT>") = 1
# fwrite_unlocked(0x55a5af0e0d5e, 1, 1, "<STDOUT>") = 1
# fwrite_unlocked(0x55a5b0cc2450, 1, 2, "<STDOUT>") = 2
# fwrite_unlocked(0x55a5af0e113c, 2, 1, "<STDOUT>") = 1
# fwrite_unlocked(0x55a5b0cac270, 1, 1, "<STDOUT>") = 1
# fwrite_unlocked(0x55a5af0e0d5e, 1, 1, "<STDOUT>") = 1
drwxrwxr-x 35 chuang chuang  4096 Mar 26 20:28 ..
# fputs_unlocked(0x7ffd733be2e0, "<STDOUT>") = 1
# fputs_unlocked(0x55a5b0cc1ce0, "<STDOUT>") = 1
# fputs_unlocked(0x55a5b0cc2300, "<STDOUT>") = 1
# fputs_unlocked(0x7ffd733be2e0, "<STDOUT>") = 1
# fwrite_unlocked(0x55a5b0cc2910, 1, 7, "<STDOUT>") = 7
-rw-rw-r--  1 chuang chuang 21370 Apr  2 00:54 fsmon.c
# fputs_unlocked(0x7ffd733be2e0, "<STDOUT>") = 1
# fputs_unlocked(0x55a5b0cc1ce0, "<STDOUT>") = 1
# fputs_unlocked(0x55a5b0cc2300, "<STDOUT>") = 1
# fputs_unlocked(0x7ffd733be2e0, "<STDOUT>") = 1
# fwrite_unlocked(0x55a5af0e113c, 2, 1, "<STDOUT>") = 1
# fwrite_unlocked(0x55a5b0cac2c4, 5, 1, "<STDOUT>") = 1
# fwrite_unlocked(0x55a5af0e0d5e, 1, 1, "<STDOUT>") = 1
# fwrite_unlocked(0x55a5b0cc26b0, 1, 8, "<STDOUT>") = 8
# fwrite_unlocked(0x55a5af0e113c, 2, 1, "<STDOUT>") = 1
# fwrite_unlocked(0x55a5b0cac270, 1, 1, "<STDOUT>") = 1
# fwrite_unlocked(0x55a5af0e0d5e, 1, 1, "<STDOUT>") = 1
-rwxrwxr-x  1 chuang chuang 33352 Apr  2 00:54 fsmon.so
# fputs_unlocked(0x7ffd733be2e0, "<STDOUT>") = 1
# fputs_unlocked(0x55a5b0cc1ce0, "<STDOUT>") = 1
# fputs_unlocked(0x55a5b0cc2300, "<STDOUT>") = 1
# fputs_unlocked(0x7ffd733be2e0, "<STDOUT>") = 1
# fwrite_unlocked(0x55a5b0cc27e0, 1, 8, "<STDOUT>") = 8
-rw-rw-r--  1 chuang chuang   390 Apr  1 23:48 Makefile
# fputs_unlocked(0x7ffd733be2e0, "<STDOUT>") = 1
# fputs_unlocked(0x55a5b0cc1ce0, "<STDOUT>") = 1
# fputs_unlocked(0x55a5b0cc2300, "<STDOUT>") = 1
# fputs_unlocked(0x7ffd733be2e0, "<STDOUT>") = 1
# fwrite_unlocked(0x55a5af0e113c, 2, 1, "<STDOUT>") = 1
# fwrite_unlocked(0x55a5b0cac271, 5, 1, "<STDOUT>") = 1
# fwrite_unlocked(0x55a5af0e0d5e, 1, 1, "<STDOUT>") = 1
# fwrite_unlocked(0x55a5b0cc2320, 1, 5, "<STDOUT>") = 5
# fwrite_unlocked(0x55a5af0e113c, 2, 1, "<STDOUT>") = 1
# fwrite_unlocked(0x55a5b0cac270, 1, 1, "<STDOUT>") = 1
# fwrite_unlocked(0x55a5af0e0d5e, 1, 1, "<STDOUT>") = 1
drwxrwxr-x  2 chuang chuang  4096 Apr  2 00:35 tests
# fflush_unlocked("<STDOUT>") = 0 (0x0)
# fflush("<STDOUT>") = 0 (0x0)
# fclose("<STDOUT>") = 0
# fflush("<STDERR>") = 0 (0x0)
# fclose("<STDERR>") = 0
$
```

In the third example, we simply show the monitored results for *fscanf* and *fprintf* functions. You can find this simple test case from our provided test case files.

```bash
$ LD_PRELOAD=./fsmon.so ./fscanf 
# fwrite(0x55716963a8f4, 1, 7, "<STDOUT>") = 7
Value: 1234
# fscanf("<STDIN>", "%d", ...) = 1
Value [1234]
# fprintf("<STDOUT>", "Value [%d]
", ...) = 13
$
```

### Some Other Hints ...

When testing your homework, you may inspect symbols used by an executable. We have mentioned that you are not able to see any symbol if the symbols were stripped using **strip**command. However, you may consider working with **readelf** command. For example, we can check the symbols that are unknown to the binary:

```bash
$ nm /usr/bin/wget
nm: /usr/bin/wget: no symbols
$ readelf --syms /usr/bin/wget | grep open
    72: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND freopen64@GLIBC_2.2.5 (2)
    73: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND iconv_open@GLIBC_2.2.5 (2)
   103: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND gzdopen
   107: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND fdopen@GLIBC_2.2.5 (2)
   119: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND open64@GLIBC_2.2.5 (2)
   201: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND fopen64@GLIBC_2.2.5 (2)
```

Alternatively, you may consider using **nm -D** to read symbols. Basically we have two different symbol tables. One is the regular symbol table and the other is dynamic symbol table. The one removed by **strip** is the regular symbol table. So you will need to work with **nm -D** or **readelf --syms** to read the dynamic symbol table.