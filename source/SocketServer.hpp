#include <utility>

#pragma once

#include <cstddef>
#include <string>
#include <netinet/in.h>
#include "Socket.hpp"

class SocketServer {
public:
    explicit SocketServer(uint16_t port, in_addr_t address = INADDR_ANY) {
        int opt = 1;
        addrLen = sizeof(addr);
        char buffer[1024] = {0};

        // Create file descriptor
        if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }

        // Forcefully attach socket
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = address;
        addr.sin_port = htons(port);

        // Bind socket
        if (bind(fd, (struct sockaddr *) &addr,
                 sizeof(addr)) < 0) {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }

        if (listen(fd, 3) < 0) {
            perror("listen");
            exit(EXIT_FAILURE);
        }
    }

    SocketServer(const SocketServer &) = delete;

    SocketServer(SocketServer &&other) noexcept {
        swap(*this, other);
    }

    friend void swap(SocketServer &a, SocketServer &b) {
        std::swap(a.addr, b.addr);
        std::swap(a.addrLen, b.addrLen);
        std::swap(a.fd, b.fd);
    }

    ~SocketServer() {
        if (fd >= 0) {
            close(fd);
        }
    }

    Socket acceptClient() {
        int new_socket;
        if ((new_socket = accept(fd, (struct sockaddr *) &addr, (socklen_t *) &addrLen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        return Socket(new_socket);
    }

private:
    struct sockaddr_in addr{};
    int fd{}, addrLen{};
};
