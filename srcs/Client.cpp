#include "../includes/Client.hpp"

int Client::GetFd(){
    return client_fd;
}

void Client::SetFd(int fd){
    client_fd = fd;
}

std::string Client::GetIpAdd(){
    return this->ip_addr;
}

void Client::setIpAdd(std::string ipadd){
    ip_addr = ipadd;
}

std::string Client::GetUsername(){
    return this->username;
}

void Client::SetUsername(std::string usr){
    username = usr;
}

std::string Client::GetNick(){
    return this->nickname;
}

void Client::SetNick(std::string nick){
    nickname = nick;
}

std::string Client::GetPass(){
    return this->password;
}

void Client::SetPass(std::string pass){
    password = pass;
}