#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <signal.h>
#include <iostream>

#include <cstdio>
#include <cstring>

#define MAXEVENTS 10

#include "../includes/parsing.hpp"

static int make_socket_non_blocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) return 1;
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) return 1;
	return 0;
}

int main(int ac, char **av)
{
	if (ac != 3)
	{
		std::cerr << "ac != 3" << '\n';
		return 1;
	}

	signal(SIGPIPE, SIG_IGN); // Prevent crash on send to closed socket

	in_port_t port = parse_port(av[1]);
	if (port == 0)
	{
		std::cerr << "port == 0" << '\n';
		return 1;
	}

	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd == -1)
	{
		std::cerr << "socket() failed\n";
		return 1;
	}

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(listen_fd, (sockaddr *)&addr, sizeof(addr)) == -1)
	{
		std::cerr << "bind() failed\n";
		return 1;
	}
	if (listen(listen_fd, SOMAXCONN) == -1)
	{
		std::cerr << "listen() failed\n";
		return 1;
	}
	if (make_socket_non_blocking(listen_fd))
	{
		std::cerr << "make_socket_non_blocking() failed\n";
		return 1;
	}

	int epoll_fd = epoll_create1(0);
	if (epoll_fd == -1)
	{
		std::cerr << "epoll_create1() failed\n";
		return 1;
	}

	epoll_event event;
	event.data.fd = listen_fd;
	event.events = EPOLLIN;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &event) == -1)
	{
		std::cerr << "epoll_ctl() failed\n";
		return 1;
	}

	epoll_event events[MAXEVENTS];

	while (true)
	{
		int n = epoll_wait(epoll_fd, events, MAXEVENTS, -1);
		if (n == -1)
		{
			perror("epoll_wait");
			break;
		}

		for (int i = 0; i < n; ++i)
		{
			if (events[i].data.fd == listen_fd)
			{
				sockaddr_in client_addr;
				socklen_t client_len = sizeof(client_addr);
				int client_fd = accept(listen_fd, (sockaddr *)&client_addr, &client_len);
				if (client_fd == -1)
				{
					perror("accept");
					continue;
				}

				make_socket_non_blocking(client_fd);
				std::cout << "client_fd: " << client_fd << '\n';

				const char *msg = "Hello World!\n";
				ssize_t sent = send(client_fd, msg, strlen(msg), MSG_NOSIGNAL);
				if (sent == -1)
					perror("send");

				close(client_fd);
			}
			else
			{
				const char *msg = "I don't want to talk to you.\nGet out.\n";
				send(events[i].data.fd, msg, strlen(msg), MSG_NOSIGNAL);
				close(events[i].data.fd);
			}
		}
	}

	close(listen_fd);
	return 0;
}
