#include <Client.hpp>

Client::cmds Client::parse_register(const std::string &str)
{
	if (str == "CAP")			return CAP;
	else if (str == "PASS")		return PASS;
	else if (str == "NICK")		return NICK;
	else if (str == "USER")		return USER;
	else						return INVALID;
}

Client::cmds Client::parse_cmd(const std::string &str)
{
	if (str == "NICK")			return NICK;
	else if (str == "PING")		return PING;
	else if (str == "JOIN")		return JOIN;
	else if (str == "PART")		return PART;
	else if (str == "PRIVMSG")	return PRIVMSG;
	else if (str == "KICK")		return KICK;
	else if (str == "INVITE")	return INVITE;
	else if (str == "TOPIC")	return TOPIC;
	else if (str == "MODE")		return MODE;
	else						return INVALID;
}

bool Client::test_nickname(std::string &str)
{
	for (std::string::iterator it = str.begin(); it != str.end(); ++it) {
		switch (*it)
		{
		case '[':	break;
		case ']':	break;
		case '{':	break;
		case '}':	break;
		case '\\':	break;
		case '|':	break;

		default:
			if (std::isalnum(*it))
				break;
			else
				return false;
		}
	}
	return true;
}

void Client::set_auth()
{
	auth = pass && !nickname.empty() && !username.empty();
	if (!auth)
		return;

	const std::string	version("1.0");
	const std::string	user_modes("");
	const std::string	c_modes("it");
	const std::string	c_modes_param("kol");
	const std::string	support_tokens(
		"CHANMODES=,k,l,it "
		"CHANTYPES=# "
		"PREFIX=(o)@"
	);

	send(RPL_WELCOME(nickname, username));
	send(RPL_YOURHOST(nickname, serv.get_name(), version));
	send(RPL_CREATED(nickname, serv.get_creation_time()));
	send(RPL_MYINFO(nickname, serv.get_name(), version, user_modes, c_modes, c_modes_param));
	send(RPL_ISUPPORT(nickname, support_tokens));

	send(RPL_MOTDSTART(nickname, "ircserv Message of the Day -"));
	send(RPL_MOTD(nickname, "Welcome to ft_IRC."));
	send(RPL_MOTD(nickname, "We hope you have a good time on our server!"));
	send(RPL_ENDOFMOTD(nickname));
}

int Client::exec_cmd(const std::string &cmd)
{
	std::vector<std::string>	split_cmd = split(cmd);
	if (split_cmd.empty())
		return -1;

	switch (auth ? parse_cmd(split_cmd[0]) : parse_register(split_cmd[0]))
	{
		std::size_t	pos;

	case PASS:
		if (split_cmd.size() != 2) {
			send(ERR_NEEDMOREPARAMS(nickname, cmd));
			return -1;
		}
		pass = serv.test_password(split_cmd[1]);
		if (!pass) {
			send(ERR_PASSWDMISMATCH(nickname));
			return -1;
		}
		break;

	case NICK:
		if (!pass) {
			send(ERR_PASSWDMISMATCH(nickname));
			return -1;
		}
		if (split_cmd.size() != 2) {
			send(ERR_NONICKNAMEGIVEN(nickname, cmd));
			return -1;
		}
		if (!test_nickname(split_cmd[0])) {
			send(ERR_ERRONEUSNICKNAME(nickname, split_cmd[0]));
			return -1;
		}
		if (!serv.test_nickname(split_cmd[1])) {
			send(ERR_NICKNAMEINUSE(split_cmd[1]));
			return -1;
		}
		if (!auth) {
			nickname = split_cmd[1];
			set_auth();
		}
		else {
			send_friends(RPL_NICK(nickname, split_cmd[1]));
			send(RPL_NICK(nickname, split_cmd[1]));
			nickname = split_cmd[1];
		}
		break;

	case USER:
		if (!pass) {
			send(ERR_PASSWDMISMATCH(nickname));
			return -1;
		}
		if (split_cmd.size() < 2 || split_cmd[1].length() < 1) {
			send(ERR_NEEDMOREPARAMS(nickname, cmd));
			return -1;
		}
		username = split_cmd[1];
		set_auth();
		break;

	case PING:
		if (split_cmd.size() < 2) {
			send(ERR_NEEDMOREPARAMS(nickname, cmd));
			return -1;
		}
		send(":" + serv.get_name() + " PONG " + serv.get_name() + " :" + cmd.substr(cmd.find(' ') + 1) + "\r\n");
		break;

	case JOIN:
		if (split_cmd.size() < 2) {
			send(ERR_NEEDMOREPARAMS(nickname, cmd));
			return -1;
		}
		if (split_cmd.size() == 2)
			join(split_cmd[1]);
		else
			join(split_cmd[1], split_cmd[2]);
		break;

	case PART:
		if (split_cmd.size() < 2) {
			send(ERR_NEEDMOREPARAMS(nickname, cmd));
			return -1;
		}
		pos = cmd.find(':');
		if (pos == std::string::npos)
			part(split_cmd[1]);
		else
			part(split_cmd[1], cmd.substr(pos + 1));
		break;

	case PRIVMSG:
		if (split_cmd.size() < 3) {
			if (split_cmd.size() == 1)
				send(ERR_NORECIPIENT(nickname, cmd));
			else
				send(ERR_NOTEXTTOSEND(nickname));
			return -1;
		}
		privmsg(split_cmd[1], PRIVMSG(nickname, split_cmd[1], cmd.substr(cmd.find(':') + 1)));
		break;
	
	case KICK:
		if (split_cmd.size() < 3) {
			send(ERR_NEEDMOREPARAMS(nickname, cmd));
			return -1;
		}
		pos = cmd.find(':');
		if (pos == std::string::npos)
			kick(split_cmd[1], split_cmd[2], "No reason specified");
		else
			kick(split_cmd[1], split_cmd[2], cmd.substr(pos + 1));
		break;

	case INVITE:
		if (split_cmd.size() < 3) {
			send(ERR_NEEDMOREPARAMS(nickname, cmd));
			return -1;
		}
		invite(split_cmd[1], split_cmd[2]);
		break;

	case TOPIC:
		if (split_cmd.size() < 2) {
			send(ERR_NEEDMOREPARAMS(nickname, cmd));
			return -1;
		}
		pos = cmd.find(':');
		if (pos == std::string::npos) {
			see_topic(split_cmd[1]);
			break;
		}
		topic(split_cmd[1], cmd.substr(pos + 1));
		break;
	
	case MODE:
		if (split_cmd.size() < 2) {
			send(ERR_NEEDMOREPARAMS(nickname, cmd));
			return -1;
		}
		if (split_cmd.size() == 2) {
			mode(split_cmd[1], "");
			break;
		}
		mode(split_cmd[1], cmd.substr(cmd.find(' ', cmd.find(' ') + 1) + 1));
		break;

	case INVALID:
		if (!auth) {
			if (!pass)
				send(ERR_PASSWDMISMATCH(nickname));
			else if (parse_cmd(split_cmd[0]) != INVALID)
				send(ERR_NOTREGISTERED());
			else
				send(ERR_UNKNOWNCOMMAND(nickname, cmd));
		}
		else if (auth && parse_register(split_cmd[0]) != INVALID)
			send(ERR_ALREADYREGISTRED(nickname));
		else
			send(ERR_UNKNOWNCOMMAND(nickname, cmd));
		return -1;

	default:
		break;
	}
	return 0;
}

