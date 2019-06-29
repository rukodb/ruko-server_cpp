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


class Socket {
public:
    explicit Socket(int fd);
    Socket(const Socket &) = delete;
    Socket(Socket &&other) noexcept;
    friend void swap(Socket &a, Socket &b);
    ~Socket();

    Bytes read(int numbytes);
    void send(const Bytes &bytes);
    Bytes readBuffer(size_t numbytes);
    bool isConnected() const;
    int getErrorCode() const;
    int getId() const;

private:
    static constexpr size_t bufferSize = 1024;
    int fd = -1;
    bool connected = true;
    int errorCode = 0;
    char buffer[bufferSize]{};
};
