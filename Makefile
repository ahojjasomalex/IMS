CC=g++

CFLAGS=-Wall -Wextra -pedantic -std=c++17 -g

TARGET = main

.PHONY: all
all: main

main: src/main.cpp src/main.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o build/pottery src/main.cpp -lsimlib

run:
	@mkdir -p logs
#	@echo 'Running base simulation'
#	@build/./pottery -v 3 2>logs/base_run.log
	@echo 'Running simulation with 3 workers, 2 circles and 1/3 more clay'
	@build/./pottery -w 3 -l 5000 -v 3 2>logs/3_workers_2_cirles.log
	@echo 'Running simulation with 3 workers, 3 circles and 1/3 more clay'
	@build/./pottery -w 3 -c 3 -l 5000 -v 3 2>logs/3_workers_3_circles.log
#	@echo 'Running simulation with 8h work time with 3 workers and 2 circles'
#	@build/./pottery -w 3 -c 2 -t 8 -l 2000 -v 3 2>logs/short_worktime.log

exp1:


clean:
	@rm -rf logs
	@rm -rf build

pack:
