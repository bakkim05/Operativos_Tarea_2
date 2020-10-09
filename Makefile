all: compile run

compile:
	gcc -o main main.c && clear

run:
	./main

hello:
	gcc -o hello hello.c && clear
