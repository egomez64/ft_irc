#ifndef BOT_UTILS_HPP
# define BOT_UTILS_HPP

# include <iostream>
# include <sstream>
# include <vector>

# define SSTR( x ) static_cast< std::ostringstream & >( ( std::ostringstream() << std::dec << x ) ).str()
# define PRINT( x ) std::cout << x << '\n'
# define PRINT_ERR( x ) std::cerr << x << '\n'

std::vector<std::string> split(std::string s);

#endif // BOT_UTILS_HPP
