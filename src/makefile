# clang++ $(pkg-config --cflags --libs opencv4) -g "$srcname" -o "$execname"

CC := clang++
CFLAGS := $$(pkg-config --cflags --libs opencv4) -g
SRC ?= main.cc
EXC ?= main.elf

default: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(INC) -o $(EXC)

clean: $(EXC)
	rm $(EXC)
	rm -r $(EXC).dSYM/
