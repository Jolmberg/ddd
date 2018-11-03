CC = gcc
CFLAGS = -O2 $(shell pkg-config --cflags sdl2 SDL2_image)
OBJECTS = ddd.o 8088.o motherboard.o text.o
LDFLAGS = $(shell pkg-config --libs sdl2 SDL2_image)


ddd : $(OBJECTS)
	$(CC) $(LDFLAGS) $(SDLFLAGS) $(OBJECTS) -o ddd

%.o : %.c
	$(CC) $(CFLAGS) -c $<

8088.o : 8088.h

motherboard.o : motherboard.h

clean:
	rm -rf *.o ddd *~
