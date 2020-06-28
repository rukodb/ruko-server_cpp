
#include "RukoServer.hpp"
#include "CLI11.hpp"
#include "Logger.hpp"

struct Arguments {
    Arguments() : app("In memory JSON database") {
        app.add_option("server_address", address, "Host and port to run on. Default: localhost:44544");
        app.add_option("-f,--file", filename, "Database file to read and write to (.db)");
        app.add_flag("--in-memory", inMemory, "Run database without file");
        app.add_flag("--as-json", asJson, "Additionally save json version of database");
        app.add_set("--log-level", logLevel, {"debug", "info", "warning", "critical"}, "Verbosity of logger");
        app.add_flag("--interactive", isInteractive, "Read and run db commands from the terminal");
        app.add_option("--log-commands", logCommandsFile, "Log all commands to this file");
    }

    void parse() {
        auto pos = address.find(':');
        if (pos != Str::npos) {
            host = address.substr(0, pos);
            auto portStr = address.substr(pos + 1);
            try {
                port = std::stoi(portStr);
            } catch (const std::exception &e) {
                port = -1;
            }
        }
        if (host.empty() || port < 0) {
            std::cerr << "Could not interpret address: " << address << std::endl;
            exit(1);
        }
        if (inMemory) {
            filename.clear();
        }
    }

    CLI::App app;
    Str address = "localhost:44544", filename = "data.db", logLevel = "info";
    bool inMemory = false, asJson = false, isInteractive = false;
    Str logCommandsFile;

    Str host;
    int port = -1;
};


int main(int argc, char const *argv[]) {
    Arguments args;
    CLI11_PARSE(args.app, argc, argv);
    args.parse();
    
    lg.setLevel(Logger::parseLevel(args.logLevel));

    RukoServer::init();
    RukoServer server(args.host, args.port, args.filename, 10000, 10, args.asJson, args.logCommandsFile);
    if (args.isInteractive) {
        server.runInteractive();
    } else {
        server.run();
    }
    server.shutdown();
    return 0;
}
