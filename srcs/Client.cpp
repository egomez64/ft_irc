#include <Server.hpp>
#include <Client.hpp>

int Client::join(std::string &chan_name)
{
	Channel		*chan;

	if ((chan = serv.add_client_to_chan(*this, chan_name)) == NULL) {
		std::cout << "Couldn't add_client\n";
		return -1;
	}
	channels.insert(std::pair<std::string&, Channel&>(chan_name, *chan));
	return 0;
}

int Client::msg_chan(std::string &chan, std::string &msg)
{
	std::map<std::string &, Channel &>::iterator	it;

	if ((it = channels.find(chan)) == channels.end()) {
		std::cerr << "You're not in this channel.\n";
		return -1;
	}
	it->second.msg(*this, msg);
	return 0;
}

int Client::receive()
{
	ssize_t	bytes_read = recv(fd, buff, buff_size, 0);
	if (bytes_read == -1) {
		std::perror("recv");
		return -1;
	}
	buff[bytes_read] = '\0';
	stock.append(buff, bytes_read);

	std::size_t		end_msg;
	while ((end_msg = stock.find("\n\r")) != std::string::npos) {
		std::string		cmd(stock, 0, end_msg);
		parse_cmd(cmd);
		stock.erase(0, end_msg);
	}
	return 0;
}

int Client::send(std::string &str)
{
	// std::string		msg = SSTR("# " << fd << ": received.\n");
	ssize_t	sent = ::send(fd, str.c_str(), str.length(), MSG_NOSIGNAL);
	if (sent == -1) {
		std::perror("send");
		return -1;
	}
	return 0;
}
