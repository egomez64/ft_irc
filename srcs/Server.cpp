#include "../includes/Server.hpp"

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

	if (listen(socket_fd, MAX_EVENTS) == -1) {
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
	std::cout << "\tsocket_fd\n";
	sockaddr_in		client_addr;
	socklen_t		client_len = sizeof (client_addr);
	int				client_fd;
	if ((client_fd = accept(socket_fd, (sockaddr*)&client_addr, &client_len)) == -1) {
		std::perror("accept");
		return -1;
	}
	// // clients.insert(client_fd);
	// Client	new_client(client_fd);
	Client	new_client;
	clients.insert(std::make_pair(client_fd, new_client));
	std::cout << "\taccept client: " << client_fd << '\n';

	make_socket_non_blocking(client_fd);

	epoll_event		client_event;
	std::memset(&client_event, 0, sizeof (client_event));
	client_event.events = EPOLLIN;
	client_event.data.fd = client_fd;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event) == -1) {
		std::perror("epoll_ctl");
		return -1;
	}

	// const char *msg = "Hello World!\n";
	// ssize_t sent = send(client_fd, msg, strlen(msg), MSG_NOSIGNAL);
	// if (sent == -1)
	// 	std::perror("send");
	// 
	// std::cout << "Hello\n";
	return 0;
}

int Server::receive(int fd)
{
// 	// std::vector<char>	buff_recv;
// 	// const std::size_t	size_recv = 10;
// 
// 	std::string			recv_str;
// 	ssize_t				bytes_read;
// 	int					i = 0;
// // 	do {
// // 		// buff_recv.insert(buff_recv.end(), size_recv, '\0');
// // 		// bytes_read = recv(fd, buff_recv.data() + (buff_recv.size() - size_recv), size_recv, 0);
// // 
// 		const size_t	n = 10;
// // 		// char			buf[n];
// 		char			buf[n + 1];
// 
// 		bytes_read = recv(fd, buf, n, 0);
// 		recv_str.append(buf, n);
// 		std::cout << "\trecv " << i++ << ": " << bytes_read << '\n';
// 		buf[n] = '\0';
// 		std::cout << "\t\tbuf: " << buf << '\n';
// 		std::cout << "\t\t" << recv_str << '\n';
// // 	} while (bytes_read > 0);

	// char	buff_recv[size_recv + 1];
	// bytes_read = recv(fd, buff_recv, size_recv, 0);

	ssize_t			bytes_read;
	const size_t	n = 10;
	// char			buf[n];
	char			buf[n + 1];
	bytes_read = recv(fd, buf, n, 0);
	if (bytes_read == -1) {
		std::perror("recv");
		return -1;
	}
	buf[bytes_read] = '\0';
	std::cout << "\t\tbuf: " << buf << '\n';
	std::cout.flush();
	std::cout << "\t\tbytes_read: " << bytes_read << '\n';
	clients[fd].received.append(buf, n);
	clients[fd].rcv_ended = (bytes_read == 0 || buf[bytes_read - 1] == '\0');
	if (clients[fd].rcv_ended)
		std::cout << clients[fd].received << '\n';
	std::cout << "\t\tclient: " << clients[fd].received;
	if (clients[fd].rcv_ended)
		std::cout << "over";
	std::cout << '\n';

	// buff_recv[bytes_read] = '\0';
	// std::cout << buff_recv.data() << '\n';

	// std::cout << "Hello " << fd << '\n';
	std::string		msg = SSTR("# " << fd << ": received.\n");
	ssize_t sent = send(fd, msg.c_str(), msg.length(), MSG_NOSIGNAL);
	if (sent == -1) {
		std::perror("send");
		return -1;
	}

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
		sleep(5);
		std::cout << "while running:\n";
		int		n_fds;
		if ((n_fds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1)) == -1) {
			std::perror("epoll_wait");
			return -1;
		}

		std::cout << "epoll_wait: " << n_fds << '\n';

		for (int i = 0; i < n_fds; i++) {
			if (events[i].data.fd == STDIN_FILENO) {
				std::string		input;
				std::getline(std::cin, input);
				if (input == "quit" || input == "exit") {
					running = false;
					return 0;
				}				
			}
			else if (events[i].data.fd == socket_fd) {
				acceptNew();
			}
			else if (clients.find(events[i].data.fd) != clients.end()) {
				receive(events[i].data.fd);
			}
			else {
				std::cout << "\tunknown event_fd: " << events[i].data.fd << '\n';

				// const char	*msg = "I don't want to talk to you.\nGet out.\n";
				// ssize_t sent = send(events[i].data.fd, msg, strlen(msg), MSG_NOSIGNAL);
				// if (sent == -1)
				// 	perror("send");

				// std::cout << "Unwanted\n";
			}
		}
	}

	return 0;
}
