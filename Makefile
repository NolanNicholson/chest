CFLAGS = -O3

all : test chest

debug : CFLAGS = -g
debug : all

chest : main.c *.h
	$(CC) $(CFLAGS) -o chest main.c

test : test.c *.h
	$(CC) $(CFLAGS) -o test test.c

clean :
	rm chest
	rm test
