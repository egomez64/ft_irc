#pragma once

#include <iostream>

enum State
{
	UNAUTHENTICATED,
	AUTHENTICATED,
	REGISTERED,
};

class Client
{
	private:
		int client_fd;
		std::string ip_addr;
		std::string hostname;

		std::string username;
		std::string realname;
		std::string nickname;

		State state;

	public:
		//getter
		int getFd() const;
		std::string getHostname() const;

		std::string getUsername() const;
		std::string getRealname() const;
		std::string getNickname() const;

		State getState() const;
		
		//setter
		void setHostname(std::string host);

		void setUsername(std::string usr);
		void setNickname(std::string nick);
		void setRealname(std::string real);

		void setState(State _state);
};