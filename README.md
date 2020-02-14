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

## Compiling

To compile, you need a `c++14` compiler and `cmake`:

```bash
mkdir build && cd build
cmake ..
make -j4  # Use number of cores
sudo make install  # Installs ruko-server binary into /usr/local/bin
```

## Development

*Warning: [Here be dragons](https://github.com/rukodb/ruko-server/blob/master/source/KeyMapper.cpp#L85). Thou hast been warned.*

The project is layed out roughly as follows:

 - `RukoServer` listens on a websocket for clients
 - It also contains a `RukoDb` object that represents the database
 - Messages get serialized into `Command`s and handled by `RukoDb` methods
 - `RukoDb` has two main objects: a `db: Object` object and an `indices: IndexManager` object
   - `IndexManager` maintains a subtree of the `db` data structure whose leaves/nodes contain precalculated indices for any dictionary/list whose values have been queried by an attribute lookup operation (ie. `GET users.email:abc@d.com` creates an index for `users.*` that tracks the `email` key).
   - `Object` is just a JSON class that represents data and can be serialized/parsed both in binary form (ie. for parsing from commands / disk) or in JSON form (ie. for exporting data or creating from a string for tests)
