build:
	gcc -Wall tema.c -o tema1 -lm

run: build
	./checker.sh

clean:
	rm -rf tema1 tema1.out