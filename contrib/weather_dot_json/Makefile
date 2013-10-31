CFLAGS := $(CFLAGS) -Wall -O2 -mtune=native -g
LFLAGS := -ljansson
CC     := gcc
BINARY := weather_dot_json

.PHONY: all clean

all: weather_dot_json

weather_dot_json: main.c
	$(CC) $(CFLAGS) main.c -o $(BINARY) $(LFLAGS)

install:
	cp -fv bin/$(BINARY) /usr/local/bin/$(BINARY)

clean:
	rm $(BINARY)

clang:
	$(MAKE) CC=clang
