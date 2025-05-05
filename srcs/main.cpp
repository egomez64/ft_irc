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
#include <cstdint>

#define MAX_EVENTS 10

#include "../includes/parsing.hpp"

// static int	make_socket_non_blocking(int fd)
// {
// 	int		flags = fcntl(fd, F_GETFL, 0);
// 	if (flags == -1)
// 		return 1;
// 	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
// 		return 1;
// 	return 0;
// }

int main(int ac, char **av)
{
	if (ac != 3)
	{
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return 1;
	}

	int	port = std::atoi(av[1]);
	if (port <= 0 || port > std::numeric_limits<in_port_t>::max())
	{
		std::cerr << "Invalid port number" << std::endl;
		return 1;
	}

	int		server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
	{
		perror("socket");
		return 1;
	}

	int		yes = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (yes));

	sockaddr_in		server_addr;
	std::memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	if (bind(server_fd, (sockaddr *)&server_addr, sizeof (server_addr)) < 0)
	{
		perror("bind");
		close(server_fd);
		return 1;
	}

	if (listen(server_fd, MAX_EVENTS) < 0)
	{
		perror("listen");
		close(server_fd);
		return 1;
	}

	std::cout << "Server listening on port " << port << '\n';


	// if (make_socket_non_blocking(server_fd) == 1)
	// {
	// 	std::cerr << "make_socket_non_blocking(" << server_fd << ")\n";
	// 	return 1;
	// }

	int		epoll_fd = epoll_create1(0);
	if (epoll_fd == -1)
	{
		std::cerr << "epoll_create1() failed\n";
		return 1;
	}

	epoll_event		event{.events = EPOLLIN, .data.fd = server_fd};
	// event.data.fd = server_fd;
	// event.events = EPOLLIN;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1)
	{
		std::cerr << "epoll_ctl(" << epoll_fd << ", EPOLL_CTL_ADD, " << server_fd << ", &event) failed\n";
		return 1;
	}

	epoll_event	events[MAX_EVENTS];
	while (true)
	{
		// std::cout << "while(true)\n";

		int		n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		if (n == -1)
		{
			perror("epoll_wait");
			break;
		}

		// std::cout << "epoll_wait(): " << n << '\n';
		// // for (int i = 0; i < n; i++)
		for (int i = 0; i < n; ++i)
		{
			// std::cout << i << " :\n";
			if (events[i].data.fd == server_fd)
			{
				sockaddr_in		client_addr;
				socklen_t		client_len = sizeof (client_addr);
				int		client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
				if (client_fd < 0)
				{
					perror("accept");
					close(server_fd);
					return 1;
				}

				// make_socket_non_blocking(client_fd);
				// // std::cout << "client_fd: " << client_fd << '\n';

				epoll_event		client_event{.events = EPOLLIN, .data.fd = client_fd};
				// client_event.data.fd = client_fd;
				// client_event.events = EPOLLOUT;
				epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event);
				// std::cout << "epoll_ctl\n";

				const char *msg = "Hello World!\n";
				ssize_t sent = send(client_fd, msg, strlen(msg), MSG_NOSIGNAL);
				if (sent == -1)
					perror("send");

				std::cout << "Hello\n";
			}
			else
			{
				const char *msg = "I don't want to talk to you.\nGet out.\n";
				ssize_t sent = send(events[i].data.fd, msg, strlen(msg), MSG_NOSIGNAL);
				if (sent == -1)
					perror("send");

				close(events[i].data.fd);
				std::cout << "Unwanted\n";
			}
		}
	}

	close(server_fd);
	return 0;
}
