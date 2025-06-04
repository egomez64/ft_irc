#include <Channel.hpp>

std::string Channel::users() const
{
	if (clients.empty())
		return "";

	std::map<const std::string, Client &>::const_iterator it = clients.begin();
	std::string		users;
	if (operators.find(it->first) != operators.end())
		users.push_back('@');
	users.append(it->first);
	it++;
	for (; it != clients.end(); it++) {
		users.push_back(' ');
		if (operators.find(it->first) != operators.end())
			users.push_back('@');
		users.append(it->first);
	}
	return users;
}

void Channel::remove_client(const std::string &nickname)
{
	clients.erase(nickname);
	operators.erase(nickname);
}

int Channel::add_client(Client &client)
{
	const std::string	nickname = client.get_nickname();
	if (clients.find(nickname) != clients.end())
		return -1;
	clients.insert(std::pair<const std::string, Client &>(nickname, client));
	// msg(":" + nickname + " JOIN " + name + "\r\n");
	msg(RPL_JOIN(nickname, name));
	client.send(RPL_NAMREPLY(nickname, name, users()));
	client.send(RPL_ENDOFNAMES(nickname, name));
	// msg(client, "Say hello to " + client.get_username() + "!\r\n");
	return 0;
}

int Channel::msg(const std::string &msg)
{
	for (std::map<const std::string, Client &>::iterator it = clients.begin(); it != clients.end(); it++) {
		it->second.send(msg);
	}
	return 0;
}

int Channel::msg(const Client &client, const std::string &msg)
{
	for (std::map<const std::string, Client &>::iterator it = clients.begin(); it != clients.end(); it++) {
		if (&it->second == &client)
			continue;
		it->second.send(msg);
	}
	return 0;
}

bool Channel::is_operator(const std::string &nickname) const
{
	return (std::find(operators.begin(), operators.end(), nickname) != operators.end());
}

int Channel::kick(const Client &_operator, const std::string &target, const std::string &reason)
{
	std::set<std::string>::iterator it = std::find(operators.begin(), operators.end(), _operator.get_nickname());

	if (it == operators.end()) {
		_operator.send(ERR_CHANOPRIVSNEEDED(_operator.get_nickname(), name));
		return -1;
	}
	std::map<const std::string, Client &>::iterator	target_it = clients.find(target);
	if (target_it == clients.end()){
		_operator.send(ERR_USERNOTINCHANNEL(_operator.get_nickname(), target, name));
		return -1;
	}
	msg(RPL_KICK(_operator.get_nickname(), name, target, reason));
	target_it->second.remove_chan(name);
	remove_client(target);
	return 0;
}

int Channel::change_modes(Client &client, const std::string &modestring)
{
	if (modestring.empty()) {
		client.send(RPL_CHANNELMODEIS(client.get_nickname(), name, ""));	// stringify modes
		return 0;
	}
	if (!is_operator(client.get_nickname())) {
		client.send(ERR_CHANOPRIVSNEEDED(client.get_nickname(), name));
		return -1;
	}

	std::vector<std::string>	split_mode_str(split(modestring));
	int		plusminus;

	plusminus = split_mode_str[0][0];
	if (plusminus != '+' && plusminus != '-') {
		client.send(ERR_UNKNOWNMODE(client.get_nickname(), split_mode_str[0][1]));
		return -1;
	}

	long	limit;
	char	*endptr;
	std::map<const std::string, Client &>::iterator it;
	switch (split_mode_str[0][1])
	{
	case MODES_INVITE:
		modes.invite = (plusminus == '+');
		break;

	case MODES_TOPIC:
		modes.topic = (plusminus == '+');
		break;

	case MODES_KEY:
		if ((plusminus == '+') && split_mode_str.size() < 2) {
			client.send(ERR_INVALIDMODEPARAM(client.get_nickname(), name, split_mode_str[0][1], '*',
				"You must specify a parameter for the key mode. Syntax: <key>"));
			return -1;
		}
		modes.key = (plusminus == '+');
		if (plusminus == '+')
			key = split_mode_str[1];
		break;

	case MODES_OPERATOR:
		if (split_mode_str.size() < 2) {
			client.send(ERR_INVALIDMODEPARAM(client.get_nickname(), name, split_mode_str[0][1], '*',
				"You must specify a parameter for the operator mode. Syntax: <operator>"));
			return -1;
		}
		if ((it = clients.find(split_mode_str[1])) == clients.end()) {
			client.send(ERR_NOSUCHNICK(client.get_nickname(), split_mode_str[1]));
			return -1;
		}
		if (plusminus == '+') {
			// check if not already, and notify change
			operators.insert(split_mode_str[1]);
		}
		else {
			// check if not already, and notify change
			operators.erase(split_mode_str[1]);
		}
		break;

	case MODES_LIMIT:
		if (plusminus != '+') {
			modes.limit = -1;
			break;
		}
		if (split_mode_str.size() < 2) {
			client.send(ERR_INVALIDMODEPARAM(client.get_nickname(), name, split_mode_str[0][1], '*',
				"You must specify a parameter for the limit mode. Syntax: <limit>"));
			return -1;
		}
		limit = std::strtol(split_mode_str[1].c_str(), &endptr, 10);
		if (*endptr != '\0') {
			client.send(ERR_INVALIDMODEPARAM(client.get_nickname(), name, split_mode_str[0][1], split_mode_str[1],
				"Invalid limit mode parameter. Syntax: <limit>"));
			return -1;
		}
		modes.limit = limit;
		break;

	case MODES_UNKNOWN:
		// if (!std::isalpha(split_mode_str[0][1]))
		client.send(ERR_UNKNOWNMODE(client.get_nickname(), split_mode_str[0][1]));
		return -1;

	default:
		break;
	}
	return 0;
}
