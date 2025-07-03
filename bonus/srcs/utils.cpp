#include <utils.hpp>

std::vector<std::string> split(std::string s)
{
	std::vector<std::string> tokens;
	std::size_t pos = 0;
	std::string token;
	while ((pos = s.find(' ')) != std::string::npos) {
		token = s.substr(0, pos);
		tokens.push_back(token);
		s.erase(0, pos + 1);
	}
	tokens.push_back(s);

	return tokens;
}

std::vector<std::string> split_on_char(std::string s, char sep)
{
	std::vector<std::string> tokens;
	std::size_t pos = 0;
	while ((pos = s.find(sep)) != std::string::npos) {
		tokens.push_back(s.substr(0, pos));
		s.erase(0, pos + 1);
	}
	tokens.push_back(s);

	return tokens;
}

std::string to_lower_str(std::string &str)
{
	std::string res;
	for (std::string::iterator it = str.begin(); it != str.end(); it++)
		res.push_back(std::tolower(*it));
	return (res);
}
