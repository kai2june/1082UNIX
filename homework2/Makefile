CC	= gcc
CXX	= g++
CFLAG	= -Wall -g -std=c++17
EXE	= -o $@

all: sandbox.so sandbox choice

sandbox.so: inject.cpp
	$(CXX) $(EXE) -shared -fPIC -std=c++17 $< -ldl

sandbox: command_launcher.cpp
	$(CXX) $(EXE) $(CFLAG) $< -ldl

choice: choice.cpp
	$(CXX) $(EXE) $(CFLAG) $< -ldl

clean:
	rm sandbox.so
	rm sandbox
	rm choice
