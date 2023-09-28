#include "qian/util.h"

namespace qian {
pid_t GetThreadId()
{
    return syscall(SYS_gettid);
}
uint32_t GetFiberId()
{
    return 0;
}
}   // namespace qian