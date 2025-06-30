#ifndef BOT_CHANNEL_HPP
# define BOT_CHANNEL_HPP

# include <string>

class BotChannel
{
private:
	std::string	name;

public:
	BotChannel(const std::string &name);
	~BotChannel();
};

#endif // BOT_CHANNEL_HPP
