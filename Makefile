CC = gcc
CFLAGS = -O2 $(shell pkg-config --cflags sdl2 SDL2_image) -std=c11 -pedantic
OBJECTS = ddd.o 8088.o motherboard.o sdl_text.o gui.o debugger.o disasm.o
LDFLAGS = $(shell pkg-config --libs sdl2 SDL2_image) -lm -lpthread


ddd : $(OBJECTS)
	$(CC) $(SDLFLAGS) $(OBJECTS) -o ddd $(LDFLAGS)

%.o : %.c
	$(CC) $(CFLAGS) -c $<

8088.o : 8088.h

motherboard.o : motherboard.h

sdl_text.o : sdl_text.h

gui.o : gui.h

debugger.o : debugger.h

disasm.o : disasm.h

clean:
	rm -rf *.o ddd *~
