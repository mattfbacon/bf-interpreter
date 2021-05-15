#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>

#define EXIT_MY_FAIL 1 // anything unrelated to BF interpretation
#define EXIT_YOUR_FAIL 2 // otherwise

#define BF_BUFFER_SIZE 30000

#define bf_current_cell (bf_buffer[bf_buffer_idx])
#define bf_wrap_idx (bf_buffer_idx = bf_buffer_idx % BF_BUFFER_SIZE)

#define current_program_char (program_buffer[program_idx])

static char bf_buffer[BF_BUFFER_SIZE] = { 0, };
static size_t bf_buffer_idx = 0;

static char* program_buffer = NULL;
static size_t program_idx = 0;
static size_t program_size = 0;

enum bf_command {
	PTR_RIGHT = 0x3e,
	PTR_LEFT = 0x3c,
	ADD = 0x2b,
	SUB = 0x2d,
	WHILE_ENTER = 0x5b,
	WHILE_EXIT = 0x5d,
	INPUT = 0x2c,
	OUTPUT = 0x2e,
};

void bf_run() {
	switch(current_program_char) {
		case PTR_RIGHT:
			bf_buffer_idx++;
			bf_wrap_idx;
			break;
		case PTR_LEFT:
			bf_buffer_idx--;
			bf_wrap_idx;
			break;
		case ADD:
			bf_current_cell++;
			break;
		case SUB:
			bf_current_cell--;
			break;
		case WHILE_ENTER:
			if (bf_current_cell == 0) {
				size_t while_depth = 0;
				program_idx++; // always forward at least one
				while (true) {
					if (current_program_char == '[') { while_depth++; }
					else if (current_program_char == ']') {
						if (while_depth == 0) {
							break; // exit as if we were on the ], so the next call will be on the next char
						} else {
							while_depth--;
						}
					}
					program_idx++;
					if (program_idx >= program_size) {
						puts("Unterminated [\n");
						exit(EXIT_YOUR_FAIL);
					}
				}
			}
			break;
		case WHILE_EXIT:
			if (bf_current_cell != 0) {
				size_t while_depth = 0;
				program_idx--; // always back at least one
				while (true) {
					if (current_program_char == ']') { while_depth++; }
					else if (current_program_char == '[') {
						if (while_depth == 0) {
							break; // exit as if we were on the [, so the next call will be on the next char
						} else {
							while_depth--;
						}
					}
					if (program_idx == 0) {
						puts("Unmatched ]\n");
						exit(EXIT_YOUR_FAIL);
					}
					program_idx--;
				}
			}
			break;
		case INPUT:
			bf_current_cell = getchar();
			break;
		case OUTPUT:
			putchar(bf_current_cell);
			break;
	}
}

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("usage: %s <file>\n", argv[0]);
		exit(1);
	}
	FILE* f = fopen(argv[1], "r");
	if (!f) {
		perror("While opening file");
		exit(EXIT_MY_FAIL);
	}
	fseek(f, 0, SEEK_END);
	program_size = ftell(f);
	program_buffer = malloc(program_size + 1);
	program_buffer[program_size] = '\0';
	rewind(f);
	for (size_t i = 0; i < program_size + 1; i++) {
		char c = fgetc(f);
		if (c == EOF) break;
		if (ferror(f)) {
			puts("Error reading from file.\n");
			exit(EXIT_MY_FAIL);
		}
		program_buffer[i] = c;
	};
	if (!feof(f)) {
		puts("fseek with SEEK_END is broken on your system!\n");
		exit(EXIT_MY_FAIL);
	}
	do { bf_run(); program_idx++; } while (program_idx < program_size);
	return 0;
}
