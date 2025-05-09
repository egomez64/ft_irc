#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <limits>

#include <vector>
#include <set>

#define MAX_EVENTS 10

#include "../includes/parsing.hpp"

#include <sstream>

#define SSTR( x ) static_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()


static int	make_socket_non_blocking(int fd)
{
	int		flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		return -1;
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		return -1;
	return 0;
}

static int	set_socket(char *port_str, sockaddr_in *server_addr)
{
	int		port = std::atoi(port_str);
	if (port <= 0 || port > std::numeric_limits<in_port_t>::max()) {
		std::cerr << "Invalid port number" << std::endl;
		return -1;
	}

	int		server_fd;
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		return -1;
	}

	int		yes = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (yes));

	std::memset(server_addr, 0, sizeof (*server_addr));
	server_addr->sin_family = AF_INET;
	server_addr->sin_addr.s_addr = INADDR_ANY;
	server_addr->sin_port = htons(port);

	if (bind(server_fd, (sockaddr *)server_addr, sizeof (*server_addr)) == -1) {
		perror("bind");
		close(server_fd);
		return -1;
	}

	if (listen(server_fd, MAX_EVENTS) == -1) {
		perror("listen");
		close(server_fd);
		return -1;
	}

	std::cout << "Server listening on port " << port << '\n';

	if (make_socket_non_blocking(server_fd) == -1) {
		perror("listen");
		close(server_fd);
		return -1;
	}

	return server_fd;
}

static int	set_epoll(epoll_event *event, int server_fd)
{
	int		epoll_fd = epoll_create1(0);
	if (epoll_fd == -1) {
		perror("epoll_create1");
		return -1;
	}

	std::memset(event, 0, sizeof (*event));
	event->events = EPOLLIN;
	event->data.fd = server_fd;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, event) == -1) {
		perror("epoll_ctl");
		// close(epoll_fd);
		return -1;
	}

	// epoll_event		stdin_event;
	// std::memset(&stdin_event, 0, sizeof (stdin_event));
	// stdin_event.events = EPOLLIN;
	// stdin_event.data.fd = STDIN_FILENO;
	// 
	// if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &stdin_event) == -1) {
	// 	perror("epoll_ctl");
	// 	// close(epoll_fd);
	// 	return -1;
	// }

	return epoll_fd;
}


/*
	



*/

int main(int ac, char **av)
{
	if (ac != 3) {
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return 1;
	}

	sockaddr_in		server_addr;
	int				server_fd;
	if ((server_fd = set_socket(av[1], &server_addr)) == -1)
		return 1;

	epoll_event		event;
	int				epoll_fd;
	if ((epoll_fd = set_epoll(&event, server_fd)) == -1) {
		close(server_fd);
		return 1;
	}

	std::set<int>	clients;

	epoll_event	events[MAX_EVENTS];
	bool		running = true;
	while (running) {
		std::cout << "while running:\n";
		int		n_fds;
		if ((n_fds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1)) == -1) {
			perror("epoll_wait");
			break;
		}

		std::cout << "epoll_wait: " << n_fds << '\n';

		for (int i = 0; i < n_fds; i++) {
			// if (events[i].data.fd == STDIN_FILENO) {
			// 	std::string		input;
			// 	std::getline(std::cin, input);
			// 	if (input == "quit" || input == "exit") {
			// 		running = false;
			// 		break;
			// 	}				
			// }
			// 
			// else
			if (events[i].data.fd == server_fd) {

				std::cout << "\tserver_fd\n";
				sockaddr_in		client_addr;
				socklen_t		client_len = sizeof (client_addr);
				int				client_fd;
				if ((client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len)) == -1) {
					perror("accept");
					close(server_fd);
					// close(epoll_fd);
					return 1;
				}
				clients.insert(client_fd);
				std::cout << "\taccept client: " << client_fd << '\n';

				make_socket_non_blocking(client_fd);

				epoll_event		client_event;
				std::memset(&client_event, 0, sizeof (client_event));
				client_event.events = EPOLLIN;
				client_event.data.fd = client_fd;

				if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event) == -1) {
					perror("epoll_ctl");
					close(server_fd);
					// close(epoll_fd);
					return 1;
				}

				// const char *msg = "Hello World!\n";
				// ssize_t sent = send(client_fd, msg, strlen(msg), MSG_NOSIGNAL);
				// if (sent == -1)
				// 	perror("send");
				// 
				// std::cout << "Hello\n";
			}
			else if (clients.find(events[i].data.fd) != clients.end()) {
				// std::vector<char>	buff_recv;
				const std::size_t	size_recv = 1000;
				ssize_t				n_recv;
				// int					i = 0;
				// do {
				// 	buff_recv.insert(buff_recv.end(), size_recv, '\0');
				// 	n_recv = recv(events[i].data.fd, buff_recv.data() + (buff_recv.size() - size_recv), size_recv, 0);
				// 	std::cout << '\t' << i++ << '\n';
				// } while (n_recv > 0);
				char	buff_recv[size_recv + 1];
				n_recv = recv(events[i].data.fd, buff_recv, size_recv, 0);
				if (n_recv == -1) {
					perror("recv");
					close(server_fd);
					// close(epoll_fd);
					return 1;
				}
				buff_recv[n_recv] = '\0';
				std::cout << buff_recv << '\n';

				// std::cout << "Hello " << events[i].data.fd << '\n';
				std::string		msg = SSTR("Hello " << events[i].data.fd << '\n');
				ssize_t sent = send(events[i].data.fd, msg.c_str(), msg.length(), MSG_NOSIGNAL);
				if (sent == -1)
					perror("send");
				
				// SALUT MAXOU !!!
			}
			else {
				std::cout << "\tunknown event_fd: " << events[i].data.fd << '\n';

				// const char	*msg = "I don't want to talk to you.\nGet out.\n";
				// ssize_t sent = send(events[i].data.fd, msg, strlen(msg), MSG_NOSIGNAL);
				// if (sent == -1)
				// 	perror("send");

				// close(events[i].data.fd);
				// std::cout << "Unwanted\n";
			}
		}
	}

	close(server_fd);
	// close(epoll_fd);
	return 0;
}
