all: ecmp-test

CC:=gcc

ecmp-test: ecmp-test.c
	$(CC) --std=c11 -o $@ $^

clean:
	rm -f ecmp-test

test: ecmp-test
	./ecmp-test

.PHONY: all clean test
