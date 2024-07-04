CC = clang++
CFLAGS = -Wall -Wextra -Wpedantic -std=c++17 -g
SRC ?= src/main.cc
EXC ?= bin/main.elf

default: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(EXC)

clean: $(EXC)
	rm $(EXC)
	rm -r $(EXC).dSYM/
