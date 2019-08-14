#pragma once
#include <string>
#include <mutex>
#include <set>
#include "objects/Object.hpp"
#include "SocketServer.hpp"
#include "SaveScheduler.hpp"
#include "RukoDb.hpp"
#include "commands/Command.hpp"


class RukoServer {
public:
    RukoServer(const Str &host, int port, const Str &filename = "", int maxDeltaWrites = 10000, std::size_t maxDeltaTime = 10, bool saveJson = false);
    ~RukoServer();

    void run();
    void runInteractive();
    void save();
    void shutdown();
    void handleClient(Socket &client);
    Bytes readNextMessage(Socket &client);

    static void init();

private:
    Bytes runCommand(Command::Ptr &&command);
    static void signalHandler(int s);
    static in_addr_t parseHost(Str host);

    Str host;
    int port;
    RukoDb db;
    SocketServer socket;
    SaveScheduler saveScheduler;

    static std::set<RukoServer*> openServers;
    static std::mutex openServersMut;
};
