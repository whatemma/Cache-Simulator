cachesim: cachesim.c
	gcc -lm -g -Wall -Werror -fsanitize=address -o cachesim cachesim.c
clean:
	rm -f cachesim
