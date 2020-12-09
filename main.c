#include "builtins.h"
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


extern bool run_in_bckg;
extern bool output_file;
extern bool input_file;


// https://brennan.io/2015/01/16/write-a-shell-in-c/
int main(int argc, char** argv)
{
	char *input_line;
	char **args;
	int status;
	int fdin;
	int fdout;
	do{
		display_prompt();
		input_line = read_input();
		find_ampersand(input_line);
		fdout = output_redirect(input_line); // finds ">" and open an output file
		fdin = input_redirect(input_line);  // finds "<" and opens an input file

		args = tokenize_line(input_line);  // splits command input into tokens
		status = execute_command(args, fdout, fdin);

		free(input_line);
		free(args);

	}while(status);

	return 0;
}