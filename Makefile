CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=gnu11

all: lrg

lrg: src/main.c
	$(CC) $(CFLAGS) src/main.c -o lrg

clean:
	rm -f lrg
