all: compile run-interactive

compile:
	gcc -o main main.c && clear

run-interactive: compile
	./main interactivo /bin/ls -a

run-automatic: compile
	./main automatico /bin/ls -a
clean:
	rm main temp.txt
