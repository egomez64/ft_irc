#ifndef SERVER_HPP
# define SERVER_HPP

# include <netinet/in.h>
# include <fcntl.h>
# include <sys/epoll.h>
# include <string>
# include <map>
# include <iostream>
# include <ctime>
# include <signal.h>

# include <cstdlib>
# include <cstdio>
# include <cstring>
# include <unistd.h>

# include <Client.hpp>
# include <Channel.hpp>

# define autodef(var, val) __typeof(val) var = val

class Client;
class Channel;

void	signal_handler(int);

class Server
{
private:
	std::string	name;
	std::string	password;
	int			server_fd;
	int			epoll_fd;
	time_t		creation_time;
	bool		running;

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
	void	closeServer()		{ running = false; }

	bool			test_password(const std::string &str) const { return str == password ;}
	bool			test_nickname(const std::string &nickname) const;
	Client			*find_client(const std::string &nickname);
	Channel			*find_channel(const std::string &chan);
	int				remove_client(const Client &);
	int				remove_channel(const Channel &);
	Channel			*add_client_to_chan(Client &, const std::string &channel, const std::string &key);

	const std::string	&get_name() const			{ return name; }
	std::string			get_creation_time() const	{ return std::string(std::ctime(&creation_time)); }
	int					get_epoll_fd() const		{ return epoll_fd; }
};

#endif // SERVER_HPP
