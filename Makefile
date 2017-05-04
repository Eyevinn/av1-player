CC=gcc
CFLAGS=
LIBS=-laom -lSDL2

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

av1play: main.o
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -f *.o core av1play
