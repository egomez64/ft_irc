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

# define SSTR( x ) static_cast< std::ostringstream & >( ( std::ostringstream() << std::dec << x ) ).str()

# include <Client.hpp>
# include <Channel.hpp>

class Client;
class Channel;

class Server
{
private:
	std::string	password;
	int			socket_fd;
	int			epoll_fd;

	std::map<const int, Client>				clients;
	std::map<const std::string&, Channel>	channels;

	static const int	max_events;

	static int	make_socket_non_blocking(int fd);

	int		setSocket(in_port_t);
	int		setEpoll();
	int		acceptNew();
	int		receive(Client &);
	
	Server(void);
	Server(const Server &other);
	Server &operator=(const Server &other);

public:
	Server(in_port_t, const std::string &password = "");
	~Server();

	int		listenLoop();

	Channel		*add_client_to_chan(Client &, const std::string &channel);
};

#endif // SERVER_HPP
