#include <Bot.hpp>

#define SSTR( x ) static_cast< std::ostringstream & >( ( std::ostringstream() << std::dec << x ) ).str()

const int	Bot::max_events = 10;

int Bot::make_socket_non_blocking(int fd)
{
	int		flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		return -1;
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		return -1;
	return 0;
}

int Bot::setSocket(const std::string &ip, in_port_t port)
{
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		std::perror("socket");
		return -1;
	}

	if (make_socket_non_blocking(server_fd) == -1) {
		// std::perror("listen");
		return -1;
	}

	sockaddr_in		server_addr;
	std::memset(&server_addr, 0, sizeof (server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);

	if (inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) == -1) {
		std::perror("inet_pton");
		return -1;
	}

	connect(server_fd, (struct sockaddr *) &server_addr, sizeof (server_addr));

	PRINT("Bot connecting to " << ip << " on port " << port);

	return server_fd;
}

int Bot::setEpoll()
{
	epoll_fd = epoll_create1(0);
	if (epoll_fd == -1) {
		std::perror("epoll_create1");
		return -1;
	}
	{
		epoll_event	event;
		std::memset(&event, 0, sizeof (event));
		event.events = EPOLLIN;
		event.data.fd = server_fd;

		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1) {
			std::perror("epoll_ctl");
			return -1;
		}
	}
	return epoll_fd;
}

int Bot::connect_to_serv(const std::string &password)
{
	std::string		to_send;

	if (!password.empty())
		to_send.append("PASS " + password + "\r\n");
	to_send.append("NICK censorBot\r\n");
	to_send.append("USER Censor_Bot\r\n");
	send(to_send);
	return 0;
}

// int Bot::acceptNew()
// {
// 	sockaddr_in		client_addr;
// 	socklen_t		client_len = sizeof (client_addr);
// 	int				client_fd;
// 	if ((client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len)) == -1) {
// 		std::perror("accept");
// 		return -1;
// 	}
// 	clients.insert(std::make_pair(client_fd, Client(client_fd, *this)));
// 	PRINT("Client " << client_fd << " accepted.");

// 	make_socket_non_blocking(client_fd);

// 	epoll_event		client_event;
// 	std::memset(&client_event, 0, sizeof (client_event));
// 	client_event.events = EPOLLIN;
// 	client_event.data.fd = client_fd;

// 	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event) == -1) {
// 		std::perror("epoll_ctl");
// 		return -1;
// 	}
// 	return 0;
// }

int Bot::send(const std::string &str) const
{
	ssize_t	sent = ::send(server_fd, str.c_str(), str.length(), MSG_NOSIGNAL);
	if (sent == -1) {
		std::perror("send");
		return -1;
	}
	return 0;
}

// int Bot::receive(Client &client)
// {
// 	Client::recv_e	result;
// 	if ((result = client.receive()) == Client::RECV_ERR)
// 		return -1;
// 	else if (result == Client::RECV_OVER) {
// 		remove_client(client);
// 		return 0;
// 	}
// 	else
// 		return 0;
// }

Bot::Bot(const std::string &ip, in_port_t port, const std::string &password)
	: nickname("ircserv")
	, running(true)
{
	signal(SIGINT, signal_handler);
	setSocket(ip, port);
	setEpoll();
	connect_to_serv(password);
	PRINT("Connected to serv !");
	PRINT(";-D");
}

Bot::~Bot()
{
	close(server_fd);
	close(epoll_fd);
	signal(SIGINT, SIG_DFL);
}

int Bot::listenLoop()
{
	epoll_event	events[max_events];
	bool		running = true;

	while (running) {
		int		n_fds;
		if ((n_fds = epoll_wait(epoll_fd, events, max_events, -1)) == -1) {
			std::perror("epoll_wait");
			return -1;
		}

		for (int i = 0; i < n_fds; i++) {
			std::map<int, Client>::iterator	client;

			if (events[i].data.fd == server_fd)
				// PRINT("events[i].data.fd == server_fd: Unexpected.");
				// acceptNew();

			// else if ((client = clients.find(events[i].data.fd)) != clients.end())
			// 	receive(client->second);
			else
				PRINT("\tunknown event_fd: " << events[i].data.fd);
		}
	}
	return 0;
}

// Channel *Bot::add_client_to_chan(Client &client, const std::string &chan_name, const std::string &key)
// {
// 	Channel		*chan;
// 	std::map<const std::string, Channel>::iterator	it;

// 	if ((it = channels.find(chan_name)) != channels.end()) {
// 		chan = &it->second;
// 		if (chan->join(client, key) == -1)
// 			return NULL;
// 		return chan;
// 	}
// 	else {
// 		std::pair<std::map<const std::string, Channel>::iterator, bool>
// 			inserted = channels.insert(std::make_pair(chan_name, Channel(chan_name, client)));

// 		if (!inserted.second)
// 			return NULL;
// 		return &inserted.first->second;
// 	}
// }

// bool Bot::test_nickname(const std::string &nickname) const
// {
// 	for (std::map<const int, Client>::const_iterator it = clients.begin(); it != clients.end(); it++) {
// 		if (it->second.get_nickname() == nickname)
// 			return false;
// 	}
// 	return true;
// }

// const Client *Bot::find_client(const std::string &nickname) const
// {
// 	for (std::map<const int, Client>::const_iterator it = clients.begin(); it != clients.end(); it++) {
// 		if (it->second.get_nickname() == nickname)
// 			return &it->second;
// 	}
// 	return NULL;
// }

// Channel *Bot::find_channel(const std::string &chan)
// {
// 	std::map<const std::string, Channel>::iterator	it;
// 	if ((it = channels.find(chan)) == channels.end()) {
// 		return NULL;
// 	}
// 	return &it->second;
// }

// int Bot::remove_client(const Client &client)
// {
// 	if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client.get_fd(), NULL) == -1) {
// 		std::perror("epoll_ctl");
// 		return -1;
// 	}
// 	close(client.get_fd());
// 	return 0;
// }
