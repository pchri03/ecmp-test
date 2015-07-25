all: ecmp-test

ecmp-test: ecmp-test.c
	gcc -Wall -W -g -o $@ $^

clean:
	rm -f ecmp-test

test: ecmp-test
	./ecmp-test

debug: ecmp-test
	gdb ./ecmp-test

.PHONY: all clean test debug
