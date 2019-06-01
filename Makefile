CFLAGS = -g3 $(shell pkg-config --cflags sdl2 SDL2_image) -std=c11 -pedantic -Wall
SRC = ddd.c 8088.c motherboard.c sdl_text.c gui.c debugger.c disasm.c
OBJ = $(SRC:.c=.o)
DEP = $(OBJ:.o=.d)
LDFLAGS = $(shell pkg-config --libs sdl2 SDL2_image) -lm -lpthread

ddd : $(OBJ)
	$(CC) $(SDLFLAGS) $(OBJ) -o ddd $(LDFLAGS)

-include $(DEP)

%.d : %.c
	@$(CPP) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

%.o : %.c
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm -f $(OBJ) ddd

.PHONY: cleandep
cleandep:
	rm -f $(DEP)
