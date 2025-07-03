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

void Channel::remove_client(std::map<const std::string, Client &>::iterator client)
{
	client->second.remove_chan(name);
	operators.erase(client->first);
	clients.erase(client);

	if (clients.empty())
		server.remove_channel(*this);
}

Channel::Channel(const std::string &name, Client &client, Server &server)
	: name(name)
	, server(server)
{
	std::string		key;

	operators.insert(client.get_nickname());
	join(client, key);
}

int Channel::join(Client &client, const std::string &key)
{
	const std::string	nickname = client.get_nickname();

	if (clients.find(nickname) != clients.end())
		return -1;
	if (invite_list.find(nickname) == invite_list.end()) {
		if (!modes.key.empty() && key != modes.key) {
			client.send(ERR_BADCHANNELKEY(nickname, name));
			return -1;
		}
		if (modes.invite) {
			client.send(ERR_INVITEONLYCHAN(nickname, name));
			return -1;
		}
		if (modes.limit != 0 && clients.size() >= modes.limit) {
			client.send(ERR_CHANNELISFULL(nickname, name));
			return -1;
		}
	}
	else
		invite_list.erase(nickname);
	clients.insert(std::pair<const std::string, Client &>(nickname, client));
	msg(RPL_JOIN(nickname, name));
	if (topic.topic.empty())
		client.send(RPL_NOTOPIC(client.get_nickname(), name));
	else
		client.send(RPL_TOPIC(client.get_nickname(), name, topic.topic));
	if (!topic.nick.empty())
		client.send(RPL_TOPICWHOTIME(client.get_nickname(), name, topic.nick, SSTR(topic.setat)));
	client.send(RPL_NAMREPLY(nickname, name, users_str()));
	client.send(RPL_ENDOFNAMES(nickname, name));
	return 0;
}

