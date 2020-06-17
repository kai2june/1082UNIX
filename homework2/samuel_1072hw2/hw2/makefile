CXX = g++
CFLAGS = -shared -fPIC -std=c++17

.PHONY: all clean

all: fsmon.so

fsmon.so: 0756706_hw2.cpp
	$(CXX) $(CFLAGS) -o $@ $< -ldl

clean:
	rm -rf fsmon.so
