#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <iostream>
# include <map>
# include <set>
# include <algorithm>

# include <Client.hpp>

class Client;
class Server;

class Channel {
public:

	struct modes_t {
		bool	invite;
		bool	topic;
		bool	key;
		int		limit;

		modes_t() : invite(false), topic(false), key(false), limit(-1) {};
	};

	enum operator_mode {
		GRANT,
		REMOVE,
	};

private:
	enum modes_e {
		MODES_INVITE = 'i',
		MODES_TOPIC = 't',
		MODES_KEY = 'k',
		MODES_OPERATOR = 'o',
		MODES_LIMIT = 'l',
		MODES_UNKNOWN,
	};

	std::string		name;
	std::string		topic;
	std::string		key;

	Server	&serv;

	std::map<const std::string, Client &>	clients;
	std::set<std::string>					operators;

	modes_t		modes;

	std::string		users() const;
	// static modes_e	parse_mode(char modechar);
	void			remove_client(const std::string &nickname);

public:

	// constructor
	Channel(const std::string &_name, Server &serv): name(_name), serv(serv) {}

	// class methods
	int		add_client(Client &);
	// int		add_client(Client &, std::string &key);
    int     msg(const std::string &msg);
    int     msg(const Client &, const std::string &msg);

	bool	is_operator(const std::string &) const;
	int     kick(const Client& _operator, const std::string &target, const std::string &reason);
	int     invite(const Client& _operator, Client& target);
	int     change_topic(const Client & _operator, std::string &);
	int     change_modes(Client &, const std::string &modestring);
	int     change_client_mode(const Client& _operator, Client& target, operator_mode);

	// getters
	const std::string	&getName() const		{ return name; }
	const std::string	&getTopic() const		{ return topic; }
	const std::string	&getPassword() const	{ return key; }

	// setters
	void	setName(const std::string &_name)			{ name = _name; }
	void	setTopic(const std::string &_topic)			{ topic = _topic; }
	void	setPassword(const std::string &_password)	{ key = _password; }
};

inline bool operator<(const Channel& x, const Channel& y) {return (x.getName() < y.getName());}

#endif // CHANNEL_HPP
