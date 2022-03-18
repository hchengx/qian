#include "../qian/timestamp.h"
#include <iostream>

int main()
{
	std::cout << "Timestamp: " << qian::Timestamp::now().toString() << std::endl;
	return 0;
}