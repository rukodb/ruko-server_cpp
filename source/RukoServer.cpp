#include "RukoServer.hpp"

#include <utility>
#include <mutex>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <fstream>
#include <unordered_map>
#include "SocketServer.hpp"
#include "utils.hpp"
#include "Object.hpp"
#include "backtrace.hpp"

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <algorithm>

std::set<RukoServer *> RukoServer::openServers;
std::mutex RukoServer::openServersMut;


RukoServer::RukoServer(const Str &host, int port, const Str &filename, int maxDeltaWrites,
                       std::size_t maxDeltaTime, bool saveJson) :
        host(host), port(port), db(filename, saveJson), socket(uint16_t(port), parseHost(host)),
        saveScheduler(maxDeltaWrites, maxDeltaTime, *this) {

    std::lock_guard<std::mutex> lock(openServersMut);
    openServers.emplace(this);
}

RukoServer::~RukoServer() {
    std::lock_guard<std::mutex> lock(openServersMut);
    openServers.erase(openServers.find(this));
}

void RukoServer::run() {
    Vec<std::thread> threads;

    lg.info("Listening on " + host + ":" + std::to_string(port) + "...");
    while (true) {
        Socket client = socket.acceptClient();
        lg.debug("New client " + std::to_string(client.getId()) + ".");
        threads.emplace_back(std::bind([&] (Socket &socket) mutable {
            handleClient(socket);
        }, std::move(client)));
    }
}

void RukoServer::save() {
    db.save();
}

void RukoServer::handleClient(Socket &client) {
    while (client.isConnected()) {
        auto bytes = readNextMessage(client);
        if (bytes.empty()) {
            break;
        }
        size_t p = 0;
        auto *data = bytes.data();
        auto command = deserialize<Byte>(data, p);
        Bytes replyData;
        try {
            switch (command) {
                case 1:
                    replyData = handleGetMessage(data, p);
                    break;
                case 2:
                    replyData = handleSetMessage(data, p);
                    break;
                case 3:
                    replyData = handleDeleteMessage(data, p);
                    break;
                case 4:
                    replyData = handleDeclareMessage(data, p);
                    break;
                case 5:
                    replyData = handleLputMessage(data, p);
                    break;
                case 6:
                    replyData = handleCreateMappingMessage(data, p);
                    break;
                case 7:
                    replyData = handleDeleteMappingMessage(data, p);
                    break;
                case 8:
                    replyData = handleGetMappingsMessage(data, p);
                    break;
                default:
                    std::cerr << "Invalid command: " << int(command) << std::endl;
                    break;
            }
        } catch (std::exception &e) {
            std::cerr << "Exception: " << backtraceStr() << std::endl;
        }
        client.send(withLen(replyData));
    }
    lg.debug("Client " + std::to_string(client.getId()) + " left");
}

Bytes RukoServer::readNextMessage(Socket &client) {
    auto lenData = client.read(4);
    if (lenData.size() != 4) {
        return {};
    }
    Byte *bytes = lenData.data();
    size_t p = 0;
    auto len = bytesToU32(bytes, p);
    auto data = client.read(len);
    if (data.size() != len) {
        return {};
    }
    return data;
}

Bytes RukoServer::handleGetMessage(const Byte *data, size_t &p) {
    auto keys = deserialize<Vec<Str>>(data, p);
    auto fields = deserialize<Vec<Str>>(data, p);
    auto exclude = deserialize<Vec<Str>>(data, p);
    const auto &res = db.get(keys, fields, exclude);
    if (res.isEmpty()) {
        return {};
    }
    return res.toBytes();
}

Bytes RukoServer::handleSetMessage(const Byte *data, size_t &p) {
    auto keys = deserialize<Vec<Str>>(data, p);
    auto obj = deserialize<Object>(data, p);
    if (db.set(keys, obj)) {
        saveScheduler.registerWrite();
    }
    return Bytes({});
}

Bytes RukoServer::handleDeleteMessage(const Byte *data, size_t &p) {
    auto keys = deserialize<Vec<Str>>(data, p);
    db.del(keys);
    return Bytes();
}

Bytes RukoServer::handleDeclareMessage(const Byte *data, size_t &p) {
    auto keys = deserialize<Vec<Str>>(data, p);
    auto dataType = deserialize<Byte>(data, p);
    auto indices = deserialize<Vec<Str>>(data, p);
    db.declare(keys, dataType, indices);
    saveScheduler.registerWrite();
    return {};
}

Bytes RukoServer::handleLputMessage(const Byte *data, size_t &p) {
    auto keys = deserialize<Vec<Str>>(data, p);
    auto value = deserialize<Object>(data, p);
    db.lput(keys, value);
    return {};
}

Bytes RukoServer::handleCreateMappingMessage(const Byte *data, size_t &p) {
    auto location = deserialize<Vec<Str>>(data, p);
    auto filter = deserialize<Vec<Str>>(data, p);
    db.createMapping(location, filter);
    return Bytes({});
}

Bytes RukoServer::handleDeleteMappingMessage(const Byte *data, size_t &p) {
    auto location = deserialize<Vec<Str>>(data, p);
    db.deleteMapping(location);
    return Bytes({});
}

Bytes RukoServer::handleGetMappingsMessage(const Byte *data, size_t &p) {
    return db.getMappings().toBytes();
}

void RukoServer::init() {
    struct sigaction exitHandler{};

    exitHandler.sa_handler = signalHandler;
    sigemptyset(&exitHandler.sa_mask);
    exitHandler.sa_flags = 0;

    sigaction(SIGINT, &exitHandler, nullptr);
    sigaction(SIGTERM, &exitHandler, nullptr);
}

void RukoServer::signalHandler(int s) {
    std::lock_guard<std::mutex> lock(openServersMut);
    lg.info("\nShutting down...");
    for (auto &i : openServers) {
        i->save();
    }
    exit(1);
}

in_addr_t RukoServer::parseHost(Str host) {
    if (host == "localhost") {
        host = "127.0.0.1";
    }
    in_addr addr{};
    int status = inet_aton(host.c_str(), &addr);
    if (status == 0) {
        throw std::invalid_argument("Invalid host ip: " + host);
    }
    return addr.s_addr;
}
