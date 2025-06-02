#include <Channel.hpp>

std::string Channel::users() const
{
	if (clients.empty())
		return "";

	std::map<const int, Client &>::const_iterator it = clients.begin();
	std::string		users(it->second.get_nickname());
	it++;
	for (; it != clients.end(); it++) {
		users.push_back(' ');
		users.append(it->second.get_nickname());
	}
	return users;
}

int Channel::add_client(Client &client)
{
	int		fd = client.get_fd();
	if (clients.find(fd) != clients.end())
		return -1;
	clients.insert(std::pair<const int, Client &>(fd, client));
	client.send(":" + client.get_nickname() + " JOIN " + name + "\r\n");
	client.send(RPL_NAMEREPLY(client.get_nickname(), name, users()));
	client.send(RPL_ENDOFNAMES(client.get_nickname(), name));
	msg(client, "Say hello to " + client.get_username() + "!\r\n");
	return 0;
}

int Channel::msg(const std::string &msg)
{
	for (std::map<const int, Client &>::iterator it = clients.begin(); it != clients.end(); it++) {
		it->second.send(msg);
	}
	return 0;
}

int Channel::msg(const Client &client, const std::string &msg)
{
	for (std::map<const int, Client &>::iterator it = clients.begin(); it != clients.end(); it++) {
		if (&it->second == &client)
			continue;
		it->second.send(msg);
	}
	return 0;
}
