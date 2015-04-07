SRC		=	./src/main.cpp				\
			./src/Term.cpp				\
			./src/Options.cpp			\
			./src/CardTerminalList.cpp	\
			./src/CardTerminal.cpp

OBJ		=	$(SRC:.cpp=.o)

NAME	=	test-nfc

CXX		=	g++

CXXFLAGS=	$(shell pkg-config --cflags libpcsclite) -I./inc --std=c++11 -W -Wall -Wextra

LDFLAGS	=	$(shell pkg-config --libs libpcsclite)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) -o $(NAME) $(OBJ) $(LDFLAGS)

clean:
	$(RM) -f $(OBJ)

fclean: clean
	$(RM) -f $(NAME)

re: fclean all
