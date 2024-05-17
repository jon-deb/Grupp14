# Client Makefile for mac
SRCDIR=./src
COMMONSRCDIR=../lib/src
INCDIR=../lib/include
CC=gcc-13
INCLUDE=/opt/homebrew/include

# Paths to the SDL2 and SDL2_image includes
SDL2_INCLUDE=/opt/homebrew/include/SDL2
SDL_IMAGE_INCLUDE=/opt/homebrew/Cellar/sdl2_image/2.8.2_1/include/SDL2

# Compiler flags
CFLAGS=-g -I$(INCLUDE) -I$(SDL2_INCLUDE) -I$(SDL_IMAGE_INCLUDE) -I$(INCDIR) -c
LIBS=/opt/homebrew/lib

# Linker flags
LDFLAGS=-L$(LIBS) -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_net -lm

client: main.o ball.o player.o text.o power.o
	$(CC) main.o ball.o player.o text.o power.o -o client $(LDFLAGS)

main.o: $(SRCDIR)/main.c $(INCDIR)/player_data.h $(INCDIR)/player.h $(INCDIR)/ball.h $(INCDIR)/text.h $(INCDIR)/power.h
	$(CC) $(CFLAGS) $(SRCDIR)/main.c -o main.o

player.o: $(COMMONSRCDIR)/player.c $(INCDIR)/player_data.h $(INCDIR)/player.h
	$(CC) $(CFLAGS) $(COMMONSRCDIR)/player.c -o player.o

text.o: $(COMMONSRCDIR)/text.c $(INCDIR)/text.h
	$(CC) $(CFLAGS) $(COMMONSRCDIR)/text.c -o text.o

ball.o: $(COMMONSRCDIR)/ball.c $(INCDIR)/ball.h $(INCDIR)/player_data.h
	$(CC) $(CFLAGS) $(COMMONSRCDIR)/ball.c -o ball.o

power.o: $(COMMONSRCDIR)/power.c $(INCDIR)/power.h
	$(CC) $(CFLAGS) $(COMMONSRCDIR)/power.c -o power.o

clean:
	rm -f *.o client
