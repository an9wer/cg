.DEFAULT_GOAL := all

all: cg cg_test

cg: src/_cg.o src/cg.o
	gcc -o cg src/_cg.o src/cg.o

cg_test: src/_cg.o test/test.o
	gcc -o cg_test src/_cg.o test/test.o

src/_cg.o src/cg.o: src/cg.h src/_cg.c src/cg.c
	gcc -I src -c src/_cg.c -o src/_cg.o
	gcc -I src -c src/cg.c -o src/cg.o

test/test.o: src/cg.h test/test.c
	gcc -I src -c test/test.c -o test/test.o

.PHONY: clean uninstall

clean:
	-rm src/_cg.o src/cg.o test/test.o

uninstall:
	-rm -f cg cg_test
