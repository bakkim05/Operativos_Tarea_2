all: compile run

compile:
	gcc -o interactive interactive.c && clear

run:
	./test

hello:
	gcc -o hello hello.c && clear
