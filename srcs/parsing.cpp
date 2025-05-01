#include "../includes/parsing.hpp"

in_port_t parse_port(std::string port)
{
    if (port == "6667")
        return 6667;
    else if (port == "6697")
        return 6697;
    else
        return 0;
}