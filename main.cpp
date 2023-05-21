#include <iostream>
#include <string>
#include "HashTable.h"


int main()
{
	HashTable<std::string, int> table =
	{
		std::pair<std::string, int>("world", 1),
		std::pair<std::string, int>("hellp", 9),
		std::pair<std::string, int>("hello", 9)
	};


	for (auto iter = table.begin(), end = table.end(); iter != end; ++iter)
	{
		std::cout << (*iter).second << std::endl;
	}
	auto iterq = ++table.begin();
	
	if (table.Erase(iterq))
	{
		std::cout << iterq->first << std::endl;
		for (auto iter = table.begin(), end = table.end(); iter != end; ++iter)
		{
			std::cout << (*iter).first << std::endl;
		}
	}

	return 0;
}