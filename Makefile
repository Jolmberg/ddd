CC=gcc
CFLAGS = -O2
OBJECTS = ddd.o 8088.o motherboard.o

ddd : $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o ddd

%.o : %.c %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o wmix *~

