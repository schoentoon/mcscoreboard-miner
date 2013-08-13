CFLAGS := $(CFLAGS) -Wall -O2 -mtune=native -std=c99 -g
DEFINES:= -D_GNU_SOURCE -D_XOPEN_SOURCE=500
INC    := -IcNBT -Iinclude $(INC)
LFLAGS := -LcNBT -lnbt -levent -lz
CC     := gcc
BINARY := mcscoreboard-miner
DEPS   := build/main.o build/debug.o build/config.o build/filereader.o build/file_processors.o

.PHONY: all clean install libnbt clang

all: build libnbt $(DEPS) bin/$(BINARY)

build:
	-mkdir -p build bin

libnbt:
	$(MAKE) -C cNBT libnbt.a CC=$(CC)

build/main.o: src/main.c
	$(CC) $(CFLAGS) $(DEFINES) $(INC) -c -o build/main.o src/main.c

build/debug.o: src/debug.c include/debug.h
	$(CC) $(CFLAGS) $(DEFINES) $(INC) -c -o build/debug.o src/debug.c

build/config.o: src/config.c include/config.h
	$(CC) $(CFLAGS) $(DEFINES) $(INC) -c -o build/config.o src/config.c

build/filereader.o: src/filereader.c include/filereader.h
	$(CC) $(CFLAGS) $(DEFINES) $(INC) -c -o build/filereader.o src/filereader.c

build/file_processors.o: src/file_processors.c include/file_processors.h
	$(CC) $(CFLAGS) $(DEFINES) $(INC) -c -o build/file_processors.o src/file_processors.c

bin/$(BINARY): $(DEPS)
	$(CC) $(CFLAGS) $(INC) -o bin/$(BINARY) $(DEPS) $(LFLAGS)

install:
	cp -fv bin/$(BINARY) /usr/local/bin/$(BINARY)

clean:
	rm -rfv build bin
	$(MAKE) -C cNBT clean

clang:
	$(MAKE) CC=clang
