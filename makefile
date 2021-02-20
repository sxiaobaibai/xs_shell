#make: main.o
#	gcc -o make main.o
#main.o: main.c
#	gcc -c -ggdb main.c
#
#clean:
#	rm *.o make


TARGET = xs_shell

SRC = main.c
OBJ = $(SRC:%.c=%.o)
INC = ./

CFLAGS = -I $(INC)
CFLAGS += -Wall -Werror -Wextra

CC = gcc
CXX = g++

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

$(OBJ): $(SRC)
	$(CC) -c $(SRC)

clean:
	$(RM) $(TARGET)

fclean: clean
	$(RM) $(OBJ)

.PHONY: all clean fclean
