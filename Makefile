CFLAGS=-g -O2

all: src/rShell.c rShell-defs.o 
	-@mkdir build
	gcc -g src/rShell.c src/*.o -o build/rShell.out

rShell-defs.o: src/rShell-defs.c
	gcc -c src/rShell-defs.c -o src/rShell-defs.o

clean:
	-@rm -f src/*.o
	-@rm -rf build