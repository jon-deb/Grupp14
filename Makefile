# Makefile for Windows
SRCDIR=./source
INCDIR=./include
CC=gcc
INCLUDE = C:\msys64\mingw64\include\SDL2

CFLAGS = -g -I$(INCLUDE) -c 
LDFLAGS = -lmingw32 -lSDL2main -lSDL2_image -lSDL2_ttf -lSDL2 -mwindows -lm

footballGame: main.o ball.o player.o text.o
	$(CC) -o footballGame main.o ball.o text.o player.o $(LDFLAGS)

main.o: $(SRCDIR)/main.c
	$(CC) $(CFLAGS) $(SRCDIR)/main.c

ball.o: $(SRCDIR)/ball.c $(INCDIR)/ball.h
	$(CC) $(CFLAGS) $(SRCDIR)/ball.c

player.o: $(SRCDIR)/player.c $(INCDIR)/player.h
	$(CC) $(CFLAGS) $(SRCDIR)/player.c

text.o: $(SRCDIR)/text.c $(INCDIR)/text.h
	$(CC) $(CFLAGS) $(SRCDIR)/text.c

clean:
	rm *.exe
	rm *.o