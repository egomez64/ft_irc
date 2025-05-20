#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <map>
#include "Channel.hpp"

class Channel;

class Client
{
	private:
		int fd;

		std::string nickname;
		std::string username;

		std::map<std::string&, Channel&> channels;

		static const int	buff_size = 1024;
		char	buff[buff_size];
		std::string	stock;

		int parse_cmd();

	public:
		int receive();
		int send(std::string &str);
};

#endif // CLIENT_HPP
