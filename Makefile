CC = gcc
CFLAGS = -O2 $(shell pkg-config --cflags sdl2 SDL2_image) -std=c11 -pedantic
OBJECTS = ddd.o 8088.o motherboard.o sdl_text.o gui.o
LDFLAGS = $(shell pkg-config --libs sdl2 SDL2_image) -lm


ddd : $(OBJECTS)
	$(CC) $(LDFLAGS) $(SDLFLAGS) $(OBJECTS) -o ddd

%.o : %.c
	$(CC) $(CFLAGS) -c $<

8088.o : 8088.h

motherboard.o : motherboard.h

sdl_text.o : sdl_text.h

clean:
	rm -rf *.o ddd *~
