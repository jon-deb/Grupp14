# Makefile for Windows
SRCDIR=./src
COMMONSRCDIR=../lib/src
INCDIR=../lib/include
CC=gcc
INCLUDE = -IC:/msys64/mingw64/include/SDL2 -I$(INCDIR) 
CFLAGS = -g $(INCLUDE) -c
LDFLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_net -mwindows -lm

client: main.o ball.o player.o text.o power.o
	$(CC) -o client main.o ball.o text.o player.o power.o $(LDFLAGS)

main.o: $(SRCDIR)/main.c $(INCDIR)/player_data.h $(INCDIR)/player.h $(INCDIR)/ball.h $(INCDIR)/text.h
	$(CC) $(CFLAGS) $(SRCDIR)/main.c

player.o: $(COMMONSRCDIR)/player.c $(INCDIR)/player_data.h $(INCDIR)/player.h $(INCDIR)/ball.h
	$(CC) $(CFLAGS) $(COMMONSRCDIR)/player.c

text.o: $(COMMONSRCDIR)/text.c $(INCDIR)/text.h
	$(CC) $(CFLAGS) $(COMMONSRCDIR)/text.c

ball.o: $(COMMONSRCDIR)/ball.c $(INCDIR)/ball.h $(INCDIR)/player_data.h
	$(CC) $(CFLAGS) $(COMMONSRCDIR)/ball.c

power.o: $(COMMONSRCDIR)/power.c $(INCDIR)/power.h
	$(CC) $(CFLAGS) $(COMMONSRCDIR)/power.c -o power.o

clean:
	rm *.exe
	rm *.o