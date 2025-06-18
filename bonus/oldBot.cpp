#include "Bot.hpp"

Bot::Bot()
	: name("Bot")
	, active(true)
{
	try
	{
		std::ifstream	file("vocabulary.txt");
		std::string		word;

		while (std::getline(file, word))
			vulg.insert(word);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
}

std::string Bot::censor(std::string &str)
{
	if (!active)
		return (str);
	std::vector<std::string> words = split(str);
	std::string save;
	std::string result;

	for (std::vector<std::string>::iterator it = words.begin(); it != words.end(); it++)
	{
		*it = to_lower_str(*it);
		if (vulg.find(*it))
		{
			result.push_back((*it).begin());
			for (std::string::iterator c = (*it).begin() + 1; c != (*it).end(); c++)
				result.push_back('*');
			result.push_back(' ');
		}
		else
		{
			for (std::string::iterator c = (*it).begin(); c != (*it).end(); c++)
				result.push_back(*c);
			result.push_back(' ');
		}
	}
	return result;
}