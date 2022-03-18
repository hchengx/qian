#include "../qian/buffer.h"
#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	qian::Buffer buffer;
	int fd = open("/etc/passwd", O_RDONLY);
	int err{0};
	buffer.readFd(fd, &err);
	std::cout << buffer.retrieveAllAsString() << std::endl;
}