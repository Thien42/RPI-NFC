SRC		=	./src/main.cpp

OBJ		=	$(SRC:.cpp=.o)

NAME	=	test-nfc

CXX		=	g++

CXXFLAGS=	$(shell pkg-config --cflags libctacs) -I./inc

LDFLAGS	=	$(shell pkg-config --libs libctacs)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) -o $(NAME) $(OBJ) $(LDFLAGS)

clean:
	$(RM) -f $(OBJ)

fclean: clean
	$(RM) -f $(NAME)

re: fclean all
