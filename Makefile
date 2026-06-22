# Makefile - dash++ (Dauphine Advanced Shell)

CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -std=c17 -Iincludes
LDFLAGS = -pthread

SRC_DIR = src
OBJ_DIR = build

SRCS = \
	$(SRC_DIR)/main.c \
	$(SRC_DIR)/error.c \
	$(SRC_DIR)/parser.c \
	$(SRC_DIR)/path.c \
	$(SRC_DIR)/stats.c \
	$(SRC_DIR)/builtins.c \
	$(SRC_DIR)/history.c \
	$(SRC_DIR)/monitor.c \
	$(SRC_DIR)/log.c \
	$(SRC_DIR)/pool.c \
	$(SRC_DIR)/executor.c

OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

NAME = dash

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME) $(LDFLAGS)

