
asciiinvaders: asciiinvaders.o
	gcc -o asciiinvaders asciiinvaders.o  -lncurses

asciiinvaders.o: asciiinvaders.c
	gcc -g -std=gnu99 -c asciiinvaders.c

clean:
	rm asciiinvaders.o asciiinvaders 
