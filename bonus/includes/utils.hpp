#ifndef UTILS_HPP
# define UTILS_HPP

# include <vector>
# include <string>
# include <cctype>
# include <sstream>

# define SSTR( x ) static_cast< std::ostringstream & >( ( std::ostringstream() << std::dec << x ) ).str()
# define PRINT( ... ) std::cout << __VA_ARGS__ << '\n'

std::vector<std::string> split(std::string s);
std::vector<std::string> split_on_char(std::string s, char sep);

#endif // UTILS_HPP
