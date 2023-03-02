# Compiler
CC = gcc

# Flags
FLAGS = -Wall -Werror

A2Tables: A2.o 
	$(CC) $(FLAGS) -o A2Tables A2.o -lpthread

A2.o: A2.c
	$(CC) $(FLAGS) -c A2.c

.PHONY: clean
clean:
	rm *.o