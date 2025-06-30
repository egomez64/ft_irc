#include <iostream>
#include <limits>

#include <Bot.hpp>

bool	g_bot_running = true;

void	signal_handler(int)
{
	g_bot_running = false;
}

int main(int ac, char **av)
{
	Bot		*censorBot;
	if (ac != 4) {
		std::cout << "Usage: ./censorbot <IPv4> <port> <password>" << std::endl;

		censorBot = new Bot(g_bot_running);
	}
	else {
		int		port = Bot::parse_port(av[2]);
		if (port == -1)
			return 1;
	
		censorBot = new Bot(g_bot_running, av[1], port, av[3]);
	}
	if (censorBot == NULL)
		return 1;

	signal(SIGINT, signal_handler);

	if (censorBot->listenLoop() == -1)
		return 1;

	return 0;
}
