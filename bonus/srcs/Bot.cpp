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

int Bot::setEpoll()
{
	epoll_fd = epoll_create1(0);
	if (epoll_fd == -1) {
		std::perror("epoll_create1");
		return -1;
	}
	if (addEpoll(EPOLLIN, STDIN_FILENO) == -1)
		return -1;

	return epoll_fd;
}

int Bot::addEpoll(uint32_t events, int fd)
{
	epoll_event		event_to_add;
	std::memset(&event_to_add, 0, sizeof (event_to_add));
	event_to_add.events = events;
	event_to_add.data.fd = fd;
	
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event_to_add) == -1) {
		std::perror("epoll_ctl");
		return -1;
	}
	return 0;
}

// int Bot::acceptNew()
// {
// 	sockaddr_in		client_addr;
// 	socklen_t		client_len = sizeof (client_addr);
// 	int				client_fd;
// 	if ((client_fd = accept(fd, (sockaddr*)&client_addr, &client_len)) == -1) {
// 		std::perror("accept");
// 		return -1;
// 	}
// 	// clients.insert(std::make_pair(client_fd, Client(client_fd, *this)));
// 	PRINT("Client " << client_fd << " accepted.");
// 
// 	make_socket_non_blocking(client_fd);
// 
// 	epoll_event		client_event;
// 	std::memset(&client_event, 0, sizeof (client_event));
// 	client_event.events = EPOLLIN;
// 	client_event.data.fd = client_fd;
// 
// 	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event) == -1) {
// 		std::perror("epoll_ctl");
// 		return -1;
// 	}
// 	return 0;
// }

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

Bot::Bot(bool &running)
	: running(running)
{
	signal(SIGINT, signal_handler);

	if (running && setEpoll() == -1)
		running = false;
}

Bot::Bot(bool &running, const std::string &ip, in_port_t port, const std::string &password)
	: running(running)
{
	signal(SIGINT, signal_handler);

	if (!running)
		return;

	if (setEpoll() == -1) {
		running = false;
		return;
	}

	if (addServer(ip, port, password) == -1) {
		running = false;
		return;
	}

	PRINT("Connected to serv !");
	PRINT(";-D");
}

Bot::~Bot()
{
	close(epoll_fd);
	for (std::map<int, BotServer *>::iterator it = servers.begin(); it != servers.end(); ++it)
		delete it->second;
	signal(SIGINT, SIG_DFL);
}

int Bot::listenLoop()
{
	epoll_event	events[max_events];

	while (running) {
		int		n_fds;
		if ((n_fds = epoll_wait(epoll_fd, events, max_events, -1)) == -1) {
			if (errno == EINTR)
				continue;
			std::perror("epoll_wait");
			return -1;
		}

		for (int i = 0; i < n_fds; i++) {
			std::map<int, BotServer *>::iterator	server;

			if (events[i].data.fd == STDIN_FILENO) {
				std::string		input;
				std::getline(std::cin, input);
				if (input == "quit" || input == "exit") {
					running = false;
					return 0;
				}
				std::vector<std::string>	params(split(input));

				if (params.size() < 2) {
					std::cout << "Usage: ./censorbot <IPv4> <port> <password>" << std::endl;
					break;
				}
				if (params.size() == 2)
					addServer(params[0], parse_port(params[1].c_str()));
				else
					addServer(params[0], parse_port(params[1].c_str()), params[2]);
			}
			else if ((server = servers.find(events[i].data.fd)) != servers.end()) {
			}
			// else if (events[i].data.fd == fd) {
			// 	PRINT("events[i].data.fd == fd");
			// 	// PRINT("events[i].data.fd == fd: Unexpected.");
			// 	acceptNew();
			// }
			// else if ((client = clients.find(events[i].data.fd)) != clients.end())
			// 	receive(client->second);
			else
				PRINT_ERR("\tunknown event_fd: " << events[i].data.fd);
		}
	}
	return 0;
}

int Bot::addServer(const std::string &ip, in_port_t port, const std::string &password)
{
	BotServer	*serv = new BotServer(ip, port, password);
	if (serv == NULL)
		return -1;

	std::pair<std::map<int, BotServer *>::iterator, bool>	inserted;

	inserted = servers.insert(std::make_pair(serv->get_fd(), serv));
	if (!inserted.second)
		return -1;

	int		fd = inserted.first->first;

	if (addEpoll(EPOLLIN | EPOLLOUT, fd) == -1) {
		delete serv;
		servers.erase(inserted.first);
		return -1;
	}
	return fd;
}

bool Bot::removeServer(int fd)
{
	std::map<int, BotServer *>::iterator it_server;

	it_server = servers.find(fd);
	if (it_server == servers.end())
		return false;
	
	delete it_server->second;
	servers.erase(it_server);
	return true;
}

int Bot::parse_port(const char *port_str)
{
	char	*endptr;
	int		port;

	errno = 0;
	port = std::strtol(port_str, &endptr, 0);
	if (endptr == port_str) {
		std::cerr << "Port: no conversion could be performed (" << port_str << ")" << std::endl;
		return -1;
	}
	if (errno == ERANGE || port <= 0 || port > std::numeric_limits<in_port_t>::max()) {
		std::cerr << "Invalid port number (" << port_str << ")" << std::endl;
		return -1;
	}
	return port;
}

// BotChannel *Bot::add_client_to_chan(Client &client, const std::string &chan_name, const std::string &key)
// {
// 	BotChannel		*chan;
// 	std::map<const std::string, BotChannel>::iterator	it;
// 
// 	if ((it = channels.find(chan_name)) != channels.end()) {
// 		chan = &it->second;
// 		if (chan->join(client, key) == -1)
// 			return NULL;
// 		return chan;
// 	}
// 	else {
// 		std::pair<std::map<const std::string, BotChannel>::iterator, bool>
// 			inserted = channels.insert(std::make_pair(chan_name, BotChannel(chan_name, client)));
// 
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

// BotChannel *Bot::find_channel(const std::string &chan)
// {
// 	std::map<const std::string, BotChannel>::iterator	it;
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
