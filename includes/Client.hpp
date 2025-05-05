#pragma once

#include <iostream>

class Client
{
	private:
		int client_fd;
		std::string ip_addr;
		std::string username;
		std::string nickname;
		std::string password;

	public:
		Client(){};
		int GetFd();
		void SetFd(int fd);
		std::string GetIpAdd();
		void setIpAdd(std::string ipadd);
		std::string GetUsername();
		void SetUsername(std::string usr);
		std::string GetNick();
		void SetNick(std::string nick);
		std::string GetPass();
		void SetPass(std::string pass);
};