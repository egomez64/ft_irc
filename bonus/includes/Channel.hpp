#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <iostream>
# include <map>
# include <set>
# include <algorithm>
# include <ctime>
# include <cerrno>

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
	modes_t		modes;
	Server		&server;

	std::map<const std::string, Client &>	clients;
	std::set<std::string>					operators;
	std::set<std::string>					invite_list;

	std::string		users_str() const;
	std::string		modes_str() const;
	void			remove_client(std::map<const std::string, Client &>::iterator client);

public:
	// constructor
	Channel(const std::string &name, Client &, Server &);

	// class methods
	int		join(Client &, const std::string &key);
	int		part(Client &, const std::string &reason);
	int		msg(const std::string &msg);
	int		msg(Client &, const std::string &msg);

	bool	is_operator(const std::string &) const;
	int		kick(Client& client, const std::string &target, const std::string &reason);
	int		invite(Client& client, Client &target);
	int		see_topic(Client & client);
	int		change_topic(Client & client, const std::string &);
	int		change_modes(Client &, const std::string &);

	void	remove_client(Client &);

	// getters
	const std::string	&getName() const		{ return name; }
	const std::string	&getPassword() const	{ return modes.key; }
	const std::map<const std::string, Client &>
						&getClients() const		{ return clients; }

	// setters
	void	setName(const std::string &_name)			{ name = _name; }
	void	setPassword(const std::string &_password)	{ modes.key = _password; }
	void	setTopic(const std::string &topic, const std::string &nick);
};

inline bool operator<(const Channel& x, const Channel& y) {return (x.getName() < y.getName());}

#endif // CHANNEL_HPP
