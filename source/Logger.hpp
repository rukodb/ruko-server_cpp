#pragma once

#include <stdexcept>
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

    explicit Logger(Level level = Level::critical);

    void debug(const std::string &msg);
    void info(const std::string &msg);
    void warning(const std::string &msg);
    void critical(const std::string &msg);

    void setLevel(Level level);
    Level getLevel();
    void setStream(Level level, std::ostream &stream);
    void disable();
    void setAllStreams(std::ostream &stream);

    static Level parseLevel(const std::string &s);

private:
    void log(Level level, const std::string &msg);

    std::vector<std::ostream *> streams;

    Level level;
};

extern Logger lg;
