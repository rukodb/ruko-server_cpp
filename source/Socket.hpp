#pragma once
#include "utils.hpp"
#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <string>
#include <cassert>
#include <vector>
#include <numeric>
#include <iostream>

ssize_t readSocket(int __fd, void *__buf, size_t __nbytes);


class Socket {
public:
    explicit Socket(int fd) : fd(fd) {}

    Socket(const Socket &) = delete;

    Socket(Socket &&other) noexcept {
        swap(*this, other);
    }

    friend void swap(Socket &a, Socket &b) {
        std::swap(a.fd, b.fd);
        std::swap(a.connected, b.connected);
        std::swap(a.errorCode, b.errorCode);
        std::swap(a.buffer, b.buffer);
    }

    ~Socket() {
        if (fd >= 0) {
            close(fd);
        }
    }

    Bytes read(int numbytes) {
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

    void send(const Bytes &bytes) {
        write(fd, bytes.data(), bytes.size());
    }

    Bytes readBuffer(size_t numbytes) {
        eassert(numbytes <= bufferSize);
        ssize_t valread = readSocket(fd, buffer, numbytes);
        if (valread <= 0) {
            connected = false;
            errorCode = int(valread);
            return {};
        }
        return {buffer, buffer + valread};
    }

    bool isConnected() const {
        return connected;
    }

    int getErrorCode() const {
        return errorCode;
    }

    int getId() const {
        return fd;
    }

private:
    static constexpr size_t bufferSize = 1024;
    int fd = -1;
    bool connected = true;
    int errorCode = 0;
    char buffer[bufferSize]{};
};
