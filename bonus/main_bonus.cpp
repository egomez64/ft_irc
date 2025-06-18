#include <iostream>
#include <limits>

#include <Bot.hpp>

Bot		*g_bot_ptr = NULL;

void	signal_handler(int)
{
	if (g_bot_ptr != NULL)
		g_bot_ptr->closeBot();
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
	if (ac != 4) {
		std::cerr << "Usage: ./censorbot <IPv4> <port> <password>" << std::endl;
		return 1;
	}

	int			port = parse_port(av[2]);
	if (port == -1)
		return 1;

	Bot		censorBot(av[1], port, av[3]);
	
	g_bot_ptr = &censorBot;

	// if (censorBot.listenLoop() == -1)
	// 	return 1;

	return 0;
}
