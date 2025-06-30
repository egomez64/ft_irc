#ifndef BOT_HPP
# define BOT_HPP

# include <map>
# include <iostream>
# include <sstream>
# include <string>
# include <vector>
# include <limits>

# include <ctime>
# include <cstdlib>
# include <cstdio>
# include <cstring>
# include <cerrno>

# include <netinet/in.h>
# include <fcntl.h>
# include <sys/epoll.h>
# include <signal.h>
# include <arpa/inet.h>
# include <unistd.h>

# include <bot_utils.hpp>

// # include <Client.hpp>
// 
// class Client;

// # include <BotChannel.hpp>

// class BotChannel;

# include <BotServer.hpp>

void	signal_handler(int);

class Bot
{
private:
	int		epoll_fd;
	bool	&running;

	std::map<int, BotServer *>	servers;

	static const int	max_events;

	static int	make_socket_non_blocking(int fd);

	int		setEpoll();
	int		addEpoll(uint32_t events, int fd);
	int		acceptNew();
	// int		receive(Client &);
	
	Bot(void);
	Bot(const Bot &other);
	Bot &operator=(const Bot &other);

public:
	Bot(bool &running);
	Bot(bool &running, const std::string &ip, in_port_t, const std::string &password = "");
	~Bot();

	int		listenLoop();
	int		addServer(const std::string &ip, in_port_t, const std::string &password = "");
	bool	removeServer(int fd);
	void	closeBot()	{ running = false; }

	static int	parse_port(const char *port_str);
};

#endif // BOT_HPP
