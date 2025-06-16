#ifndef BOT_HPP
# define BOT_HPP

#include <iostream>
#include <set>
#include <vector>
#include "../includes/utils.hpp"

class Bot{
	private:
		std::string name;
		std::set <std::string> vulg;
		bool active = 1;
	
	public:
		Bot();
		std::string getName() {return name;}
		std::set<std::string> getVulg() {return vulg;}
		std::string censor(std::string &str);
		void setActive(bool b) {active = b;}
};

#endif