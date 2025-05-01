NAME	= ircserv
CC		= c++
CFLAGS	= -Wall -Werror -Wextra
CFLAGS	+= --std=c++98

DIR_DUP	= mkdir -p $(@D)
OBJ_DIR	= .obj
SRC_DIR	= srcs

SRC		= $(SRC_DIR)/main.cpp \
		$(SRC_DIR)/parsing.cpp

OBJ		:= $(SRC:%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

$(OBJ_DIR)/%.o: %.cpp
	@$(DIR_DUP)
	@$(CC) $(CFLAGS) -o $@ -c $< && printf "Compiling: $(notdir $<)\n"

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
