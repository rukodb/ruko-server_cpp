#pragma once
#include <string>
#include <mutex>
#include <set>
#include "objects/Object.hpp"
#include "SocketServer.hpp"
#include "SaveScheduler.hpp"
#include "RukoDb.hpp"


class RukoServer {
public:
    RukoServer(const Str &host, int port, const Str &filename = "", int maxDeltaWrites = 10000, std::size_t maxDeltaTime = 10, bool saveJson = false);
    ~RukoServer();

    void run();
    void save();
    void handleClient(Socket &client);
    Bytes readNextMessage(Socket &client);
    Bytes handleGetMessage(const Byte *data, size_t &p);
    Bytes handleSetMessage(const Byte *data, size_t &p);
    Bytes handleDeleteMessage(const Byte *data, size_t &p);
    Bytes handleDeclareMessage(const Byte *data, size_t &p);
    Bytes handleLputMessage(const Byte *data, size_t &p);
    Bytes handleCreateMappingMessage(const Byte *data, size_t &p);
    Bytes handleDeleteMappingMessage(const Byte *data, size_t &p);
    Bytes handleGetMappingsMessage(const Byte *data, size_t &p);

    static void init();


private:
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
