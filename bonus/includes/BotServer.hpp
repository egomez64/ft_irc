#ifndef BOT_SERVER_HPP
# define BOT_SERVER_HPP

# include <string>
# include <map>
# include <cstdio>
# include <cstring>
# include <cerrno>

# include <arpa/inet.h>
# include <netinet/in.h>
# include <unistd.h>

# include <BotChannel.hpp>
# include <bot_utils.hpp>
# include <numerics.hpp>

class BotServer
{
private:
	enum repls {
		CAP,
		PASS,
		NICK,
		USER,
		PING,
		JOIN,
		PRIVMSG,
		KICK,
		INVITE,
		TOPIC,
		MODE,
		INVALID,
		NICKNAMEINUSE = 433,
		PASSWDMISMATCH = 464,
	};

	int			fd;

	std::string	ip;
	in_port_t	port;
	std::string	password;

	std::string	nickname;
	std::string	username;

	std::string server_name;

	bool		pass;
	bool		auth;

	std::map<const std::string, BotChannel>	channels;

	std::string	stock;

	static int	make_socket_non_blocking(int fd);

	int		set_socket();
	int		authentificate();

	static repls	parse_register(const std::string &);
	static repls	parse_repl(const std::string &);
	static bool		test_nickname(std::string &);
	void			set_auth();

	int		exec_reply(const std::string &);

public:
	enum recv_e {
		RECV_ERR = -1,
		RECV_OVER = 0,
		RECV_OK = 1,
	};

	BotServer(const std::string &ip, in_port_t, const std::string &password = "");
	~BotServer()	{ if (fd != -1) close(fd); }

	recv_e	receive();
	int		send(const std::string &) const;

	int		get_fd() const	{ fd; }
};

#endif // BOT_SERVER_HPP
