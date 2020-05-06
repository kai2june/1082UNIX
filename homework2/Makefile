CC	= gcc
CXX	= g++
CFLAG	= -Wall -g -std=c++17
EXE	= -o $@
all: inject.so sandbox preload

inject.so: inject.cpp
	$(CXX) $(EXE) -shared -fPIC $< -ldl

sandbox: command_launcher.cpp
	$(CXX) $(EXE) $(CFLAG) $< -ldl

preload:
	LD_PRELOAD=./inject.so ./sandbox

clean:
	rm sandbox
	rm inject.so
