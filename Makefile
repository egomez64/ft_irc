NAME			= ircserv
CC				= c++
CFLAGS			= -Wall -Werror -Wextra
CFLAGS			+= --std=c++98 -g
INCLUDE			= includes

SRC_DIR		= srcs
DIR_DUP		= mkdir -p $(@D)
OBJ_DIR		= .obj

SRC		= $(SRC_DIR)/main.cpp \
		$(SRC_DIR)/Server.cpp \
		$(SRC_DIR)/Client.cpp \
		$(SRC_DIR)/Channel.cpp \
		$(SRC_DIR)/utils.cpp

OBJ		:= $(SRC:%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -I$(INCLUDE) $(OBJ) -o $(NAME)

$(OBJ_DIR)/%.o: %.cpp
	@$(DIR_DUP)
	@$(CC) $(CFLAGS) -I$(INCLUDE) -o $@ -c $< && printf "Compiling: $(notdir $<)\n"

bonus:
	make -C bonus

clean:
	rm -rf $(OBJ_DIR)
	make -C bonus clean

fclean: clean
	rm -f $(NAME)
	make -C bonus fclean

re: fclean all

.PHONY: all clean fclean re bonus
