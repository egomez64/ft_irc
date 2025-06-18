NAME	= ircserv
NAMEBOT	= censorbot
CC		= c++
CFLAGS	= -Wall -Werror -Wextra
CFLAGS	+= --std=c++98
INCLUDE	= includes

DIR_DUP	= mkdir -p $(@D)
OBJ_DIR	= .obj
SRC_DIR	= srcs

SRC		= $(SRC_DIR)/main.cpp \
		$(SRC_DIR)/Server.cpp \
		$(SRC_DIR)/Client.cpp \
		$(SRC_DIR)/Channel.cpp \
		$(SRC_DIR)/utils.cpp

OBJ		:= $(SRC:%.cpp=$(OBJ_DIR)/%.o)

BONUS_DIR	= bonus
SRC_BONUS	= $(BONUS_DIR)/main_bonus.cpp \
			$(BONUS_DIR)/Bot.cpp

OBJ_BONUS	:= $(SRC_BONUS:%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -I$(INCLUDE) $(OBJ) -o $(NAME)

$(OBJ_DIR)/%.o: %.cpp
	@$(DIR_DUP)
	@$(CC) $(CFLAGS) -I$(INCLUDE) -o $@ -c $< && printf "Compiling: $(notdir $<)\n"

bonus: $(OBJ_BONUS)
	$(CC) $(CFLAGS) -I$(INCLUDE) $(OBJ_BONUS) -o $(NAMEBOT)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re bonus
