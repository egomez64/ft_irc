#include <NumericReplies.hpp>

std::string NumericReplies::format(int num, const std::string &msg)
{
	return SSTR(":ircserv/ NNN <target> [message]\r\n");
}

std::string NumericReplies::reply(int num, const Client &)
{
	switch (num) {
		case ERR_NICKNAMEINUSE:
			return format(num, "");
			break;
	}
}