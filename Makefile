CC = g++

CFLAGS = -g -Wall

TARGET = main

all: main

main: main.cpp
	$(CC) $(CFLAGS) -o main main.cpp -lsimlib

clean:
	rm -rf main