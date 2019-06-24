#include "Socket.hpp"
constexpr size_t Socket::bufferSize;

ssize_t readSocket(int __fd, void *__buf, size_t __nbytes) {
    return read(__fd, __buf, __nbytes);
}
