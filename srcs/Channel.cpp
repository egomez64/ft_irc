#include <Channel.hpp>

std::string Channel::users_str() const
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

std::string Channel::modes_str() const
{
	std::string		modes_str("+");

	if (modes.invite)
		modes_str.push_back(MODES_INVITE);
	if (modes.topic)
		modes_str.push_back(MODES_TOPIC);
	if (!modes.key.empty())
		modes_str.push_back(MODES_KEY);
	if (modes.limit != 0)
		modes_str.push_back(MODES_LIMIT);
	
	return modes_str;
}

void Channel::remove_client(const std::string &nickname)
{
	clients.erase(nickname);
	operators.erase(nickname);
}

Channel::Channel(const std::string &name, Client &client, Server &serv)
	: name(name)
	, serv(serv)
{
	std::string		key;

	operators.insert(client.get_nickname());
	add_client(client, key);
}

int Channel::add_client(Client &client, const std::string &key)
{
	const std::string	nickname = client.get_nickname();

	if (clients.find(nickname) != clients.end())
		return -1;
	// if (key != modes.key) {
	// 	PRINT("key: " << key);
	// 	PRINT("modes.key: " << modes.key);
	// 	client.send(ERR_BADCHANNELKEY(nickname, name));
	// 	return -1;
	// }
	if (modes.invite) {
		client.send(ERR_INVITEONLYCHAN(nickname, name));
		return -1;
	}
	if (modes.limit != 0 && clients.size() >= modes.limit) {
		client.send(ERR_CHANNELISFULL(nickname, name));
		return -1;
	}
	clients.insert(std::pair<const std::string, Client &>(nickname, client));
	msg(RPL_JOIN(nickname, name));
	client.send(RPL_NAMREPLY(nickname, name, users_str()));
	client.send(RPL_ENDOFNAMES(nickname, name));
	return 0;
}

int Channel::msg(const std::string &msg)
{
	for (std::map<const std::string, Client &>::iterator it = clients.begin(); it != clients.end(); it++)
		it->second.send(msg);
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
		client.send(RPL_CHANNELMODEIS(client.get_nickname(), name, modes_str()));
		client.send(RPL_CREATIONTIME(client.get_nickname(), name, SSTR(std::time(NULL))));
		return 0;
	}
	if (!is_operator(client.get_nickname())) {
		client.send(ERR_CHANOPRIVSNEEDED(client.get_nickname(), name));
		return -1;
	}

	std::vector<std::string>	split_mode_str(split(modestring));
	int		plusminus = '+';

	plusminus = split_mode_str[0][0];
	if (plusminus != '+' && plusminus != '-') {
		client.send(ERR_UNKNOWNMODE(client.get_nickname(), split_mode_str[0][1]));
		return -1;
	}

	std::string	mode_change;
	long		limit;
	char		*endptr;
	std::map<const std::string, Client &>::iterator it;
	switch (split_mode_str[0][1])
	{
	case MODES_INVITE:
		if (modes.invite == (plusminus == '+'))
			break;

		modes.invite = (plusminus == '+');
		mode_change.push_back(plusminus);
		mode_change.push_back(split_mode_str[0][1]);
		msg(RPL_MODE(client.get_nickname(), name, mode_change));
		break;

	case MODES_TOPIC:
		if (modes.topic == (plusminus == '+'))
			break;

		modes.topic = (plusminus == '+');
		mode_change.push_back(plusminus);
		mode_change.push_back(split_mode_str[0][1]);
		msg(RPL_MODE(client.get_nickname(), name, mode_change));
		break;

	case MODES_KEY:
		if ((plusminus == '+') == !modes.key.empty())
			break;
		if (split_mode_str.size() < 2) {
			client.send(ERR_INVALIDMODEPARAM(client.get_nickname(), name, split_mode_str[0][1], '*',
				"You must specify a parameter for the key mode. Syntax: <key>."));
			return -1;
		}
		if (plusminus == '-' ) {
			if (split_mode_str[1] != modes.key) {
				client.send(ERR_KEYSET(name));
				return -1;
			}
			modes.key.clear();
		}
		else
			modes.key = split_mode_str[1];

		modes.key = (plusminus == '+');
		mode_change.push_back(plusminus);
		mode_change.push_back(split_mode_str[0][1]);
		msg(RPL_MODEARG(client.get_nickname(), name, mode_change, split_mode_str[1]));
		break;

	case MODES_OPERATOR:
		if (split_mode_str.size() < 2) {
			client.send(ERR_INVALIDMODEPARAM(client.get_nickname(), name, split_mode_str[0][1], '*',
				"You must specify a parameter for the op mode. Syntax: <nick>."));
			return -1;
		}
		if ((it = clients.find(split_mode_str[1])) == clients.end()) {
			client.send(ERR_NOSUCHNICK(client.get_nickname(), split_mode_str[1]));
			return -1;
		}
		if (plusminus == '+') {
			if (operators.find(split_mode_str[1]) != operators.end())
				break;
			operators.insert(split_mode_str[1]);
			msg(RPL_MODEARG(client.get_nickname(), name, "+o", split_mode_str[1]));
		}
		else {
			if (operators.find(split_mode_str[1]) == operators.end())
				break;
			operators.erase(split_mode_str[1]);
			msg(RPL_MODEARG(client.get_nickname(), name, "-o", split_mode_str[1]));
		}
		break;

	case MODES_LIMIT:
		if (plusminus != '+') {
			modes.limit = -1;
			mode_change.push_back(plusminus);
			mode_change.push_back(split_mode_str[0][1]);
			msg(RPL_MODE(client.get_nickname(), name, mode_change));
			break;
		}
		if (split_mode_str.size() < 2) {
			client.send(ERR_INVALIDMODEPARAM(client.get_nickname(), name, split_mode_str[0][1], '*',
				"You must specify a parameter for the limit mode. Syntax: <limit>."));
			return -1;
		}
		limit = std::strtol(split_mode_str[1].c_str(), &endptr, 10);
		if (limit <= 0) {
			limit = 0;
			client.send(ERR_INVALIDMODEPARAM(client.get_nickname(), name, split_mode_str[0][1], split_mode_str[1],
				"Invalid limit mode parameter. Syntax: <limit>."));
			return -1;
		}
		if ((limit == static_cast<long>(modes.limit)) && *endptr != '\0')
			break;

		modes.limit = limit;
		mode_change.push_back(plusminus);
		mode_change.push_back(split_mode_str[0][1]);
		msg(RPL_MODEARG(client.get_nickname(), name, mode_change, std::string(split_mode_str[1].c_str(), endptr - split_mode_str[1].c_str())));
		break;

	case MODES_UNKNOWN:
		client.send(ERR_UNKNOWNMODE(client.get_nickname(), split_mode_str[0][1]));
		return -1;

	default:
		break;
	}
	return 0;
}
