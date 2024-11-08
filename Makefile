EXECUTABLE = sshl

SRC = main.c shell.c color_print.c shell_context.c modified_prints.c

FLAGS = -Wall -O0 -lc -lreadline -g

CC = gcc

all:
	$(CC) $(SRC) $(FLAGS) -o $(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE)
