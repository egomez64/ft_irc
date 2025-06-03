#include <Channel.hpp>

std::string Channel::users() const
{
	if (clients.empty())
		return "";

	std::map<const std::string, Client &>::const_iterator it = clients.begin();
	std::string		users(it->first);
	it++;
	for (; it != clients.end(); it++) {
		users.push_back(' ');
		users.append(it->first);
	}
	return users;
}

int Channel::add_client(Client &client)
{
	const std::string	nickname = client.get_nickname();
	if (clients.find(nickname) != clients.end())
		return -1;
	clients.insert(std::pair<const std::string, Client &>(nickname, client));
	// msg(":" + nickname + " JOIN " + name + "\r\n");
	msg(RPL_JOIN(nickname, name));
	client.send(RPL_NAMEREPLY(nickname, name, users()));
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

int Channel::kick(const Client &_operator, const std::string &target, const std::string &reason)
{
	// std::vector<int>::iterator it = std::find(operators.begin(), operators.end(), _operator.get_fd());

	// if (it == operators.end()) {
	// 	_operator.send(ERR_CHANOPRIVSNEEDED(_operator.get_nickname(), name));
	// 	return -1;
	// }
	std::map<const std::string, Client &>::iterator	target_it = clients.find(target);
	if (target_it == clients.end()){
		_operator.send(ERR_USERNOTINCHANNEL(_operator.get_nickname(), target, name));
		return -1;
	}
	msg(RPL_KICK(_operator.get_nickname(), name, target, reason));
	target_it->second.remove_chan(name);
	clients.erase(target);
	return 0;
}
