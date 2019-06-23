#pragma once

#include <iostream>
#include <ostream>
#include <vector>


class Logger {
public:
    enum class Level : int {
        debug = 0,
        info,
        warning,
        critical
    };

    explicit Logger(Level level = Level::debug) : level(level),
                                                  streams((unsigned int) (Level::critical) + 1, nullptr) {}

    static Level parseLevel(const std::string &s) {
        if (s == "debug") {
            return Level::debug;
        } else if (s == "info") {
            return Level::info;
        } else if (s == "warning") {
            return Level::warning;
        } else if (s == "critical") {
            return Level::critical;
        } else {
            throw std::runtime_error("Invalid level: " + s);
        }
    }

    void debug(const std::string &msg) {
        log(Level::debug, msg);
    }

    void info(const std::string &msg) {
        log(Level::info, msg);
    }

    void warning(const std::string &msg) {
        log(Level::warning, msg);
    }

    void critical(const std::string &msg) {
        log(Level::critical, msg);
    }

    void setLevel(Level level) {
        this->level = level;
    }

    void setStream(Level level, std::ostream &stream) {
        streams[int(level)] = &stream;
    }

    void disable() {
        for (auto &i : streams) {
            i = nullptr;
        }
    }

    void setAllStreams(std::ostream &stream) {
        for (Level level = Level::debug; int(level) < (int(Level::critical) + 1);
        level = Level(int(level) + 1)) {
            streams[int(level)] = &stream;
        }
    }

private:
    void log(Level level, const std::string &msg) {
        if (level >= this->level && streams[int(level)]) {
            *streams[int(level)] << msg << std::endl;
        }
    }

    std::vector<std::ostream *> streams;

    Level level;
};

extern Logger lg;
