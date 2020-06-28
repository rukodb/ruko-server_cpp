#pragma once

#include <iostream>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <vector>


class Logger {
public:
    enum class Level : int {
        verbose = 0,
        debug,
        info,
        warning,
        critical,
        temp  // Use for temporary logging during debugging
    };

    explicit Logger(Level level = Level::temp);

    template <typename... Args>
    void verbose(const std::string &fmt, Args &&... args) {
        log(Level::verbose, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void debug(const std::string &fmt, Args &&... args) {
        log(Level::debug, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void info(const std::string &fmt, Args &&... args) {
        log(Level::info, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void warning(const std::string &fmt, Args &&... args) {
        log(Level::warning, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void critical(const std::string &fmt, Args &&... args) {
        log(Level::critical, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void temp(const std::string &fmt, Args &&... args) {
        log(Level::temp, fmt, std::forward<Args>(args)...);
    }

    void setLevel(Level level);
    Level getLevel();
    void setStream(Level level, std::ostream &stream);
    void disable();
    void setAllStreams(std::ostream &stream);

    static Level parseLevel(const std::string &s);

private:
    template <typename... Args>
    void log(Level level, const std::string &fmt, Args &&... args) {
        size_t size = snprintf(nullptr, 0, fmt.c_str(), args...) + 1;
        if (size <= 0) {
            throw std::runtime_error("Formatting error:" + std::string(fmt));
        }
        std::unique_ptr<char[]> buffer(new char[size]);
        snprintf(buffer.get(), size, fmt.c_str(), std::forward<Args>(args)...);
        *streams[int(level)] << buffer.get() << std::endl;
    }

    std::vector<std::ostream *> streams;

    Level level;
};

extern Logger lg;
