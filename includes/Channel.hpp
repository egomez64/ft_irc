#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <iostream>
# include <map>
# include <set>
# include <algorithm>
# include <ctime>

# include <Client.hpp>

class Client;
class Server;

class Channel {
private:
	struct modes_t {
		bool		invite;
		bool		topic;
		std::string	key;
		std::size_t	limit;

		modes_t() : invite(false), topic(false), limit(0) {};
	};

	struct topic_t {
		std::string	topic;
		std::string	nick;
		time_t		setat;

		topic_t() : setat(std::time(NULL)) {};
	};

	enum modes_e {
		MODES_INVITE = 'i',
		MODES_TOPIC = 't',
		MODES_KEY = 'k',
		MODES_OPERATOR = 'o',
		MODES_LIMIT = 'l',
		MODES_UNKNOWN,
	};

	std::string	name;
	topic_t		topic;

	Server	&serv;

	std::map<const std::string, Client &>	clients;
	std::set<std::string>					operators;

	modes_t		modes;

	std::string		users_str() const;
	std::string		modes_str() const;
	void			remove_client(const std::string &nickname);

public:
	// constructor
	Channel(const std::string &name, Client &, Server &serv);

	// class methods
	int		add_client(Client &, const std::string &key);
	int		msg(const std::string &msg);
	int		msg(const Client &, const std::string &msg);

	bool	is_operator(const std::string &) const;
	int		kick(const Client& _operator, const std::string &target, const std::string &reason);
	int		invite(const Client& _operator, Client& target);
	int		see_topic(const Client & client);
	int		change_topic(const Client & client, const std::string &);
	int		change_modes(Client &, const std::string &);

	// getters
	const std::string	&getName() const		{ return name; }
	const std::string	&getPassword() const	{ return modes.key; }

	// setters
	void	setName(const std::string &_name)			{ name = _name; }
	void	setPassword(const std::string &_password)	{ modes.key = _password; }
	void	setTopic(const std::string &topic, const std::string &nick);
};

inline bool operator<(const Channel& x, const Channel& y) {return (x.getName() < y.getName());}

#endif // CHANNEL_HPP
