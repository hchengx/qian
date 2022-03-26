#include "util.h"
#include <sys/syscall.h>

namespace qian {
    pid_t CurrentThread::tid() {
        return syscall(SYS_gettid);
    }

} // namespace qian