SRCDIR=./source
INCDIR=./include
CC=gcc-13
INCLUDE=/usr/local/include
SDL_IMAGE_INCLUDE1=/usr/local/Cellar/sdl2_image/2.8.2_1/include/SDL2/   # First path
SDL_IMAGE_INCLUDE2=/usr/local/include/SDL2/                           # Second path

CFLAGS=-g -I$(INCLUDE) -I$(SDL_IMAGE_INCLUDE1) -I$(SDL_IMAGE_INCLUDE2) -c
LIBS=/usr/local/lib
LDFLAGS=-lSDL2main -lSDL2_image -lSDL2

simpleSDLexample1: main.o ball.o
	$(CC) main.o ball.o -o simpleSDLexample2 $(LDFLAGS) -L$(LIBS)

main.o: $(SRCDIR)/main.c
	$(CC) $(CFLAGS) $(SRCDIR)/main.c -o main.o

ball.o: $(SRCDIR)/ball.c $(INCDIR)/ball.h
	$(CC) $(CFLAGS) $(SRCDIR)/ball.c -o ball.o

clean:
	rm -f simpleSDLexample2
	rm -f main.o
	rm -f ball.o
