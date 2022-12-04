CC=g++

CFLAGS=-Wall -Wextra -pedantic -std=c++17 -g

TARGET = main

.PHONY: all
all: main

main: main.cpp main.h
	$(CC) $(CFLAGS) -o pottery main.cpp -lsimlib

run:
	@mkdir -p logs
	@echo 'Running base simulation'
	@./pottery -v 1 2>logs/default_run.log
	@echo 'Running simulation with 3 workers and 1/3 more clay'
	@./pottery -w 3 -l 2150 -v 1 2>logs/3_workers.log
	@echo 'Running simulation with 8h work time with 3 workers and 3 circles'
	@./pottery -w 3 -t 8 -v 1 2>logs/short_worktime.log

clean:
	rm -rf pottery
	rm -rf logs