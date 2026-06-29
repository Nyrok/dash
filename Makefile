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

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Sources testées : tout sauf main.c (qui fournit son propre main).
TEST_SRCS = $(filter-out $(SRC_DIR)/main.c,$(SRCS)) $(wildcard tests/*.c)

# Compile et lance les tests unitaires (framework maison MinUnit), un fichier
# de tests par module.
test: $(NAME)
	$(CC) $(CFLAGS) $(TEST_SRCS) -o test_dash $(LDFLAGS)
	./test_dash

# Couverture des tests via gcov (nécessite gcov/lcov).
coverage:
	$(CC) $(CFLAGS) --coverage $(TEST_SRCS) -o test_cov $(LDFLAGS)
	./test_cov
	gcov -n $(TEST_SRCS) | grep -A1 "File '$(SRC_DIR)" || true

clean:
	rm -rf $(OBJ_DIR) test_dash test_cov *.gcda *.gcno *.gcov

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all test coverage clean fclean re
