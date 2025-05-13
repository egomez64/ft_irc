#ifndef SERVER_HPP
# define SERVER_HPP

# include <netinet/in.h>
# include <fcntl.h>
// # include <sys/types.h>
// # include <sys/socket.h>
# include <sys/epoll.h>
# include <string>
# include <map>
# include <iostream>
# include <sstream>

# include <cstdlib>
# include <cstdio>
# include <cstring>
// # include <cerrno>
# include <unistd.h>

#define SSTR( x ) static_cast< std::ostringstream & >( ( std::ostringstream() << std::dec << x ) ).str()

# define MAX_EVENTS 10

// class Client;
struct Client {
	std::string		received;
	bool			rcv_ended;
};

class Server
{
private:
	std::string	password;
	int			socket_fd;
	int			epoll_fd;

	std::map<int, Client>	clients;

	static const int	max_events;

	static int	make_socket_non_blocking(int fd);

	int		setSocket(in_port_t port);
	int		setEpoll();
	int		acceptNew();
	int		receive(int fd);
	
	Server(void);
	Server(const Server &other);
	Server &operator=(const Server &other);

public:
	Server(in_port_t port, const std::string &password = "");
	~Server();

	int		listenLoop();
};

#endif // SERVER_HPP
