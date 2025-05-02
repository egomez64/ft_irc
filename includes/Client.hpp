#pragma once

#include <iostream>

class Client
{
private:
	int client_fd;
	std::string ip_addr;

public:
	Client(){};
	int GetFd();
	void SetFd(int fd);
	void setIpAdd(std::string ipadd);
};