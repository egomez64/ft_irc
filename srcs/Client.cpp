#include "../includes/Client.hpp"

int Client::GetFd(){
    return client_fd;
}

void Client::SetFd(int fd){
    client_fd = fd;
}

void Client::setIpAdd(std::string ipadd){
    ip_addr = ipadd;
}