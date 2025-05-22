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