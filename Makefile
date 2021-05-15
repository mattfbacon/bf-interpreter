main: main.c
	gcc -Wall -Wextra -Werror -O2 $< -o $@
