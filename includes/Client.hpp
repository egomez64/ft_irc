#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <map>
#include <utility>
#include <Server.hpp>
#include "Channel.hpp"

class Channel;

class Client
{
private:
	int fd;

	std::string nickname;
	std::string username;

	std::map<std::string&, Channel&> channels;
	Server &serv;

	static const int	buff_size = 1024;
	char	buff[buff_size];
	std::string	stock;

	int		parse_cmd(std::string &);
	int		join(std::string &chan_name);
	// int		join(std::string &chan_name, std::string &password);
	int		msg_chan(std::string &chan, std::string &msg);

public:
	Client(int fd, Server &serv) : fd(fd), serv(serv) {};

	int		receive();
	int		send(std::string &);

	const std::string	&get_nickname() const { return nickname; }
	const std::string	&get_username() const { return username; }

	void	set_nickname(std::string &str) { nickname = str; }
	void	set_username(std::string &str) { username = str; }
};

#endif // CLIENT_HPP