int Client::join(const std::string &chan, const std::string key)
{
	std::vector<std::string> chans_names = split_on_char(chan, ',');
	std::vector<std::string> keys = split_on_char(key, ',');

	for(std::vector<std::string>::iterator it_chan = chans_names.begin(), it_key = keys.begin(); it_chan != chans_names.end(); ++it_chan, ++it_key)
	{
		Channel		*chan_ptr;

		if ((*it_chan)[0] != '#') {
			send(ERR_BADCHANMASK(*it_chan));
			continue;
		}
		if ((chan_ptr = serv.add_client_to_chan(*this, *it_chan, *it_key)) == NULL) {
			PRINT("Couldn't add_client");
			continue;
		}
		channels.insert(std::pair<const std::string, Channel &>(*it_chan, *chan_ptr));
	}
	return 0;
}

int Client::part(const std::string &chan, const std::string reason)
{
	std::vector<std::string> chans_names = split_on_char(chan, ',');

	for(std::vector<std::string>::iterator it_chan = chans_names.begin(); it_chan != chans_names.end(); ++it_chan)
	{
		Channel		*chan_ptr;

		if ((*it_chan)[0] != '#') {
			send(ERR_BADCHANMASK(*it_chan));
			continue;
		}
		chan_ptr = serv.find_channel(*it_chan);
		if (chan_ptr == NULL) {
			send(ERR_NOSUCHCHANNEL(nickname, *it_chan));
			continue;
		}
		chan_ptr->part(*this, reason);
	}
	return 0;
}

int Client::privmsg(const std::string &target, const std::string &msg)
{
	if (target[0] == '#') {
		std::map<const std::string, Channel &>::iterator	it_chan;

		if ((it_chan = channels.find(target)) == channels.end()) {
			send(ERR_CANNOTSENDTOCHAN(nickname, target));
			return -1;
		}
		it_chan->second.msg(*this, msg);
	}
	else {
		if (target == Server::bot_name) {
			std::size_t	pos = msg.find(':');
			if (pos == std::string::npos)
				return 0;
			pos = msg.find(':', pos + 1);
			if (pos == std::string::npos)
				return 0;
			if (msg.substr(pos + 1) == "PING\r\n")
				send(":" + Server::bot_name + " PRIVMSG " + nickname + " :PONG\r\n");
			else
				send(":" + Server::bot_name + " PRIVMSG " + nickname + " :Pas PONG >:-(\r\n");
			return 0;
		}

		Client	*client;
		if ((client = serv.find_client(target)) == NULL) {
			send(ERR_NOSUCHNICK(nickname, target));
			return -1;
		}
		client->send(msg);
	}
	return 0;
}

