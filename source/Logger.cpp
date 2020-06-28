#include <stdarg.h>
#include "Logger.hpp"

Logger lg;

Logger::Logger(Logger::Level level) : level(level),
                                      streams((unsigned int) (Level::critical) + 1, nullptr) {
    setStream(Level::debug, std::cout);
    setStream(Level::info, std::cout);
    setStream(Level::warning, std::cerr);
    setStream(Level::critical, std::cerr);
}

void Logger::setLevel(Logger::Level level) {
    this->level = level;
}

Logger::Level Logger::getLevel() {
    return level;
}

void Logger::setStream(Logger::Level level, std::ostream &stream) {
    streams[int(level)] = &stream;
}

void Logger::disable() {
    for (auto &i : streams) {
        i = nullptr;
    }
}

void Logger::setAllStreams(std::ostream &stream) {
    for (Level level = Level::debug; int(level) < (int(Level::critical) + 1);
    level = Level(int(level) + 1)) {
        streams[int(level)] = &stream;
    }
}

Logger::Level Logger::parseLevel(const std::string &s) {
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
