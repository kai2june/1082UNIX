CC	= gcc
CXX	= g++
CFLAG	= -Wall -g -std=c++17
EXE	= -o $@

all: sandbox.so sandbox preload

sandbox.so: inject.cpp
	$(CXX) $(EXE) -shared -fPIC $< -ldl

sandbox: command_launcher.cpp
	$(CXX) $(EXE) $(CFLAG) $< -ldl

preload:
	LD_PRELOAD=./sandbox.so ./sandbox

clean:
	rm sandbox
	rm sandbox.so
	rmdir dirrr
	rm link.txt
	rm trash.txt
