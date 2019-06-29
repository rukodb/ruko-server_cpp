#include "Socket.hpp"
constexpr size_t Socket::bufferSize;

static ssize_t readSocket(int __fd, void *__buf, size_t __nbytes) {
    return read(__fd, __buf, __nbytes);
}

Socket::Socket(int fd) : fd(fd) {}

Socket::Socket(Socket &&other) noexcept {
    swap(*this, other);
}

void swap(Socket &a, Socket &b) {
    std::swap(a.fd, b.fd);
    std::swap(a.connected, b.connected);
    std::swap(a.errorCode, b.errorCode);
    std::swap(a.buffer, b.buffer);
}

Socket::~Socket() {
    if (fd >= 0) {
        close(fd);
    }
}

Bytes Socket::read(int numbytes) {
    if (!connected) {
        return {};
    }
    Vec<Bytes> parts;
    for (int i = 0; i < numbytes;) {
        parts.push_back(readBuffer(std::min(bufferSize, size_t(numbytes - i))));
        auto len = parts.back().size();
        if (len == 0) {
            connected = false;
            return concat(parts);
        }
        i += len;
    }
    return concat(parts);
}

void Socket::send(const Bytes &bytes) {
    write(fd, bytes.data(), bytes.size());
}

Bytes Socket::readBuffer(size_t numbytes) {
    eassert(numbytes <= bufferSize);
    ssize_t valread = readSocket(fd, buffer, numbytes);
    if (valread <= 0) {
        connected = false;
        errorCode = int(valread);
        return {};
    }
    return {buffer, buffer + valread};
}

bool Socket::isConnected() const {
    return connected;
}

int Socket::getErrorCode() const {
    return errorCode;
}

int Socket::getId() const {
    return fd;
}
