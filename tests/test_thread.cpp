#include "../qian/thread.h"
#include <iostream>

int main(int argc, char* argv[])
{
    qian::Thread t([]() {
        std::cout << "Hello, world!" << std::endl;
    });
    std::cout << "Thread id: " << t.tid() << std::endl;
    t.start();
    t.join();
    return 0;
}