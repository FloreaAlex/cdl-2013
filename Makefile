HashMap   : invertedindex.o
		gcc -lm -o invertedindex invertedindex.o

invertedIndex.o      : invertedindex.c invertedindex.h
		gcc -c invertedindex.c

run         : 
		./invertedindex

clean       : 
		rm invertedindex.o
		rm invertedindex
