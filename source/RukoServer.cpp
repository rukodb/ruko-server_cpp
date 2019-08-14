#include "RukoServer.hpp"
#include "SocketServer.hpp"
#include "utils.hpp"
#include "objects/Object.hpp"
#include "backtrace.hpp"
#include "Logger.hpp"

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
#include <functional>
#include <unordered_map>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <algorithm>
#include <commands/Command.hpp>


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
        threads.emplace_back(std::bind([&](Socket &socket) mutable {
            handleClient(socket);
        }, std::move(client)));
    }
}

void RukoServer::runInteractive() {
    while (true) {
        std::string line;
        std::cout << "> ";
        if (!std::getline(std::cin, line)) {
            break;
        }
        try {
            if (line.empty()) {
                continue;
            }
            const char *pos = line.c_str();
            auto command = Command::fromString(pos);
            if (!command) {
                std::cout << "No such command. Type HELP for a list of commands." << std::endl;
                continue;
            }
            if (*pos) {
                std::cout << "Unrecognized extra parameters: " << pos << std::endl;
                continue;
            }
            auto bytes = runCommand(std::move(command));
            if (!bytes.empty()) {
                const Byte *data = bytes.data();
                size_t p = 0;
                auto obj = Object(data, p);
                std::cout << obj.toString() << std::endl;
            }
        } catch (const std::invalid_argument &e) {
            std::cout << "Error parsing command: " << e.what() << std::endl;
        } catch (const std::runtime_error &e) {
            std::cout << "Error executing command: " << e.what() << std::endl;
        }
    }
}

void RukoServer::save() {
    db.save();
}

void RukoServer::shutdown() {
    save();
    saveScheduler.shutdown();
}

void RukoServer::handleClient(Socket &client) {
    while (client.isConnected()) {
        auto bytes = readNextMessage(client);
        if (bytes.empty()) {
            break;
        }
        size_t p = 0;
        auto *data = bytes.data();
        auto replyData = runCommand(Command::fromStream(data, p));
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

void RukoServer::init() {
    struct sigaction exitHandler{};

    exitHandler.sa_handler = signalHandler;
    sigemptyset(&exitHandler.sa_mask);
    exitHandler.sa_flags = 0;

    sigaction(SIGINT, &exitHandler, nullptr);
    sigaction(SIGTERM, &exitHandler, nullptr);
}

Bytes RukoServer::runCommand(Command::Ptr &&command) {
    try {
        if (lg.getLevel() <= Logger::Level::debug) {
            lg.debug(command->toString());
        }
        auto res = command->perform(db);
        if (res.didWrite) {
            saveScheduler.registerWrite();
        }
        return res.output;
    } catch (std::exception &e) {
        std::cerr << "Exception: " << backtraceStr() << std::endl;
    }
    return {};
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