int Channel::part(Client &client, const std::string &reason)
{
	std::map<const std::string, Client &>::iterator		it;
	it = clients.find(client.get_nickname());
	if (it == clients.end()) {
		client.send(ERR_NOTONCHANNEL(client.get_nickname(), name));
		return -1;
	}

	if (reason.empty())
		msg(RPL_PART(client.get_nickname(), name));
	else
		msg(RPL_PARTMESSAGE(client.get_nickname(), name, reason));
	remove_client(it);
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

int Channel::kick(const Client &client, const std::string &target, const std::string &reason)
{
	std::map<const std::string, Client &>::iterator	target_it;

	if (std::find(operators.begin(), operators.end(), client.get_nickname()) == operators.end()) {
		client.send(ERR_CHANOPRIVSNEEDED(client.get_nickname(), name));
		return -1;
	}
	target_it = clients.find(target);
	if (target_it == clients.end()){
		client.send(ERR_USERNOTINCHANNEL(client.get_nickname(), target, name));
		return -1;
	}
	msg(RPL_KICK(client.get_nickname(), name, target, reason));
	remove_client(target_it);
	return 0;
}

int Channel::invite(const Client &client, const Client &target)
{
	if (clients.find(client.get_nickname()) == clients.end()) {
		client.send(ERR_NOTONCHANNEL(client.get_nickname(), name));
		return -1;
	}
	if (modes.invite && operators.find(client.get_nickname()) == operators.end()) {
		client.send(ERR_CHANOPRIVSNEEDED(client.get_nickname(), name));
		return -1;
	}
	if (clients.find(target.get_nickname()) != clients.end()) {
		client.send(ERR_USERONCHANNEL(client.get_nickname(), target.get_nickname(), name));
		return -1;
	}
	invite_list.insert(target.get_nickname());
	client.send(RPL_INVITING(client.get_nickname(), target.get_nickname(), name));
	target.send(RPL_INVITERCVR(client.get_nickname(), target.get_nickname(), name));
	return 0;
}

int Channel::see_topic(const Client &client)
{
	if (this->topic.topic.empty())
		client.send(RPL_NOTOPIC(client.get_nickname(), name));
	else
		client.send(RPL_SEETOPIC(client.get_nickname(), name, this->topic.topic));
	if (!this->topic.nick.empty())
		client.send(RPL_TOPICWHOTIME(client.get_nickname(), name, this->topic.nick, SSTR(this->topic.setat)));
	return 0;
}

int Channel::change_topic(const Client &client, const std::string &topic)
{
	if (modes.topic && operators.find(client.get_nickname()) == operators.end()) {
		client.send(ERR_CHANOPRIVSNEEDED(client.get_nickname(), name));
		return -1;
	}
	setTopic(topic, client.get_nickname());
	msg(RPL_TOPIC(client.get_nickname(), name, this->topic.topic));
	return 0;
}

int Channel::change_modes(Client &client, const std::string &str)
{
	if (str.empty()) {
		client.send(RPL_CHANNELMODEIS(client.get_nickname(), name, modes_str()));
		client.send(RPL_CREATIONTIME(client.get_nickname(), name, SSTR(std::time(NULL))));
		return 0;
	}
	if (!is_operator(client.get_nickname())) {
		client.send(ERR_CHANOPRIVSNEEDED(client.get_nickname(), name));
		return -1;
	}

	std::vector<std::string>	split_mode(split(str));
	std::string					&modestring = split_mode[0];
	int							plusminus = '+';

	for (std::size_t i = 0, param = 1; i < modestring.length(); ++i) {
		std::string	mode_change;
		long		limit;
		char		*endptr;
		std::map<const std::string, Client &>::iterator it;
		switch (modestring[i])
		{
		case '+':
			plusminus = '+';
			break;

		case '-':
			plusminus = '-';
			break;

		case MODES_INVITE:
			if (modes.invite == (plusminus == '+'))
				break;

			modes.invite = (plusminus == '+');
			mode_change.push_back(plusminus);
			mode_change.push_back(modestring[i]);
			msg(RPL_MODE(client.get_nickname(), name, mode_change));
			break;

		case MODES_TOPIC:
			if (modes.topic == (plusminus == '+'))
				break;

			modes.topic = (plusminus == '+');
			mode_change.push_back(plusminus);
			mode_change.push_back(modestring[i]);
			msg(RPL_MODE(client.get_nickname(), name, mode_change));
			break;

		case MODES_KEY:
			++param;
			if ((plusminus == '+') == !modes.key.empty())
				break;
			if (param > split_mode.size()) {
				client.send(ERR_INVALIDMODEPARAM(client.get_nickname(), name, modestring[i], '*',
					"You must specify a parameter for the key mode. Syntax: <key>."));
				break;
			}
			if (plusminus == '-' ) {
				if (split_mode[param - 1] != modes.key) {
					client.send(ERR_KEYSET(client.get_nickname(), name));
					break;
				}
				modes.key.clear();
			}
			else
				modes.key = split_mode[param - 1];

			mode_change.push_back(plusminus);
			mode_change.push_back(modestring[i]);
			msg(RPL_MODEARG(client.get_nickname(), name, mode_change, split_mode[param - 1]));
			break;

		case MODES_OPERATOR:
			++param;
			if (param > split_mode.size()) {
				client.send(ERR_INVALIDMODEPARAM(client.get_nickname(), name, modestring[i], '*',
					"You must specify a parameter for the op mode. Syntax: <nick>."));
				break;
			}
			if ((it = clients.find(split_mode[param - 1])) == clients.end()) {
				client.send(ERR_NOSUCHNICK(client.get_nickname(), split_mode[param - 1]));
				break;
			}
			if (plusminus == '+') {
				if (operators.find(split_mode[param - 1]) != operators.end())
					break;
				operators.insert(split_mode[param - 1]);
				msg(RPL_MODEARG(client.get_nickname(), name, "+o", split_mode[param - 1]));
			}
			else {
				if (operators.find(split_mode[param - 1]) == operators.end())
					break;
				operators.erase(split_mode[param - 1]);
				msg(RPL_MODEARG(client.get_nickname(), name, "-o", split_mode[param - 1]));
			}
			break;

		case MODES_LIMIT:
			if (plusminus != '+' && modes.limit == 0)
				break;
			if (plusminus != '+') {
				modes.limit = 0;
				mode_change.push_back(plusminus);
				mode_change.push_back(modestring[i]);
				msg(RPL_MODE(client.get_nickname(), name, mode_change));
				break;
			}
			++param;
			if (param > split_mode.size()) {
				client.send(ERR_INVALIDMODEPARAM(client.get_nickname(), name, modestring[i], '*',
					"You must specify a parameter for the limit mode. Syntax: <limit>."));
				break;
			}
			errno = 0;
			limit = std::strtol(split_mode[param - 1].c_str(), &endptr, 10);
			if (limit <= 0 || errno == ERANGE) {
				limit = 0;
				client.send(ERR_INVALIDMODEPARAM(client.get_nickname(), name, modestring[i], split_mode[param - 1],
					"Invalid limit mode parameter. Syntax: <limit>."));
				break;
			}
			if ((limit == static_cast<long>(modes.limit)) && *endptr == '\0')
				break;

			modes.limit = limit;
			mode_change.push_back(plusminus);
			mode_change.push_back(modestring[i]);
			msg(RPL_MODEARG(client.get_nickname(), name, mode_change, std::string(split_mode[param - 1].c_str(), endptr - split_mode[param - 1].c_str())));
			break;

		case MODES_UNKNOWN:
			client.send(ERR_UNKNOWNMODE(client.get_nickname(), modestring[i]));
			break;

		default:
			break;
		}
	}
	return 0;
}

void Channel::remove_client(Client &client)
{
	clients.erase(client.get_nickname());
	operators.erase(client.get_nickname());

	if (clients.empty())
		server.remove_channel(*this);
}

void Channel::setTopic(const std::string &topic, const std::string &nick)
{
	this->topic.topic = topic;
	this->topic.nick = nick;
	this->topic.setat = std::time(NULL);
}
