#include "SocketServer.hpp"
#include "Logger.hpp"

SocketServer::SocketServer(uint16_t port, in_addr_t address) {
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
        lg.debug("Problem setting options: %d", strerror(errno));
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

SocketServer::SocketServer(SocketServer &&other) noexcept {
    swap(*this, other);
}

void swap(SocketServer &a, SocketServer &b) {
    std::swap(a.addr, b.addr);
    std::swap(a.addrLen, b.addrLen);
    std::swap(a.fd, b.fd);
}

SocketServer::~SocketServer() {
    if (fd >= 0) {
        close(fd);
    }
}

Socket SocketServer::acceptClient() {
    int new_socket;
    if ((new_socket = accept(fd, (struct sockaddr *) &addr, (socklen_t *) &addrLen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    return Socket(new_socket);
}
