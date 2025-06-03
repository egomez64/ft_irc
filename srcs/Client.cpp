#include <Client.hpp>

Client::cmds Client::parse_register(const std::string &str)
{
	if (str == "CAP")
		return CAP;
	else if (str == "PASS")
		return PASS;
	else if (str == "NICK")
		return NICK;
	else if (str == "USER")
		return USER;
	else
		return INVALID;
}

Client::cmds Client::parse_cmd(const std::string &str)
{
	if (str == "JOIN")
		return JOIN;
	else if (str == "PRIVMSG")
		return PRIVMSG;
	else if (str == "KICK")
		return KICK;
	else if (str == "INVITE")
		return INVITE;
	else if (str == "TOPIC")
		return TOPIC;
	else if (str == "MODE")
		return MODE;
	else
		return INVALID;
}

void Client::check_auth()
{
	PRINT("check auth");
	PRINT("auth: " << auth);
	PRINT("pass: " << pass);
	PRINT("nick: " << nickname);
	PRINT("user: " << username);
	auth = pass && !nickname.empty() && !username.empty();
	PRINT("auth: " << auth);
	if (!auth)
		return;

	std::cout << username << " \"" << nickname << "\" authentificated.\n";
	const std::string	msg("Welcome to our Server " + username +" \"" + nickname + "\""  +  ".\n\r");
	send(msg);
}

// enum cmds {
// 	CAP,
// 	PASS,
// 	NICK,
// 	USER,
// 	PRIVMSG,
// 	KICK,
// 	INVITE,
// 	TOPIC,
// 	MODE,
// 	INVALID,
// };

int Client::exec_cmd(const std::string &cmd)
{
	std::vector<std::string>	split_cmd = split(cmd);
	if (split_cmd.empty())
		return -1;

	switch (auth ? parse_cmd(split_cmd[0]) : parse_register(split_cmd[0]))
	{
		std::size_t	pos;

	case PASS:
		if (split_cmd.size() != 2)
			return -1;
		pass = serv.test_password(split_cmd[1]);
		check_auth();
		break;

	case NICK:
		if (split_cmd.size() != 2)
			return -1;
		if (!serv.nick_test(split_cmd[1])) {
			send(ERR_NICKNAMEINUSE(split_cmd[1]));
			return -1;
		}
		nickname = split_cmd[1];
		check_auth();
		break;

	case USER:
		if (split_cmd.size() < 2)
			return -1;
		username = split_cmd[1];
		check_auth();
		break;

	case INVALID:
		return -1;

	case JOIN:
		if (split_cmd.size() < 2)
			return -1;
		join(split_cmd[1]);
		break;

	case PRIVMSG:
		if (split_cmd.size() < 3) {
			if (split_cmd.size() == 1)
				send(ERR_NORECIPIENT(nickname, cmd));
			else
				send(ERR_NOTEXTTOSEND(nickname));
			return -1;
		}
		pos = cmd.find_first_of(':') + 1;
		// privmsg(split_cmd[1], ":" + nickname + " PRIVMSG " + split_cmd[1] + " :" + cmd.substr(pos, cmd.length() - pos));
		privmsg(split_cmd[1], PRIVMSG(nickname, split_cmd[1], cmd.substr(pos, cmd.length() - pos)));
		break;
	
	case KICK:
		if (split_cmd.size() < 3) {
			send(ERR_NEEDMOREPARAMS(nickname, cmd));
			return -1;
		}
		kick(split_cmd[1], split_cmd[2]);
		break;

	default:
		break;
	}
	return 0;
}

int Client::join(const std::string &chan_name)
{
	Channel		*chan;

	if (chan_name[0] != '#') {
		send(ERR_BADCHANMASK(chan_name));
		return -1;
	}
	if ((chan = serv.add_client_to_chan(*this, chan_name)) == NULL) {
		std::cout << "Couldn't add_client\n";
		return -1;
	}
	channels.insert(std::pair<const std::string, Channel &>(chan_name, *chan));
	// std::string msg("Welcome to the channel " + chan_name + "\r\n");
	// send(msg);
	return 0;
}

int Client::privmsg(const std::string &target, const std::string &msg)
{
	if (target[0] == '#') {
		std::map<const std::string, Channel &>::iterator	it;
	
		if ((it = channels.find(target)) == channels.end()) {
			send(ERR_CANNOTSENDTOCHAN(nickname, target));
			return -1;
		}
		it->second.msg(*this, msg);
	}
	else {
		const Client	*client;
		if ((client = serv.findClient(target)) == NULL) {
			send(ERR_NOSUCHNICK(nickname, target));
			return -1;
		}
		client->send(msg);
	}
	return 0;
}

int Client::kick(const std::string &chan, const std::string &user, const std::string &reason)
{
	std::map<const std::string, Channel &>::iterator it = channels.find(chan);
	if ( it == channels.end()){
		send(ERR_NOTONCHANNEL(nickname, chan));
		return -1;
	}
	it->second.kick(*this, user, reason);
	return 0;
}

int Client::receive()
{
	std::cout << "Client " << fd << ": receive\n";
	static const int	buff_size = 1024;
	char				buff[buff_size];

	ssize_t	bytes_read = recv(fd, buff, buff_size - 1, 0);
	if (bytes_read == -1) {
		std::perror("recv");
		return -1;
	}
	else if (bytes_read == 0) {
		close(fd);
		return -1;
	}
	buff[bytes_read] = '\0';
	stock.append(buff, bytes_read);

	std::size_t		end_msg;
	while ((end_msg = stock.find("\r\n")) != std::string::npos) {
	// while ((end_msg = stock.find("\n")) != std::string::npos) {
		std::string		cmd(stock, 0, end_msg);
		std::cout << cmd << '\n';
		exec_cmd(cmd);
		stock.erase(0, end_msg + 2);
		// stock.erase(0, end_msg + 1);
	}
	return 0;
}

int Client::send(const std::string &str) const
{
	// std::string		msg = SSTR("# " << fd << ": received.\n");
	ssize_t	sent = ::send(fd, str.c_str(), str.length(), MSG_NOSIGNAL);
	if (sent == -1) {
		std::perror("send");
		return -1;
	}
	return 0;
}

int Client::remove_chan(const std::string &chan)
{
	if (channels.erase(chan) == 0)
		return -1;
	return 0;
}
