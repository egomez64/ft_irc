#pragma once

#include <iostream>

class Client
{
private:
	int client_fd;
	std::string ip_addr;
    std::string login;
    std::string name;
    std::string nick_name;
    std::string pass;

public:
	Client(){};
	int GetFd();
	void SetFd(int fd);
	void setIpAdd(std::string ipadd);
};