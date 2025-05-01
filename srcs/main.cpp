#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>

#include "../includes/parsing.hpp"

int main(int ac, char **av)
{
	if (ac != 3)
		return 1;

	in_port_t	port = parse_port(av[1]);

	int			socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in	addr{};
	// sockaddr_storage	sto{};

	addr.sin_family = AF_INET;
	addr.sin_port = port;
	// // sto.

	int		fd = epoll_create1(0);
}