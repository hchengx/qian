#include "qian/log.h"
#include <iostream>

int main()
{
    qian::Logger::ptr logger = QIAN_LOG_ROOT();
    QIAN_LOG_DEBUG(logger) << "Hello qian log!";
    return 0;
}