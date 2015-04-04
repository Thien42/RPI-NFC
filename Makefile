SRC		=	./src/main.cpp	\
			./src/Term.cpp	\
			./src/Options.cpp

OBJ		=	$(SRC:.cpp=.o)

NAME	=	test-nfc

CXX		=	g++

CXXFLAGS=	$(shell pkg-config --cflags libctacs) -I./inc -fpermissive

LDFLAGS	=	$(shell pkg-config --libs libctacs)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) -o $(NAME) $(OBJ) $(LDFLAGS)

clean:
	$(RM) -f $(OBJ)

fclean: clean
	$(RM) -f $(NAME)

re: fclean all
