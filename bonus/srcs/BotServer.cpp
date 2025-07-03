#include "BotServer.hpp"

int BotServer::set_socket()
{
	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		std::perror("socket");
		return -1;
	}

	if (make_socket_non_blocking(fd) == -1) {
		std::perror("fcntl");
		return -1;
	}

	sockaddr_in		server_addr;
	std::memset(&server_addr, 0, sizeof (server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);

	if (inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) != 1) {
		std::perror("inet_pton");
		return -1;
	}

	if (connect(fd, (struct sockaddr *) &server_addr, sizeof (server_addr)) == -1 && errno != EINPROGRESS) {
		std::perror("connect");
		return -1;
	}

	PRINT("Bot connecting to " << ip << " on port " << port);

	return fd;
}

int BotServer::authentificate()
{
	std::string		to_send;

	if (!password.empty())
		to_send.append("PASS " + password + "\r\n");
	to_send.append("NICK " + nickname + "\r\n");
	to_send.append("USER " + username + "\r\n");

	//TODO: check errors (password incorrect, nickname already taken, etc)

	return send(to_send);
}

bool BotServer::try_new_nickname()
{
	if ()
}

int BotServer::exec_reply(const std::string &repl)
{
	std::vector<std::string>	split_repl = split(repl);
	if (split_repl.empty())
		return -1;

	switch (auth ? parse_repl(split_repl[0]) : parse_register(split_repl[0]))
	{
		std::size_t	pos;

	// case PASS:
	// 	if (split_repl.size() != 2) {
	// 		send(ERR_NEEDMOREPARAMS(nickname, repl));
	// 		return -1;
	// 	}
	// 	pass = test_password(split_repl[1]);
	// 	if (!pass) {
	// 		send(ERR_PASSWDMISMATCH(nickname));
	// 		return -1;
	// 	}
	// 	break;
	
	case PASSWDMISMATCH:
		PRINT_ERR(repl.substr(0, repl.find("\r\n")));
		return -1;

	case NICK:
		// if (!pass) {
		// 	send(ERR_PASSWDMISMATCH(nickname));
		// 	return -1;
		// }
		// if (split_repl.size() != 2) {
		// 	send(ERR_NONICKNAMEGIVEN(nickname, repl));
		// 	return -1;
		// }
		// if (!test_nickname(split_repl[0])) {
		// 	send(ERR_ERRONEUSNICKNAME(nickname, split_repl[0]));
		// 	return -1;
		// }
		// if (!test_nickname(split_repl[1])) {
		// 	send(ERR_NICKNAMEINUSE(split_repl[1]));
		// 	return -1;
		// }
		if (!auth) {
			nickname = split_repl[1];
			set_auth();
		}
		// else {
		// 	PRINT("new nickname: " << split_repl[1]);
		// 	send_friends(RPL_NICK(nickname, split_repl[1]));
		// 	send(RPL_NICK(nickname, split_repl[1]));
		// 	nickname = split_repl[1];
		// }
		break;
	
	case NICKNAMEINUSE:
		PRINT_ERR(repl.substr(0, repl.find("\r\n")));
		return -1;

	case USER:
		// if (!pass) {
		// 	send(ERR_PASSWDMISMATCH(nickname));
		// 	return -1;
		// }
		// if (split_repl.size() < 2 || split_repl[1].length() < 1) {
		// 	send(ERR_NEEDMOREPARAMS(nickname, repl));
		// 	return -1;
		// }
		username = split_repl[1];
		set_auth();
		break;

	case PING:
		if (split_repl.size() < 2) {
			send(ERR_NEEDMOREPARAMS(nickname, repl));
			return -1;
		}
		send("PONG " + repl.substr(repl.find_first_of(' ') + 1) + "\r\n");
		break;

	case JOIN:
		// if (split_repl.size() < 2) {
		// 	send(ERR_NEEDMOREPARAMS(nickname, repl));
		// 	return -1;
		// }
		// if (split_repl.size() == 2)
		// 	join(split_repl[1]);
		// else
		// 	join(split_repl[1], split_repl[2]);
		join_channel(split_repl[2]);
		break;

	// case PRIVMSG:
	// 	if (split_repl.size() < 3) {
	// 		if (split_repl.size() == 1)
	// 			send(ERR_NORECIPIENT(nickname, repl));
	// 		else
	// 			send(ERR_NOTEXTTOSEND(nickname));
	// 		return -1;
	// 	}
	// 	privmsg(split_repl[1], PRIVMSG(nickname, split_repl[1], repl.substr(repl.find_first_of(':') + 1)));
	// 	break;
	
	case KICK:
	// 	// if (split_repl.size() < 3) {
	// 	// 	send(ERR_NEEDMOREPARAMS(nickname, repl));
	// 	// 	return -1;
	// 	// }
	// 	pos = repl.find_first_of(':');
	// 	if (pos == std::string::npos)
	// 		kick(split_repl[1], split_repl[2], "No reason specified");
	// 	else
	// 		kick(split_repl[1], split_repl[2], repl.substr(pos + 1));
		break;

	case INVITE:
		// if (split_repl.size() < 3) {
		// 	send(ERR_NEEDMOREPARAMS(nickname, repl));
		// 	return -1;
		// }
		invited(split_repl[3]);
		break;

	// case TOPIC:
	// 	if (split_repl.size() < 2) {
	// 		send(ERR_NEEDMOREPARAMS(nickname, repl));
	// 		return -1;
	// 	}
	// 	pos = repl.find_first_of(':');
	// 	if (pos == std::string::npos) {
	// 		see_topic(split_repl[1]);
	// 		break;
	// 	}
	// 	topic(split_repl[1], repl.substr(pos + 1));
	// 	break;
	
	case MODE:
		// if (split_repl.size() < 2) {
		// 	send(ERR_NEEDMOREPARAMS(nickname, repl));
		// 	return -1;
		// }
		// if (split_repl.size() == 2) {
		// 	mode(split_repl[1], "");
		// 	break;
		// }
		// mode(split_repl[1], repl.substr(repl.find_first_of(' ', repl.find_first_of(' ') + 1) + 1));
		break;

	case INVALID:
		// if (!auth) {
		// 	if (!pass)
		// 		send(ERR_PASSWDMISMATCH(nickname));
		// 	else if (parse_repl(split_repl[0]) != INVALID)
		// 		send(ERR_NOTREGISTERED());
		// }
		// else if (auth && parse_register(split_repl[0]) != INVALID)
		// 	send(ERR_ALREADYREGISTRED(nickname));
		// else
		// 	send(ERR_UNKNOWNCOMMAND(nickname, repl));
		PRINT_ERR(server_name << ": unknown command: " << repl.substr(0, repl.find("\r\n")));
		return -1;

	default:
		break;
	}
	return 0;
}

