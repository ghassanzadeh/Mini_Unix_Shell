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

extern int num_args;
bool run_in_bckg = false;
bool input_file = false;
bool output_file = false;

/*
	looks for & at the end of the command and set run_in_bckg as true
	arguments: input_line
	return: none
*/

void find_ampersand(char* input_line){
	char *amp = strstr(input_line, "&");
	strtok(input_line, "&");
	if (amp != NULL){
		run_in_bckg = true;
	}
}


/*
	looks for > in the command. If found, opens an output file with the provided filename and sets outFile as true
	arguments: input_line
	return: fd
*/

int output_redirect(char* input_line){
	int fd; // file descriptor
	char *saveptr_out = NULL;
	char *before_output_token = strtok_r(input_line, ">", &saveptr_out);
	char *output_fname_token = strtok_r(NULL, "> \n\r", &saveptr_out);

	if (output_fname_token != NULL){

		fd = open (output_fname_token, O_CREAT| O_WRONLY | O_TRUNC | O_APPEND, S_IRGRP | S_IROTH | S_IRUSR | S_IWUSR | S_IWGRP | S_IWOTH);
		if (fd < 0){
			perror("open file failure for output");
		}
		output_file = true;
		char *after_output_token = strtok_r(NULL, "", &saveptr_out);
		if (after_output_token != NULL){
			strcat(input_line, after_output_token);
		}
	}
	return fd;
}


/*
	looks for < in the command. If found, opens an input file with the provided filename and sets input_file as true
	arguments: input_line
	return: fd
*/

int input_redirect(char* input_line){

	int fd;
	char *saveptr = NULL;
	char *before_input_token = strtok_r(input_line, "<", &saveptr);
	char *input_fname_token = strtok_r(NULL, "< \n\r", &saveptr);

	if (input_fname_token != NULL){

		fd = open (input_fname_token, O_RDONLY , S_IRGRP | S_IROTH | S_IRUSR);
		if (fd < 0){
			perror("open file failure for input");
		}
		input_file = true;
		
		char *after_input_token = strtok_r(NULL, "", &saveptr);

		if (after_input_token != NULL){
			strcat(input_line, after_input_token);
		}
	}

	return fd;
}

