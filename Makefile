# Makefile for Windows
SRCDIR=./source
INCDIR=./include
CC=gcc
INCLUDE = C:\msys64\mingw64\include\SDL2

CFLAGS = -g -I$(INCLUDE) -c 
LDFLAGS = -lmingw32 -lSDL2main -lSDL2_image -lSDL2 -mwindows -lm

footballGame: main.o ball.o
	$(CC) -o footballGame main.o ball.o $(LDFLAGS)

main.o: $(SRCDIR)/main.c
	$(CC) $(CFLAGS) $(SRCDIR)/main.c

ball.o: $(SRCDIR)/ball.c $(INCDIR)/ball.h
	$(CC) $(CFLAGS) $(SRCDIR)/ball.c

clean:
	rm *.exe
	rm *.o
