	CFLAGS=-g -O2

all: src/rShell.c jobs-handler.o string-parser.o 
	-@mkdir build
	gcc -g src/rShell.c src/*.o -o build/rShell.out

jobs-handler.o: src/jobs-handler.c
	gcc -c src/jobs-handler.c -o src/jobs-handler.o

string-parser.o: src/string-parser.c
	gcc -c src/string-parser.c -o src/string-parser.o

clean:
	-@rm -f src/*.o
	-@rm -rf build