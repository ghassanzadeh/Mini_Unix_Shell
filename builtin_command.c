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

struct timeval user_time, sys_time;

extern int num_args;
extern bool run_in_bckg;
extern bool output_file;
extern bool input_file;

char *process_pids[MAX_PT_ENTRIES];
char *commands[MAX_PT_ENTRIES];
char *process_status[MAX_PT_ENTRIES];


/*
	calculates the system and user time for all processes
	args: non
	return: none
*/

void usage_calculator(){
	//https://stackoverflow.com/questions/35100805/using-getrusage-to-get-the-time-for-parent-and-children
	// https://stackoverflow.com/questions/46105827/getrusage-on-child-process
	// https://stackoverflow.com/questions/10509660/getting-getrusage-to-measure-system-time-in-c
	struct rusage children_use_time;

  	getrusage(RUSAGE_CHILDREN, &children_use_time);
  	user_time = children_use_time.ru_utime;
  	sys_time = children_use_time.ru_stime;
  }


 /*
	checks if a process is currently running by piping "ps" command
	args: pid
	return: 0 or 1

 */

 int ps_process_tracker(char* pid){
 // https://brennan.io/2015/01/16/write-a-shell-in-c/
	FILE *pipe;
	int lines = 0;
	char *ps_buffer = NULL;
	char **ps_token;
	ssize_t ps_buffer_size = 1024;


	if (!(pipe = popen("ps","r"))){
		printf("opening pipe failed in ps_tracker");
	}

	while (getline(&ps_buffer, &ps_buffer_size, pipe) != -1){
		lines ++;
		if (lines == 1){
			continue;
		}
		ps_token = tokenize_line(ps_buffer);

		if (strcmp(ps_token[0], pid) == 0){
			return 1;
		}
	}

	pclose(pipe);
	return 0;
 }

 /*

 	runs wait command to wait until process <pid> has completed its execution by first checking if the process is running or not
 	args: args
 	return: 1 or 0, for successful/failed wait
 */

 int wait_command(char** args){
 	// https://stackoverflow.com/questions/1058047/wait-for-a-process-to-finish
 	// https://linuxhostsupport.com/blog/what-is-the-wait-command-in-linux-with-examples/

		if (num_args == 1){
			fprintf(stderr, "pid of the process should be added for wait\n");
		}
		else if (num_args > 2){
			fprintf(stderr, "Too many arguments for wait\n");
		}

		else if (!(ps_process_tracker(args[1]))){
			fprintf(stderr, " did not find the process to wait in the ps tracker\n");
		}

		else{
			waitpid((pid_t) atoi(args[1]), NULL, 0);
		}

	return 1;
 }


/*

	display the information about the running processes in shell379; pid, status, time in second, command
	as well as the number of active processes, total system time and user time
	args: args
	return: 
*/
 int jobs_command(char** args){

 	build_process_table();
	printf("Completed processes:\n");
	printf("User time =          %lld seconds\n",(long long) user_time.tv_sec);
 	printf("Sys time =          %lld seconds\n",(long long) sys_time.tv_sec);
 	clean_process_table();

 	return 1;
 }

 /*
 	resume the execution of a suspended process
 */
 int resume_command(char** args){
 	// https://stackoverflow.com/questions/37715003/how-to-use-sigstop-sigcont-and-sigkill-for-child-process-in-c
 	// https://ostechnix.com/suspend-process-resume-later-linux/

	if (num_args == 1){
		fprintf(stderr, "pid  of the process must be added for resume\n");
	}
	else if (num_args > 2){
		fprintf(stderr, "Too many arguments for resume\n");
	}
	else if (!(ps_process_tracker(args[1]))){
		fprintf(stderr, "pid is not in process table, resume process failed\n");
	}
	else{
		int resume_pid = atoi(args[1]);
		kill(resume_pid, SIGCONT);
	}
	
	return 1;
 }

 /*

 	suspends a running process
 */

 int suspend_command(char** args){
 	// https://stackoverflow.com/questions/42136946/sigstop-not-working-in-c-program
	if (num_args == 1){
		fprintf(stderr, "pid  of the process should be added for suspend\n");
	}
	else if (num_args > 2){
		fprintf(stderr, "Too many arguments for suspend\n");
	}
	else if (!(ps_process_tracker(args[1]))){
		fprintf(stderr, "pid is not an active process, suspend process failed\n");
	}
	else{
		int suspend_pid = atoi(args[1]);
		kill(suspend_pid, SIGSTOP);
	}	
	return 1;

 }

 /*
  sleep for certain number od seconds
 */

 int sleep_command(char** args){

 	if (num_args == 1){
		fprintf(stderr, "time second should be added for sleep\n");
	}
	else if (num_args > 2){
		fprintf(stderr, "Too many arguments for sleep\n");
	}		
	else{
		int sleep_sec = atoi(args[1]);
		if (sleep_sec){
			sleep(sleep_sec);
		}
	}

	return 1;
 }

 /*
 	end the execution of shell379
 */

