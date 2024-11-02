EXECUTABLE = myshell

SRC = main.c shell.c color_print.c 

FLAGS = -Wall -O0 -lc -lreadline 

CC = gcc

all:
	$(CC) $(SRC) $(FLAGS) -o $(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE)
