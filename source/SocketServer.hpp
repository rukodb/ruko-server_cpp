#include <utility>

#pragma once

#include <cstddef>
#include <string>
#include <netinet/in.h>
#include "Socket.hpp"

class SocketServer {
public:
    explicit SocketServer(uint16_t port, in_addr_t address = INADDR_ANY);
    SocketServer(const SocketServer &) = delete;
    SocketServer(SocketServer &&other) noexcept;
    friend void swap(SocketServer &a, SocketServer &b);
    ~SocketServer();

    Socket acceptClient();

private:
    struct sockaddr_in addr{};
    int fd{}, addrLen{};
};
