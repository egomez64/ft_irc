#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <iostream>
# include <map>
# include <vector>

# include <Client.hpp>

class Client;
class Server;

class Channel {
public:
	struct modes_t {
		bool i;
		bool t;
		bool k;
		int l;

		modes_t() : i(false), t(false), k(false), l(-1) {};
	};

	enum operator_mode {
		GRANT,
		REMOVE,
	};

private:
	std::string		name;
	std::string		topic;
	std::string		password;

	Server	&serv;

	std::map<const int, Client &>	clients;
	std::vector<int>				operators;

	modes_t		modes;

public:

	// constructor
	Channel(const std::string &_name, Server &serv): name(_name), serv(serv) {}

	// class methods
	int		add_client(Client &);
	// int		add_client(Client &, std::string &password);
    int     msg(const Client &, const std::string &msg);

	int     kick(const Client& _operator, Client& target);
	int     invite(const Client& _operator, Client& target);
	int     change_topic(const Client & _operator, std::string &);
	int     change_modes(const Client & _operator, modes_t &);
	int     change_client_mode(const Client& _operator, Client& target, operator_mode);

	// getters
	const std::string	&getName() const		{ return name; }
	const std::string	&getTopic() const		{ return topic; }
	const std::string	&getPassword() const	{ return password; }

	// setters
	void	setName(const std::string &_name)			{ name = _name; }
	void	setTopic(const std::string &_topic)			{ topic = _topic; }
	void	setPassword(const std::string &_password)	{ password = _password; }
};

#endif // CHANNEL_HPP
