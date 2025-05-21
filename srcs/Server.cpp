#include <Server.hpp>

#define SSTR( x ) static_cast< std::ostringstream & >( ( std::ostringstream() << std::dec << x ) ).str()

const int	Server::max_events = 10;

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
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		std::perror("socket");
		// std::strerror(errno);
		return -1;
	}

	int		yes = 1;
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (yes));

	sockaddr_in		server_addr;
	std::memset(&server_addr, 0, sizeof (server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	if (bind(socket_fd, (sockaddr *)&server_addr, sizeof (server_addr)) == -1) {
		std::perror("bind");
		return -1;
	}

	if (listen(socket_fd, max_events) == -1) {
		std::perror("listen");
		return -1;
	}

	// std::cout << "Server listening on port " << port << '\n';

	if (make_socket_non_blocking(socket_fd) == -1) {
		std::perror("listen");
		return -1;
	}

	return socket_fd;
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
		event.data.fd = socket_fd;

		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event) == -1) {
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
	if ((client_fd = accept(socket_fd, (sockaddr*)&client_addr, &client_len)) == -1) {
		std::perror("accept");
		return -1;
	}
	clients.insert(std::pair<const int, Client>(client_fd, Client(client_fd, *this)));

	make_socket_non_blocking(client_fd);

	epoll_event		client_event;
	std::memset(&client_event, 0, sizeof (client_event));
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
	client.receive();
	return 0;
}

Server::Server(in_port_t port, const std::string &password)
	: password(password)
{
	setSocket(port);
	setEpoll();
}

Server::~Server()
{
	close(socket_fd);
	close(epoll_fd);
}

int Server::listenLoop()
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

			if (events[i].data.fd == STDIN_FILENO) {
				std::string		input;
				std::getline(std::cin, input);
				if (input == "quit" || input == "exit") {
					running = false;
					return 0;
				}				
			}
			else if (events[i].data.fd == socket_fd)
				acceptNew();
			else if ((client = clients.find(events[i].data.fd)) != clients.end())
				receive(client->second);
			else
				std::cout << "\tunknown event_fd: " << events[i].data.fd << '\n';
		}
	}
	return 0;
}

Channel *Server::add_client_to_chan(Client &client, const std::string &chan_name)
{
	std::map<const std::string &, Channel>::iterator	it;
	if ((it = channels.find(chan_name)) != channels.end()) {
		if (it->second.add_client(client) == -1)
			return NULL;
		return &it->second;
	}
	else {
		Channel		new_chan(chan_name);
		Channel		&inserted_chan = channels.insert(std::pair<const std::string &, Channel>(new_chan.getName(), new_chan)).first->second;

		inserted_chan.add_client(client);
		return &inserted_chan;
	}
}
