CFLAGS := $(CFLAGS) -Wall -O2 -mtune=native -g
INC    := -Iinclude $(INC)
LFLAGS :=
CC     := gcc
BINARY := mcscoreboard-miner
DEPS   := build/main.o build/debug.o build/config.o

.PHONY: all clean install

all: build $(DEPS) bin/$(BINARY)

build:
	-mkdir -p build bin

build/main.o: src/main.c
	$(CC) $(CFLAGS) $(INC) -c -o build/main.o src/main.c

build/debug.o: src/debug.c include/debug.h
	$(CC) $(CFLAGS) $(INC) -c -o build/debug.o src/debug.c

build/config.o: src/config.c include/config.h
	$(CC) $(CFLAGS) $(INC) -c -o build/config.o src/config.c

bin/$(BINARY): $(DEPS)
	$(CC) $(CFLAGS) $(INC) -o bin/$(BINARY) $(DEPS) $(LFLAGS)

install:
	cp -fv bin/$(BINARY) /usr/local/bin/$(BINARY)

clean:
	rm -rfv build bin

clang:
	$(MAKE) CC=clang
