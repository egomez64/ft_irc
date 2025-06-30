NAME	= censorbot
CC		= c++
CFLAGS	= -Wall -Werror -Wextra
CFLAGS	+= --std=c++98
INCLUDE	= includes

SRC_DIR	= srcs
DIR_DUP	= mkdir -p $(@D)
OBJ_DIR	= .obj

SRC	= $(SRC_DIR)/main.cpp \
	$(SRC_DIR)/Bot.cpp

OBJ		:= $(SRC:%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -I$(INCLUDE) $(OBJ) -o $(NAME)

$(OBJ_DIR)/%.o: %.cpp
	@$(DIR_DUP)
	@$(CC) $(CFLAGS) -I$(INCLUDE) -o $@ -c $< && printf "Compiling: $(notdir $<)\n"

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
