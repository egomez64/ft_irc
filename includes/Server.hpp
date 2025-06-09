#ifndef SERVER_HPP
# define SERVER_HPP

# include <netinet/in.h>
# include <fcntl.h>
# include <sys/epoll.h>
# include <string>
# include <map>
# include <iostream>
# include <sstream>

# include <cstdlib>
# include <cstdio>
# include <cstring>
# include <unistd.h>

# define SSTR( x ) static_cast< std::ostringstream & >( ( std::ostringstream() << std::dec << x ) ).str()
# define PRINT( x ) std::cout << x << '\n'

# include <Client.hpp>
# include <Channel.hpp>

class Client;
class Channel;

class Server
{
private:
	std::string	password;
	int			server_fd;
	int			epoll_fd;

	std::map<const int, Client>				clients;
	std::map<const std::string, Channel>	channels;

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

	bool			test_password(const std::string &);
	Channel			*add_client_to_chan(Client &, const std::string &channel);
	bool			nick_test(const std::string &nickname);
	const Client	*findClient(const std::string &nickname) const;
};

#endif // SERVER_HPP
