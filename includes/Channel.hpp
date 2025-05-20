#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <map>
#include <vector>
#include <Client.hpp>

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

	std::map<int, Client &>	clients;
	std::vector<int>		operators;

	modes_t		modes;

public:

	// constructor
	Channel(std::string &_name) : name(_name) {}

	// class methods
	int		add_client(Client &);
	// int		add_client(Client &, std::string &password);
    int     msg(Client &, std::string &msg);

	int     kick(Client& _operator, Client& target);
	int     invite(Client& _operator, Client& target);
	int     change_topic(Client &, std::string &);
	int     change_modes(Client &, modes_t &);
	int     change_operator(Client& _operator, Client& target, operator_mode);

	// getters
	const std::string	&getName() const		{ return name; }
	const std::string	&getTopic() const		{ return topic; }
	const std::string	&getPassword() const	{ return password; }

	// setters
	void	setName(std::string &_name)			{ name = _name; }
	void	setTopic(std::string &_topic)		{ topic = _topic; }
	void	setPassword(std::string &_password)	{ password = _password; }
};

#endif // CHANNEL_HPP