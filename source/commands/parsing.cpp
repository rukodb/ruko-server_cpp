#include <iostream>
#include "parsing.hpp"

void skipSpaces(const char *&data) {
    while (*data && isspace(*data)) {
        ++data;
    }
}

Str parseWord(const char *&data) {
    const char *startWord = data;
    while (*data && !isspace(*data)) {
        ++data;
    }
    auto word = Str(startWord, data);
    skipSpaces(data);
    return word;
}

bool parseWordTest(const char *&data, const char *word) {
    const char *pos = data;
    while (*pos && *word && isspace(*pos) && tolower(*pos) == tolower(*word)) {
        ++pos;
        ++word;
    }
    if (!*pos && !*word) {
        data = pos;
        skipSpaces(data);
        return true;
    }
    return false;
}

Vec<Str> parseLocation(const char *&data) {
    return split(parseWord(data), '.');
}

Vec<Str> parseList(const char *&data, char sep) {
    return split(parseWord(data), sep);
}

Object parseObject(const char *&data) {
    return Object::parseFromString(data);
}

int parseInt(const char *&data) {
    char *end;
    errno = 0;
    auto num = strtol(data, &end, 10);
    if (errno != 0 || end == data) {
        throw std::invalid_argument(data);
    }
    data = end;
    return int(num);
}
