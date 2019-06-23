#pragma once

#include <string>
#include <vector>
#include "Object.hpp"

Str jsonStr(const void *);
Str jsonStr(int x);
Str jsonStr(float x);
Str jsonStr(bool x);
Str jsonStr(const Object::Ptr &o);
Str jsonStr(const Str &str);
Str jsonStr(const Vec<Object::Ptr> &array);
Str jsonStr(const Map<Str, Object::Ptr> &map);