int Client::kick(const std::string &chan, const std::string &user, const std::string &reason)
{
	std::map<const std::string, Channel &>::iterator	it;

	it = channels.find(chan);
	if (it == channels.end()){
		send(ERR_NOTONCHANNEL(nickname, chan));
		return -1;
	}
	it->second.kick(*this, user, reason);
	return 0;
}

int Client::invite(const std::string &target, const std::string &chan_name)
{
	Channel		*chan;
	Client		*invited;

	if ((chan = serv.find_channel(chan_name)) == NULL) {
		send(ERR_NOSUCHCHANNEL(nickname, chan_name));
		return -1;
	}
	if ((invited = serv.find_client(target)) == NULL) {
		send(ERR_NOSUCHNICK(nickname, target));
		return -1;
	}
	chan->invite(*this, *invited);
	return 0;
}

int Client::see_topic(const std::string &chan)
{
	std::map<const std::string, Channel &>::iterator	it;

	if ((it = channels.find(chan)) == channels.end()) {
		send(ERR_NOSUCHCHANNEL(nickname, chan));
		return -1;
	}
	it->second.see_topic(*this);
	return 0;
}

int Client::topic(const std::string &chan, const std::string &topic)
{
	std::map<const std::string, Channel &>::iterator	it;

	if ((it = channels.find(chan)) == channels.end()) {
		send(ERR_NOSUCHCHANNEL(nickname, chan));
		return -1;
	}
	it->second.change_topic(*this, topic);
	return 0;
}

int Client::mode(const std::string &target, const std::string &str)
{
	if (target[0] != '#') {
		send(ERR_UMODEUNKNOWNFLAG(nickname));
		return -1;
	}

	std::map<const std::string, Channel &>::iterator	it_chan;

	if ((it_chan = channels.find(target)) == channels.end()) {
		send(ERR_NOSUCHCHANNEL(nickname, target));
		return -1;
	}
	return it_chan->second.change_modes(*this, str);
}

int Client::send_friends(const std::string &message)
{
	std::map<const std::string, Client &>	friends;

	for (std::map<const std::string, Channel &>::iterator chan = channels.begin(); chan != channels.end(); ++chan)
		friends.insert(chan->second.getClients().begin(), chan->second.getClients().end());

	friends.erase(nickname);

	for (std::map<const std::string, Client &>::iterator it = friends.begin(); it != friends.end(); ++it)
		it->second.send(message);

	return 0;
}

Client::recv_e Client::receive()
{
	static const int	buff_size = 1024;
	char				buff[buff_size];

	ssize_t	bytes_read = recv(fd, buff, buff_size - 1, 0);
	if (bytes_read == -1) {
		return RECV_ERR;
	}
	else if (bytes_read == 0) {
		send_friends(RPL_QUIT(nickname, "Connection closed"));
		for (autodef(it, channels.begin()); it != channels.end(); ++it)
			it->second.remove_client(*this);
		return RECV_OVER;
	}

	buff[bytes_read] = '\0';			//TODO: remove
	if (std::string(buff) != "\r\n")
		PRINT(nickname << " << " << buff);

	input_stock.append(buff, bytes_read);

	std::size_t		end_msg;
	while ((end_msg = input_stock.find("\r\n")) != std::string::npos) {
		std::string		cmd(input_stock, 0, end_msg);
		exec_cmd(cmd);
		input_stock.erase(0, end_msg + 2);
	}
	return RECV_OK;
}

static int epollmod(int epoll_fd, uint32_t events, int fd)
{
	epoll_event		event_to_add;
	std::memset(&event_to_add, 0, sizeof (event_to_add));
	event_to_add.events = events;
	event_to_add.data.fd = fd;
	
	if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event_to_add) == -1) {
		std::perror("epoll_ctl");
		return -1;
	}
	return 0;
}

int Client::send()
{
	if (output_stock.empty())
		return 0;

	ssize_t	sent = ::send(fd, output_stock.c_str(), output_stock.length(), MSG_NOSIGNAL);
	// ssize_t	sent = ::send(fd, output_stock.c_str(), 5, MSG_NOSIGNAL);
	if (sent == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			epollmod(serv.get_epoll_fd(), EPOLLIN | EPOLLOUT, fd);
			return 0;
		}
		std::perror("send");
		return -1;
	}
	output_stock.erase(0, sent);

	if (!output_stock.empty())
		epollmod(serv.get_epoll_fd(), EPOLLIN | EPOLLOUT, fd);
	else
		epollmod(serv.get_epoll_fd(), EPOLLIN, fd);
	return 0;

	// ssize_t	sent = ::send(fd, str.c_str(), str.length(), MSG_NOSIGNAL);
	// if (sent == -1) {
	// 	std::perror("send");
	// 	return -1;
	// }
	// return 0;
}

int Client::send(const std::string &str)
{
	output_stock += str;
	return send();
}

int Client::remove_chan(const std::string &chan)
{
	if (channels.erase(chan) == 0)
		return -1;
	return 0;
}
