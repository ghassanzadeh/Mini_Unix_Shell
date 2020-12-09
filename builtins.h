#ifndef DECLARATIONS_SHELL
#define DECLARATIONS_SHELL

/////// Constants

#define LINE_LENGTH 100    // Max # of characters in an input line
#define MAX_ARGS 7         // Max number of arguments to a command
#define MAX_LENGTH 20	   // Max # of characters in an argument
#define MAX_PT_ENTRIES 32  // Max entries in the process table
#define DELIMITERS_LIST " \a\n\r\t" // delimiters user for splitting input line into tokens

// functions declarations

void display_prompt();
void usage_calculator();
char *read_input();
char **tokenize_line(char *input_line);
int execute_command(char **args, int stdout, int stdin);
int init_process(char **args, int stdout, int stdin);
int exit_command(char** args);
int kill_command(char** args);
int ps_process_tracker(char* pid);
int sleep_command(char** args);
int jobs_command(char** args);
int wait_command(char** args);
int suspend_command(char** args);
void find_ampersand(char* input_line);
int resume_command(char** args);
int output_redirect(char* input_line);
int input_redirect(char* input_line);
void build_process_table();
void clean_process_table();

///////////////////
#endif