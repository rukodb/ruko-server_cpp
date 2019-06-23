# Ruko

*A dead-simple in-memory database*

Ruko is a dead simple in memory database where json is a first
class citezen.

## Usage

```cpp
#include <Ruko.hpp>

RukoDb db;

void getNotes() {
    return db.get({"notes"}).toString();
}

void addNote(const std::string &title, const std::string &content) {
    db.lput({"notes"}, Object::parse::(R"({"uuid": createUuid(), "title": ")" + title + R"(", "content": ")", + content + R"("})"));
}

void getNote(const std::string &uuid) {
    return db.get({"notes", "uuid:" + uuid});
}

```

## Development

*Warning: Here be dragons. Thou hast been warned.*

To compile, you need a `c++14` compiler and `cmake`:

```bash
mkdir build && cd build
cmake ..
make -j4  # Use number of cores
sudo make install  # Installs ruko-server binary into /usr/local/bin
```
