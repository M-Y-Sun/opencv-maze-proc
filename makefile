# clang++ $(pkg-config --cflags --libs opencv4) -g "$srcname" -o "$execname"

CC := clang++
CFLAGS := $$(pkg-config --cflags --libs opencv4) -g

SRC_DIR := src
SRC ?= $(SRC_DIR)/main.cc
INCL_DIR := $(SRC_DIR)/include
EXC ?= bin/main.elf

default: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(INCL_DIR)/*.cc -o $(EXC)

clean: $(EXC)
	rm $(EXC)
	rm -r $(EXC).dSYM/
