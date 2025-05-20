#include <Server.hpp>
#include <Client.hpp>

int Client::receive()
{
	ssize_t			bytes_read;
	bytes_read = recv(fd, buff, buff_size, 0);
	if (bytes_read == -1) {
		std::perror("recv");
		return -1;
	}
	buff[bytes_read] = '\0';
	stock.append(buff, bytes_read);
	// std::string		msg = SSTR("# " << fd << ": received.\n");
	// ssize_t sent = send(fd, msg.c_str(), msg.length(), MSG_NOSIGNAL);
	// if (sent == -1) {
	// 	std::perror("send");
	// 	return -1;
	// }

	return 0;
}