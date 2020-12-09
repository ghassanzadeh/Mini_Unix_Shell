# https://stackoverflow.com/questions/5871016/compiling-with-flags-in-a-makefile
# https://www.qnx.com/developers/docs/6.4.1/neutrino/utilities/m/make.html
# https://www.cs.swarthmore.edu/~newhall/unixhelp/howto_makefiles.html

CC = gcc
TARGET = shell379
OBJS = main.o init_shell.o redirect_command.o builtin_command.o
WALLFLAGS = -w
CFLAGS = -w -g -Wall


all: $(TARGET)

$(TARGET): $(OBJS)
		$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c builtins.h
	$(CC) $(WALLFLAGS) -c main.c

init_shell.o: init_shell.c builtins.h
			$(CC) $(WALLFLAGS) -c init_shell.c

redirect_command.o: redirect_command.c builtins.h
					$(CC) $(WALLFLAGS) -c redirect_command.c

builtin_command.o: builtin_command.c builtins.h
					$(CC) $(WALLFLAGS) -c builtin_command.c

clean:
	rm -f $(OBJS) $(TARGET)