int BotServer::join_channel(const std::string &channel)
{
	std::pair<std::map<const std::string, BotChannel>::iterator, bool>	inserted;

	inserted = channels.insert(std::make_pair(channel, BotChannel(channel)));
	if (!inserted.second)
		return -1;
	return 0;
}

BotServer::BotServer(const std::string &ip, in_port_t port, const std::string &password)
	: ip(ip)
	, port(port)
	, password(password)
	, nickname("censorBot")
	, username("Censor_Bot")
{
	set_socket();
	authentificate();
}

BotServer::recv_e BotServer::receive()
{
	static const int	buff_size = 1024;
	char				buff[buff_size];

	ssize_t	bytes_read = recv(fd, buff, buff_size - 1, 0);
	if (bytes_read == -1) {
		// std::perror("recv");
		return RECV_ERR;
	}
	else if (bytes_read == 0) {
		// send_friends(RPL_QUIT(nickname, "Connection closed"));
		return RECV_OVER;
	}

	buff[bytes_read] = '\0';			//TODO: remove
	PRINT("<< " << buff);
	stock.append(buff, bytes_read);

	std::size_t		end_msg;
	while ((end_msg = stock.find("\r\n")) != std::string::npos) {
		std::string		reply(stock, 0, end_msg);
		parse_repl(reply);
		stock.erase(0, end_msg + 2);
	}
	return RECV_OK;
}

int BotServer::send(const std::string &str) const
{
	ssize_t	bytes_sent = ::send(fd, str.c_str(), str.length(), MSG_NOSIGNAL);
	if (bytes_sent == -1) {
		std::perror("send");
		return -1;
	}
	return 0;
}
