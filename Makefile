all: compile run

compile:
	gcc -o main main.c

run:
	./main interactivo ./hello

hello:
	gcc -o hello hello.c && clear
