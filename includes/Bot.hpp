#ifndef BOT_HPP
# define BOT_HPP

# include <netinet/in.h>
# include <fcntl.h>
# include <sys/epoll.h>
# include <string>
# include <map>
# include <iostream>
# include <sstream>
# include <ctime>
# include <signal.h>
# include <arpa/inet.h>

# include <cstdlib>
# include <cstdio>
# include <cstring>
# include <unistd.h>

# define SSTR( x ) static_cast< std::ostringstream & >( ( std::ostringstream() << std::dec << x ) ).str()
# define PRINT( x ) std::cout << x << '\n'

// # include <Client.hpp>
// 
// class Client;

# include <Channel.hpp>

class Channel;

void	signal_handler(int);

class Bot
{
private:
	std::string	nickname;
	int			server_fd;
	int			epoll_fd;
	bool		running;

	std::map<const std::string, Channel>	channels;

	static const int	max_events;

	static int	make_socket_non_blocking(int fd);

	int		setSocket(const std::string &ip, in_port_t);
	int		setEpoll();
	int		connect_to_serv(const std::string &password);
	// int		acceptNew();
	int		send(const std::string &) const;
	// int		receive(Client &);
	
	Bot(void);
	Bot(const Bot &other);
	Bot &operator=(const Bot &other);

public:
	Bot(const std::string &ip, in_port_t, const std::string &password = "");
	~Bot();

	int		listenLoop();
	void	closeBot()	{ running = false; }

	// bool			test_nickname(const std::string &nickname) const;
	// const Client	*find_client(const std::string &nickname) const;
	// Channel			*find_channel(const std::string &chan);
	// int				remove_client(const Client &);
	// Channel			*add_client_to_chan(Client &, const std::string &channel, const std::string &key);

	const std::string	&get_nickname() const			{ return nickname; }
};

#endif // BOT_HPP
