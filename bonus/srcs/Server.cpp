#include <Server.hpp>

#define SSTR( x ) static_cast< std::ostringstream & >( ( std::ostringstream() << std::dec << x ) ).str()

const int			Server::max_events = 10;
const std::string	Server::bot_name("bot");

int Server::make_socket_non_blocking(int fd)
{
	int		flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		return -1;
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		return -1;
	return 0;
}

int Server::setSocket(in_port_t port)
{
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		std::perror("socket");
		return -1;
	}

	int		yes = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (yes)) == -1) {
		std::perror("setsockopt");
		return -1;
	}

	sockaddr_in		server_addr;
	std::memset(&server_addr, 0, sizeof (server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	if (bind(server_fd, (sockaddr *)&server_addr, sizeof (server_addr)) == -1) {
		std::perror("bind");
		return -1;
	}

	if (listen(server_fd, max_events) == -1) {
		std::perror("listen");
		return -1;
	}

	PRINT("Server listening on port " << port);

	if (make_socket_non_blocking(server_fd) == -1) {
		std::perror("fcntl");
		return -1;
	}

	return server_fd;
}

int Server::setEpoll()
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
	{
		epoll_event		stdin_event;
		std::memset(&stdin_event, 0, sizeof (stdin_event));
		stdin_event.events = EPOLLIN;
		stdin_event.data.fd = STDIN_FILENO;
		
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &stdin_event) == -1) {
			std::perror("epoll_ctl");
			return -1;
		}
	}
	return epoll_fd;
}

int Server::acceptNew()
{
	sockaddr_in		client_addr;
	socklen_t		client_len = sizeof (client_addr);
	int				client_fd;
	if ((client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len)) == -1) {
		std::perror("accept");
		return -1;
	}
	clients.insert(std::make_pair(client_fd, Client(client_fd, *this, !password.empty())));
	PRINT("Client " << client_fd << " accepted.");

	if (make_socket_non_blocking(client_fd) == -1) {
		std::perror("fcntl");
		return -1;
	}

	epoll_event		client_event;
	std::memset(&client_event, 0, sizeof (client_event));
	// client_event.events = EPOLLIN | EPOLLOUT;
	client_event.events = EPOLLIN;
	client_event.data.fd = client_fd;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event) == -1) {
		std::perror("epoll_ctl");
		return -1;
	}
	return 0;
}

int Server::receive(Client &client)
{
	Client::recv_e	result;
	if ((result = client.receive()) == Client::RECV_ERR)
		return -1;
	else if (result == Client::RECV_OVER) {
		remove_client(client);
		return 0;
	}
	else
		return 0;
}

Server::Server(in_port_t port, const std::string &password)
	: name("ircserv@localhost")
	, password(password)
	, running(true)
{
	signal(SIGINT, signal_handler);
	setSocket(port);
	setEpoll();
	creation_time = std::time(NULL);
}

Server::~Server()
{
	for (std::map<const int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
		close(it->first);
	close(server_fd);
	close(epoll_fd);
	signal(SIGINT, SIG_DFL);
}

int Server::listenLoop()
{
	epoll_event	events[max_events];
	bool		running = true;

	while (running) {
		int		n_fds;
		if ((n_fds = epoll_wait(epoll_fd, events, max_events, -1)) == -1) {
			if (errno == EINTR)
				break;
			std::perror("epoll_wait");
			return -1;
		}

		for (int i = 0; i < n_fds; i++) {
			std::map<int, Client>::iterator	client;

			if (events[i].data.fd == STDIN_FILENO) {
				std::string		input;
				std::getline(std::cin, input);
				if (input == "quit" || input == "exit")
					running = false;
			}
			else if (events[i].data.fd == server_fd)
				acceptNew();
			else if ((client = clients.find(events[i].data.fd)) != clients.end()) {
				if ((events[i].events & EPOLLOUT) != 0)
					client->second.send("");
				if ((events[i].events & EPOLLIN) != 0)
					receive(client->second);
			}
			else
				PRINT("\tunknown event_fd: " << events[i].data.fd);
		}
	}
	return 0;
}

Channel *Server::add_client_to_chan(Client &client, const std::string &chan_name, const std::string &key)
{
	Channel		*chan;
	std::map<const std::string, Channel>::iterator	it;

	if ((it = channels.find(chan_name)) != channels.end()) {
		chan = &it->second;
		if (chan->join(client, key) == -1)
			return NULL;
		return chan;
	}
	else {
		std::pair<std::map<const std::string, Channel>::iterator, bool>		inserted;

		inserted = channels.insert(std::make_pair(chan_name, Channel(chan_name, client, *this)));
		if (!inserted.second)
			return NULL;

		return &inserted.first->second;
	}
}

bool Server::test_nickname(const std::string &nickname) const
{
	if (nickname == bot_name)
		return false;

	for (std::map<const int, Client>::const_iterator it = clients.begin(); it != clients.end(); it++) {
		if (it->second.get_nickname() == nickname)
			return false;
	}
	return true;
}

Client *Server::find_client(const std::string &nickname)
{
	for (std::map<const int, Client>::iterator it = clients.begin(); it != clients.end(); it++) {
		if (it->second.get_nickname() == nickname)
			return &it->second;
	}
	return NULL;
}

Channel *Server::find_channel(const std::string &chan)
{
	std::map<const std::string, Channel>::iterator	it;
	if ((it = channels.find(chan)) == channels.end()) {
		return NULL;
	}
	return &it->second;
}

int Server::remove_client(const Client &client)
{
	int		err = 0;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client.get_fd(), NULL) == -1) {
		std::perror("epoll_ctl");
		err = -1;
	}
	if (close(client.get_fd()) == -1) {
		std::perror("close");
		err = -1;
	}
	std::map<const int, Client>::iterator	it;

	it = clients.find(client.get_fd());
	if (it == clients.end())
		err = -1;

	clients.erase(it);
	return err;
}

int Server::remove_channel(const Channel &chan)
{
	std::map<const std::string, Channel>::iterator	it;
	it = channels.find(chan.getName());
	if (it == channels.end())
		return -1;

	channels.erase(it);
	return 0;
}
