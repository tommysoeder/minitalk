NAME		= minitalk
CC			= cc
CFLAGS		= -Wall -Wextra -Werror

SRCS_SERVER	= server.c utils.c
SRCS_CLIENT	= client.c utils.c

OBJS_SERVER	= $(SRCS_SERVER:.c=.o)
OBJS_CLIENT	= $(SRCS_CLIENT:.c=.o)

all: $(NAME)

$(NAME): server client

server: $(OBJS_SERVER)
	$(CC) $(CFLAGS) $(OBJS_SERVER) -o $@

client: $(OBJS_CLIENT)
	$(CC) $(CFLAGS) $(OBJS_CLIENT) -o $@

clean:
	rm -f $(OBJS_SERVER) $(OBJS_CLIENT)

fclean: clean
	rm -f server client

re: fclean all

.PHONY: all clean fclean re
