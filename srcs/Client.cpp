#include "../includes/Client.hpp"

int Client::getFd() const{
    return client_fd;
}

std::string Client::getHostname() const{
    return this->hostname;
}

std::string Client::getUsername() const
{
    return this->username;
}

std::string Client::getNickname() const{
    return this->nickname;
}

std::string Client::getRealname() const{
    return this->realname;
}

State Client::getState() const{
    return this->state;
}

void Client::setHostname(std::string host){
    this->hostname = host;
}

void Client::setUsername(std::string usr){
    this->username = usr;
}

void Client::setNickname(std::string nick){
    this->nickname = nick;
}

void Client::setRealname(std::string real){
    this->realname = real;
}

void Client::setState(State _state){
    this->state = _state;
}