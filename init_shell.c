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

int num_args = 0;

void display_prompt(){
	run_in_bckg = false;
	output_file = false;
	input_file = false;
	printf("%s","shell379>"); // display prompt
}

/*
	reads the input from stdin and returns the input for further tokenization
*/

char *read_input(){
// https://brennan.io/2015/01/16/write-a-shell-in-c/

	ssize_t buffer_size = 0;
	char *buffer = NULL;

	if(getline(&buffer, &buffer_size, stdin) == -1){
		if(feof(stdin)){
			exit(EXIT_SUCCESS);
		}else{
			perror("failed to readline\n");
			exit(EXIT_FAILURE);
		}
	}

	return buffer;
}

/*
	separate the input line based on the delimiters list into a token array and returns a token array of arguments
*/

char **tokenize_line(char *input_line){
// https://brennan.io/2015/01/16/write-a-shell-in-c/

	char *token;
	int token_buffer_len = LINE_LENGTH;
	int index = 0;

	// https://www.programiz.com/c-programming/c-dynamic-memory-allocation
	char **token_array = malloc(token_buffer_len * sizeof(char*));
	char *delimiters = DELIMITERS_LIST;

	if (!token_array){
		fprintf(stderr, "buffer allocation faild for tokenization\n");
		exit(EXIT_FAILURE);
	}
	token = strtok(input_line, delimiters);

	while (token != NULL){
		token_array[index] = token;
		index ++;

		if (token_buffer_len <= index){
		 	token_buffer_len += LINE_LENGTH;
			token_array = realloc(token_array, token_buffer_len * sizeof(char*));
		}

		token = strtok(NULL, delimiters);
	}

	token_array[index] = NULL;
	num_args = index;

	return token_array;		
}


/*
	forks a new processes and child process and handles input and output file read as well as signal handler for preventing the creation of zombie processes
*/

int init_process(char **args, int fdout, int fdin){

// three easy pieces: Fig 5.3
// https://www.tutorialspoint.com/unix_system_calls/waitpid.htm
// https://www.geeksforgeeks.org/wait-system-call-c/
// reference: man page for wait
// https://stackoverflow.com/questions/35100805/using-getrusage-to-get-the-time-for-parent-and-children
// https://linux.die.net/man/2/wait

	int status;
	pid_t wait_pid;
	pid_t wait_pi;
	pid_t rc;
	int ret_output, ret_input;

	// used to prevent zombie processes
	// https://stackoverflow.com/questions/17015830/how-can-i-prevent-zombie-child-processes
	// https://www.geeksforgeeks.org/zombie-processes-prevention/

	if (run_in_bckg){
		struct sigaction sigchld_action = {
  			.sa_handler = SIG_DFL,
  			.sa_flags = SA_NOCLDWAIT
		};
		sigaction(SIGCHLD, &sigchld_action, NULL);
	}

	rc = fork();

	if (rc < 0){
		perror("fork failed\n");
		// exit(EXIT_FAILURE);
	} else if (rc == 0){ // child process

		if (input_file == true){ // if the command needed to read from a file
			ret_input = dup2(fdin, STDIN_FILENO);
			if (ret_input <0){
				perror("dup2 input");
				exit(1);
			}

		}

		if (output_file == true){ // if the command needed to write to a file
			ret_output = dup2(fdout, STDOUT_FILENO);
			if (ret_output <0){
				perror("dup2 output");
				exit(1);
			}

		}
		if (execvp(args[0], args) == -1){ // execute a command to overwrite the child process
			perror("execvp");
		}
		// fflush(stdout);


		close(fdout);
		close(fdin);
		_exit(EXIT_FAILURE);
		// _exit(0);
		
	} else{ // parent process

		if (run_in_bckg == false){

			do {
				// https://www.tutorialspoint.com/unix_system_calls/waitpid.htm
			
				wait_pid = waitpid(rc, &status, WUNTRACED | WCONTINUED);

        	} while (!WIFEXITED(status) && !WIFSIGNALED(status));


		}else if (run_in_bckg == true){

			wait_pi = waitpid(-1, &status, WNOHANG);        
			if (wait_pi == -1){
				fprintf(stderr,"failed waitpid negative in background");

			}
		}
		usage_calculator();
	}
	return 1;
}


/*
	calls the appropriate function to handle the execution of the command
*/

int execute_command(char **args, int fdout, int fdin){

	int fd_output = fdout;
	int fd_input = fdin;

	if (args[0] == NULL){
		return 1;
	}

	if (args[0] != NULL){

		if (strcmp(args[0], "exit") == 0){
			return exit_command(args);
		}
		else if (strcmp(args[0], "jobs") == 0){
			return jobs_command(args);
		}
		else if (strcmp(args[0], "kill") == 0){
			return kill_command(args);
		}
		else if (strcmp(args[0], "resume") == 0){
			return resume_command(args);
		}
		else if (strcmp(args[0], "sleep") == 0){
			return sleep_command(args);
		}
		else if (strcmp(args[0], "suspend") == 0){
			return suspend_command(args);
		}
		else if (strcmp(args[0], "wait") == 0){
			return wait_command(args);
		}	
	}

	return init_process(args, fd_output, fd_input); // return the status of the command to the main

}