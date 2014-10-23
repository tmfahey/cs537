# Defines
# The FILE and TARGET denote the test C file
# that was created to test our mem alloc code
CC = gcc
FILE = project3A.c
TARGET = project3A

mem: mem.c mem.h
	#gcc -c -Wall -m32 -fpic mem.c -O
	#gcc -shared -Wall -m32 -o libmem.so mem.o -O
	$(CC) -c -fpic mem.c -Wall -Werror
	$(CC) -shared -o libmem1.so mem.o
	$(CC) -shared -o libmem2.so mem.o
	$(CC) -shared -o libmem3.so mem.o

project3A: Project3A.c
	$(CC) -lmem1 -L. -o $(TARGET) $(FILE) -Wall -Werror

clean:
	rm -rf mem.o libmem1.so libmem2.so libmem3.so
