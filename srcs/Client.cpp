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
	if (str == "JOIN")			return JOIN;
	else if (str == "PRIVMSG")	return PRIVMSG;
	else if (str == "KICK")		return KICK;
	else if (str == "INVITE")	return INVITE;
	else if (str == "TOPIC")	return TOPIC;
	else if (str == "MODE")		return MODE;
	else						return INVALID;
}

void Client::check_auth()
{
	auth = pass && !nickname.empty() && !username.empty();
	if (!auth)
		return;

	std::cout << username << " \"" << nickname << "\" authentificated.\n";
	const std::string	msg("Welcome to our Server " + username +" \"" + nickname + "\""  +  ".\n\r");
	send(msg);
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
		privmsg(split_cmd[1], PRIVMSG(nickname, split_cmd[1], cmd.substr(pos, cmd.length() - pos)));
		break;
	
	case KICK:
		if (split_cmd.size() < 3) {
			send(ERR_NEEDMOREPARAMS(nickname, cmd));
			return -1;
		}
		kick(split_cmd[1], split_cmd[2]);
		break;

	case TOPIC:
		if (split_cmd.size() < 2){
			send(ERR_NEEDMOREPARAMS(nickname, cmd));
			return -1;
		}
		pos = cmd.find_first_of(':');
		if (pos == std::string::npos) {
			see_topic(split_cmd[1]);
			break;
		}
		++pos;
		topic(split_cmd[1], cmd.substr(pos, cmd.length() - pos));
		break;
	
	case MODE:
		if (split_cmd.size() < 2)
			return -1;
		if (split_cmd.size() == 2) {
			mode(split_cmd[1], "");
			break;
		}
		pos = cmd.find_first_of(' ', cmd.find_first_of(' ') + 1) + 1;
		mode(split_cmd[1], cmd.substr(pos, cmd.length() - pos));
		break;

	default:
		break;
	}
	return 0;
}

int Client::join(const std::string &chan_name, const std::string key)
{
	std::vector<std::string> chans_names = split_on_char(chan_name, ',');
	std::vector<std::string> keys = split_on_char(key, ',');

	for(std::vector<std::string>::iterator it_chan = chans_names.begin(), it_key = keys.begin(); it_chan != chans_names.end(); ++it_chan, ++it_key)
	{
		Channel		*chan;

		if ((*it_chan)[0] != '#') {
			send(ERR_BADCHANMASK(*it_chan));
			continue ;
		}
		if ((chan = serv.add_client_to_chan(*this, *it_chan, *it_key)) == NULL) {
			std::cout << "Couldn't add_client\n";
			continue ;
		}
		channels.insert(std::pair<const std::string, Channel &>(*it_chan, *chan));
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
	std::map<const std::string, Channel &>::iterator it_chan = channels.find(chan);
	if ( it_chan == channels.end()){
		send(ERR_NOTONCHANNEL(nickname, chan));
		return -1;
	}
	it_chan->second.kick(*this, user, reason);
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
		send (ERR_NOSUCHCHANNEL(nickname, target));
		return -1;
	}
	return it_chan->second.change_modes(*this, str);
}

int Client::quit_server(const std::string &message)
{
	std::set<Client &>	friends;

	for (std::map<const std::string, Channel &>::iterator chan = channels.begin(); chan != channels.end(); ++chan) {
		std::pair<std::map<const std::string, Client &>::const_iterator, std::map<const std::string, Client &>::const_iterator> range;
		range = chan->second.getClients();
		friends.insert(range.first, range.second);
	}
	friends.erase(*this);
	for (std::set<Client &>::iterator it = friends.begin(); it != friends.end(); ++it) {
		it->send(RPL_QUIT(nickname, message));
	}
	return 0;
}

Client::recv_e Client::receive()
{
	static const int	buff_size = 1024;
	char				buff[buff_size];

	ssize_t	bytes_read = recv(fd, buff, buff_size - 1, 0);
	if (bytes_read == -1) {
		std::perror("recv");
		return RECV_ERR;
	}
	else if (bytes_read == 0) {
		quit_server("Connection closed");
		return RECV_OVER;
	}

	buff[bytes_read] = '\0';			// TODO! remove
	PRINT("<< " << buff);
	stock.append(buff, bytes_read);

	std::size_t		end_msg;
	while ((end_msg = stock.find("\r\n")) != std::string::npos) {
		std::string		cmd(stock, 0, end_msg);
		exec_cmd(cmd);
		stock.erase(0, end_msg + 2);
	}
	return RECV_OK;
}

int Client::send(const std::string &str) const
{
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
