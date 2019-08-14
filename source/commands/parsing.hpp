#pragma once

#include <IndexManager.hpp>
#include "utils.hpp"


void skipSpaces(const char *&data);
Str parseWord(const char *&data);
bool parseWordTest(const char *&data, const char *word);
Vec<Str> parseLocation(const char *&data);
Vec<Str> parseList(const char *&data, char sep=',');
Object parseObject(const char *&data);
int parseInt(const char *&data);
