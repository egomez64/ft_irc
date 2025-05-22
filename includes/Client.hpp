#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>
# include <map>
# include <utility>

# include <Server.hpp>

class Server;
class Channel;

class Client
{
private:
	enum cmds {
		CAP,
		PASS,
		NICK,
		USER,
		PRIVMSG,
		KICK,
		INVITE,
		TOPIC,
		MODE,
		INVALID,
	};

	const int	fd;
	Server		&serv;

	std::string	nickname;
	std::string	username;
	bool		auth;

	std::map<const std::string &, Channel &>	channels;

	std::string	stock;

	cmds	registering(const std::string &);
	cmds	parse_cmd(const std::string &);
	int		join(const std::string &chan_name);
	// int		join(std::string &chan_name, std::string &password);
	int		msg_chan(const std::string &chan, const std::string &msg);

public:
	Client(int fd, Server &serv) : fd(fd), serv(serv), auth(false) { (void)auth; };
	~Client() { /*close(fd);*/ }

	int		receive();
	int		send(const std::string &) const;

	const int			&get_fd() const			{ return fd; }
	const std::string	&get_nickname() const	{ return nickname; }
	const std::string	&get_username() const	{ return username; }

	void	set_nickname(const std::string &str)	{ nickname = str; }
	void	set_username(const std::string &str)	{ username = str; }
};

#endif // CLIENT_HPP
