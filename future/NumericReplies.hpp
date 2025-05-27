#ifndef NUMERIC_REPLIES_HPP
# define NUMERIC_REPLIES_HPP

# include <Server.hpp>
# include <Client.hpp>
# include <string>

# define SERVER_NAME "ircserv"
# define REPLY( x ) :SERVER_NAME #x <target> NUM_REP( x ) "\r\n"

# define NUM_REP( x )

# define NUM_REP_433 "Nickname is already in use"

class NumericReplies
{
private:
	static const in_port_t		port;

	enum	Numerics {
		ERR_NICKNAMEINUSE = 433,
	};

	~NumericReplies() {};

public:
	NumericReplies() {};
	static std::string	format(int num, const std::string &msg);
	static std::string	reply(int num, const Client &);
};



#endif // NUMERIC_REPLIES_HPP
