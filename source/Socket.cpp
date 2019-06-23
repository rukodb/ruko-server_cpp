#include "Socket.hpp"

ssize_t readSocket(int __fd, void *__buf, size_t __nbytes) {
    return read(__fd, __buf, __nbytes);
}
