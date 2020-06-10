all:Project

Project:main.o functions.o
	gcc -o Project main.o functions.o -static BF_64.a

main.o:main.c
	gcc -c main.c

functions.o:functions.c
	gcc -c functions.c
