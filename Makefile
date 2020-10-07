all: compile run

compile:
	gcc -o test test.c && clear

run:
	./test