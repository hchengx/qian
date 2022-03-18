#include "logger.h"

namespace qian {
Logger &Logger::getInstance()
{
	static Logger instance;
	return instance;	
}

void Logger::log(std::string message)
{
#define QIAN_XX(level) \
	case LogLevel::level: \
	QIAN_LOG_##level(message.c_str()); \
	break;

	switch (loglevel_){
		QIAN_XX(INFO);
		QIAN_XX(DEBUG);
	}
#undef QIAN_XX
}

} // namespace qian