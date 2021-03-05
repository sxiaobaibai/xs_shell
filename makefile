TARGET = xs_shell

#SRC = ./src/test.c
SRC = ./src/main.c
SRC += ./src/listen_input.c
OBJ = $(SRC:%.c=%.o)
INC = ./inc

CFLAGS = -I $(INC)
CFLAGS += -Wall -Werror -Wextra

CC = gcc
CXX = g++

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $^ -o $@

clean:
	$(RM) $(TARGET)

fclean: clean
	$(RM) $(OBJ)

.PHONY: all clean fclean
