CC	= gcc
CXX	= g++
CFLAG	= -Wall -g -std=c++17
EXE	= -o $@
#BOOST	= -I /usr/include -lboost_system-mt -lboost_filesystem-mt

all: sandbox.so sandbox

sandbox.so: inject.cpp
	$(CXX) $(EXE) -shared -fPIC $< -ldl

sandbox: command_launcher.cpp
	$(CXX) $(EXE) $(CFLAG) $< -ldl

preload:
	LD_PRELOAD=./sandbox.so ./sandbox

clean:
	chmod 777 helloworld.cpp
	rm sandbox.so
	rm sandbox
	rm core.*
	rmdir dirrr
	rm trash.txt
	rm link.txt
