#include <Channel.hpp>

int Channel::add_client(Client &client)
{
	int		fd = client.get_fd();
	if (clients.find(fd) != clients.end())
		return -1;
	clients.insert(std::pair<const int, Client &>(fd, client));
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
