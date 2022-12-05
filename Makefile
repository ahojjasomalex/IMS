CC=g++

CFLAGS=-Wall -Wextra -pedantic -std=c++17 -g

TARGET = main

.PHONY: all
all: main

main: src/main.cpp src/main.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o build/pottery src/main.cpp -lsimlib

run: exp1 exp2 exp3 exp4

exp1:
	@mkdir -p logs/exp1
	@echo 'Running experiment 1'
	@build/pottery -v 3 > logs/exp1/exp1.run 2>logs/exp1/exp1.log

exp2:
	@mkdir -p logs/exp2
	@echo 'Running experiment 2'
	@build/pottery --workshop-capacity 200 > logs/exp2/exp2.run 2>logs/exp2/exp2.log

exp3:
	@mkdir -p logs/exp3
	@echo 'Running experiment 3'
	@build/pottery --workers 2 --circles 2 --clay 10000 --furnace-capacity 150 --workshop-capacity 8000 -v 3 > logs/exp3/exp3_0.run 2>logs/exp3/exp3_0.log
	@build/pottery --workers 5 --circles 5 --clay 10000 --furnace-capacity 150 --workshop-capacity 8000 -v 3 > logs/exp3/exp3_1.run 2>logs/exp3/exp3_1.log
	@build/pottery --workers 6 --circles 6 --clay 10000 --furnace-capacity 150 --workshop-capacity 8000 -v 3 > logs/exp3/exp3_2.run 2>logs/exp3/exp3_2.log
	@build/pottery --workers 7 --circles 7 --clay 10000 --furnace-capacity 150 --workshop-capacity 8000 -v 3 > logs/exp3/exp3_3.run 2>logs/exp3/exp3_3.log
	@build/pottery --workers 8 --circles 8 --clay 10000 --furnace-capacity 150 --workshop-capacity 8000 -v 3 > logs/exp3/exp3_4.run 2>logs/exp3/exp3_4.log
	@build/pottery --workers 9 --circles 9 --clay 10000 --furnace-capacity 150 --workshop-capacity 8000 -v 3 > logs/exp3/exp3_5.run 2>logs/exp3/exp3_5.log
	@build/pottery --workers 8 --circles 7 --clay 10000 --furnace-capacity 150 --workshop-capacity 8000 -v 3 > logs/exp3/exp3_6.run 2>logs/exp3/exp3_6.log

exp4:
	@mkdir -p logs/exp4
	@echo 'Running experiment 4'
	@build/pottery -w 3 -c 2 -t 8 -l 2000 -v 3 > logs/exp4/exp4.run 2>logs/exp4/exp4.log

clean:
	@rm -rf logs
	@rm -rf build

