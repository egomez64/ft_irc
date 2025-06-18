#include <iostream>
#include <limits>

#include "../includes/Server.hpp"

Server	*g_serv_ptr = NULL;

void	signal_handler(int)
{
	if (g_serv_ptr != NULL)
		g_serv_ptr->closeServer();
}

static int	parse_port(char *port_str)
{
	int		port = std::atoi(port_str);
	if (port <= 0 || port > std::numeric_limits<in_port_t>::max()) {
		std::cerr << "Invalid port number" << std::endl;
		return -1;
	}
	return port;
}

int main(int ac, char **av)
{
	if (ac != 3) {
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return 1;
	}

	int		port = parse_port(av[1]);
	if (port == -1)
		return 1;

	Server	server(port, av[2]);
	
	g_serv_ptr = &server;

	if (server.listenLoop() == -1)
		return 1;

	return 0;
}
