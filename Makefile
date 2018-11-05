CC=gcc
CFLAGS=-O3 -funroll-loops

oliperftmake: oliperft.c
	$(CC) -o oliperft *.c $(CFLAGS)
