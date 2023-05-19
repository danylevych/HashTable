#include <iostream>
#include <string>
#include "HashTable.h"


int main()
{
	HashTable<std::string, int> table =
	{
		std::pair<std::string, int>("world", 1),
		std::pair<std::string, int>("hello", 9)
	};

	HashTable<std::string, int> table2{ table };

	for (auto iter = table.rbegin(), end = table.rend(); iter != end; ++iter)
	{
		std::cout << (*iter).second << std::endl;
	}

	std::cout << table2.Find("world")->first << std::endl;

	return 0;
}