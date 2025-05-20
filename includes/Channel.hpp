#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <map>
#include <vector>
#include <Client.hpp>

class Channel{
    public:
        struct modes_t{
            bool i;
            bool t;
            bool k;
            int l;

            modes_t() : i(false), t(false), k(false), l(-1) {};
        };

        enum operator_mode {
            GRANT,
            REMOVE,
        };

    private:
        std::string name;
        std::string topic;
        std::string password;

        std::map<int, Client &> clients;
        std::vector<int> operators;

        modes_t modes;

    public:
        bool    can_join(Client &);
        bool    can_join(Client &, std::string &password);
        int     kick(Client& _operator, Client& target);
        int     invite(Client& _operator, Client& target);
        int     change_topic(Client &, std::string &str);
        int     change_modes(Client &, modes_t &);
        int     change_operator(Client& _operator, Client& target, operator_mode);
};

#endif // CHANNEL_HPP