int exit_command(char** args){

	if (num_args > 1){
		fprintf(stderr, "Too many arguments for exit\n");
		return 1;
	}else{
		wait(0); // waits untill all processes initiated by the shell are complete
		printf("Resources used\n");
		printf("User time =          %lld seconds\n", (long long) user_time.tv_sec);
 		printf("Sys time =          %lld seconds\n", (long long) sys_time.tv_sec);
		return 0;	
	}	
}

/*
	kill a requested process
*/
int kill_command(char** args){
	// https://stackoverflow.com/questions/29248585/c-checking-command-line-argument-is-integer-or-not
	if (num_args == 1){
		fprintf(stderr, "Enter a pid to kill\n");
	}

	if ( num_args > 2){ 
		fprintf(stderr, "Too many arguments for kill\n");
	}

	if (ps_process_tracker(args[1])){
		kill(atoi(args[1]), SIGTERM);
		waitpid(atoi(args[1]), NULL, 0);
		kill(atoi(args[1]), SIGKILL);
	}else{
		fprintf(stderr, "The pid is not for an active process for kill");
	}

	return 1;
}

/*
	builds a process table and prints its information for jobs command
*/

void build_process_table(){

	FILE *pcb_pipe;
 	char* pcb_buffer = NULL;
	ssize_t pcb_buffer_size = 1024;
	int pcb_lines = 0;
	int active_processes = 0;
	int num_processes = 0;
	char ** pcb_token;

	char *pid_token;
	char *stat_token;
	char *time_token;
	char *cmd_token;
	char *saveptr = NULL;

	if (!(pcb_pipe = popen("ps -o pid,stat,time,cmd","r"))){
		printf("pipe failed");
	}

	while (getline(&pcb_buffer, &pcb_buffer_size, pcb_pipe) != -1){
		pcb_lines ++;

		if (pcb_lines == 4){
			printf("#      PID    S    SEC    COMMAND\n");
		}

		if (pcb_lines > 3){
			pid_token = strtok_r(pcb_buffer, " ", &saveptr);
			stat_token = strtok_r(NULL, " ", &saveptr);
			time_token = strtok_r(NULL, " ", &saveptr);
			cmd_token = strtok_r(NULL, "\n", &saveptr);

			// convert to second:

			char *hr_token = strtok(time_token, ":");
			char *min_token = strtok(NULL, ":");
			char *sec_token = strtok(NULL, "");
			int hr = (int) atoi(hr_token);
			int min = (int) atoi(min_token);
			int sec = (int) atoi(sec_token);
			int total_sec = (hr*360) + (min*60) + sec;

			//////////////////

			if ((strcmp(cmd_token,"sh -c ps -o pid,stat,time,cmd") != 0) && (strcmp(cmd_token, "ps -o pid,stat,time,cmd") != 0) && (strcmp(cmd_token, "./shell379") != 0)){ // exclude sh and ps proccesses
				if ((strcmp(stat_token, "R+") == 0) || (strcmp(stat_token, "R") == 0) || (strcmp(stat_token, "S+") == 0) || (strcmp(stat_token, "S") == 0)){
					active_processes ++;
				}
				process_pids[num_processes] = pid_token;
				commands[num_processes] = cmd_token;
				process_status[num_processes] = stat_token;
				printf ("%d      %s   %s   %d   %s\n",pcb_lines-4, process_pids[num_processes], process_status[num_processes], total_sec, commands[num_processes]);
				num_processes ++;

			}

		}
	}
	pclose(pcb_pipe);

	printf("Processes =          %d active\n", active_processes);
	printf("Total Processes =          %d (all status)\n", num_processes);
}

/*
	clean up the process table after entering jobs command
*/

void clean_process_table(){
	int max_table_size = MAX_PT_ENTRIES;
	for (int i = 0; i < max_table_size; i++ ){
		process_pids[i] = NULL;
		commands[i] = NULL;
		process_status[i] = NULL;
	}

}