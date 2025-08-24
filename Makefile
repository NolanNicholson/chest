all : test chest

chest : main.c *.h
	gcc -g -o chest main.c

test : test.c *.h
	gcc -g -o test test.c
