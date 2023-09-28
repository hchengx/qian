#include "qian/log.h"
#include <iostream>

int main()
{
    qian::Logger::ptr logger = QIAN_LOG_ROOT();
    QIAN_LOG_DEBUG(logger) << "Hello qian log!";
    QIAN_LOG_FMT_DEBUG(logger, "hello%s","hello");
    return 0;
}