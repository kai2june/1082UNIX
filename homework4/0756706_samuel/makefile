CXX = g++
CFLAGS = -g -Wall -std=c++17
LIBRARY = -lelf -lcapstone

.PHONY: all clean

all: sdb

elftool.o: elftool.c elftool.h
	$(CXX) $(CFLAGS) -c $< -o $@

sdb.o: sdb.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

sdb: elftool.o sdb.o
	$(CXX) $(CFLAGS) $^ -o $@ $(LIBRARY)

clean:
	rm -rf elftool.o sdb.o sdb
