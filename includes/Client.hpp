#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>
# include <map>
# include <utility>

# include <Server.hpp>
# include <utils.hpp>
# include <numerics.hpp>

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
		JOIN,
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
	bool		pass;
	bool		auth;

	std::map<const std::string, Channel &>	channels;

	std::string	stock;

	static cmds		parse_register(const std::string &);
	static cmds		parse_cmd(const std::string &);
	void	check_auth();
	int		exec_cmd(const std::string &);
	int		join(const std::string &chan, const std::string key = "");
	int		privmsg(const std::string &target, const std::string &msg);
	int		kick(const std::string &chan, const std::string &user, const std::string &reason = "");
	// int		invite();
	int		see_topic(const std::string &chan);
	int		topic(const std::string &chan, const std::string &topic);
	int		mode(const std::string &target, const std::string &);

public:
	Client(int fd, Server &serv) : fd(fd), serv(serv), pass(false), auth(false) { (void)auth; };
	~Client() { /*close(fd);*/ }

	int		receive();
	int		send(const std::string &) const;

	const int			&get_fd() const			{ return fd; }
	const std::string	&get_nickname() const	{ return nickname; }
	const std::string	&get_username() const	{ return username; }

	void	set_nickname(const std::string &str)	{ nickname = str; }
	void	set_username(const std::string &str)	{ username = str; }

	int	remove_chan(const std::string &chan);
};

#endif // CLIENT_HPP
