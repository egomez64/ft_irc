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

	switch (auth ? parse_cmd(split_cmd[0]) : parse_register(split_cmd[0]))
	{
	case NICK:
		break;

	case USER:
		break;

	case INVALID:
		return -1;

	default:
		break;
	}
	return 0;
}

int Client::join(const std::string &chan_name)
{
	Channel		*chan;

	if ((chan = serv.add_client_to_chan(*this, chan_name)) == NULL) {
		std::cout << "Couldn't add_client\n";
		return -1;
	}
	channels.insert(std::pair<const std::string &, Channel &>(chan->getName(), *chan));
	return 0;
}

int Client::msg_chan(const std::string &chan, const std::string &msg)
{
	std::map<const std::string &, Channel &>::iterator	it;

	if ((it = channels.find(chan)) == channels.end()) {
		std::cerr << "You're not in this channel.\n";
		return -1;
	}
	it->second.msg(*this, msg);
	return 0;
}

int Client::receive()
{
	static const int	buff_size = 1024;
	char				buff[buff_size];

	ssize_t	bytes_read = recv(fd, buff, buff_size - 1, 0);
	if (bytes_read == -1) {
		std::perror("recv");
		return -1;
	}
	buff[bytes_read] = '\0';
	stock.append(buff, bytes_read);

	std::cout << fd << ": " << buff;
	if (bytes_read >= 1 && buff[bytes_read - 1] != '\n')
		std::cout << "$\n";

	std::size_t		end_msg;
	// while ((end_msg = stock.find("\n\r")) != std::string::npos) {
	while ((end_msg = stock.find("\n")) != std::string::npos) {
		std::string		cmd(stock, 0, end_msg);
		exec_cmd(cmd);
		stock.erase(0, end_msg);
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
