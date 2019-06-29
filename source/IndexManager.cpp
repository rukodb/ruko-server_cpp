#include <sstream>
#include "IndexManager.hpp"
#include "objects/IndexableData.hpp"
#include "objects/ListData.hpp"
#include "objects/DictData.hpp"

Vec<Str> split(const Str &s, char delim) {
    Vec<Str> result;
    std::stringstream ss(s);
    Str item;

    while (getline(ss, item, delim)) {
        result.push_back(item);
    }

    return result;
}

IndexManager::Frame IndexManager::Frame::traverse(const Str &key, bool createIndex, bool createObj) {
    Frame next{nullptr, nullptr, key};
    if (obj) {
        if (createObj && !obj->has<IndexableData>()) {
            *obj = Object(DICT_ID);  // TODO: Handle updating indices
        }
        if (obj->has<IndexableData>()) {
            auto &data = obj->get<IndexableData>();
            if (data.hasKey(key)) {
                next.obj = &data[key];
            } else if (createObj) {
                next.obj = data.add(key, Object());
            }
        }
    }
    if (index) {
        auto &children = (*index)->children;
        if (children.find(key) != children.end()) {
            next.index = &children.at(key);
        } else if (createIndex) {
            next.index = &children.emplace(key, std::make_unique<IndexNode>()).first->second;
        }
    }
    return next;
}

bool IndexManager::Frame::invalid(bool requireIndex, bool requireObj) {
    return key.empty() || (requireIndex && !index) || (requireObj && !obj);
}

Vec<Str> IndexManager::convertKeys(const Vec<Str> &keys, Object &obj) {
    return extractLocation(traverseTo(keys, obj), true);
}

Vec<Str> IndexManager::convertKeys(const Vec<Str> &keys, Object &&obj) {
    return convertKeys(keys, obj);
}

bool IndexManager::patternMatches(const Str &pattern, const Str &key) {
    return pattern == "*" || pattern == key;
}

Vec<int> IndexManager::findValidPatterns(const Vec<Str> &keys) {
    std::vector<int> validPatterns;
    validPatterns.reserve(mappings.size());
    for (auto i = 0; i < mappings.size(); ++i) {
        if (mappings[i].pattern.size() >= keys.size()) {
            validPatterns.emplace_back(i);
        }
    }
    for (auto i = 0; i < keys.size(); ++i) {
        for (auto it = validPatterns.begin(); it != validPatterns.end();) {
            auto &pat = mappings[*it].pattern;
            if (i < pat.size() && patternMatches(pat[i], keys[i])) {
                ++it;
            } else {
                it = validPatterns.erase(it);
            }
        }
    }
    return validPatterns;
}

void IndexManager::handleCreate(const Vec<Str> &keys, Object &dbRoot) {
    eassert(!keys.empty());
    auto frames = traverseTo(keys, dbRoot);
    if (frames.back().invalid()) {
        throw std::runtime_error("Creating indices for invalid key address");
    }

    handleCreate(frames, dbRoot);
}

void IndexManager::handleCreate(deque<IndexManager::Frame> &frames, Object &dbRoot) {
    if (frames.size() < 2) {  // No parent (assigning root node)
        return;
    }
    auto *node = frames[frames.size() - 2].index;
    if (node && *node) {
        if ((*node)->mapper) {
            (*node)->mapper->registerVal(*frames.back().obj, frames.back().key);
        }
    }
    auto location = extractLocation(deque<Frame>(frames.begin(), frames.end() - 1), true);
    recurseHandleCreate(frames, findValidPatterns(location));
}

void IndexManager::handleDelete(const Vec<Str> &keys, Object &dbRoot) {
    auto frames = traverseTo(keys, dbRoot);
    if (frames.back().invalid()) {
        throw std::runtime_error("Creating indices for invalid key address");
    }
    handleDelete(frames, dbRoot);
}

void IndexManager::handleDelete(deque<IndexManager::Frame> &frames, Object &dbRoot) {
    Vec<Str> location;
    if (frames.size() > 1) {
        location = extractLocation(deque<Frame>(frames.begin(), frames.end() - 1), true);
    }
    if (frames.back().obj && !frames.back().obj->isEmpty()) {
        recurseHandleDelete(frames, findValidPatterns(location));
    }
    if (frames.size() < 2) {  // No parent (deleting root node)
        return;
    }
    auto *node = frames[frames.size() - 2].index;
    if (node && *node) {
        if ((*node)->mapper) {
            eassert(frames.back().obj);
            (*node)->mapper->unregisterVal(*frames.back().obj, frames.back().key);
        }
    }
}

void IndexManager::recurseHandleCreate(deque<IndexManager::Frame> &frames, Vec<int> validPatterns) {
    auto &frame = frames.back();
    int depth = int(frames.size()) - 2;
    eassert(frame.obj);
    if (depth >= 0) {
        for (auto it = validPatterns.begin(); it != validPatterns.end();) {
            auto &pat = mappings[*it].pattern;
            if (depth < pat.size() && patternMatches(pat[depth], frame.key)) {
                if (depth == pat.size() - 1) {
                    eassert(mappings[*it].node->mapper);
                    mappings[*it].node->mapper->registerVal(*frame.obj, toDotted(extractLocation(frames, true)));
                }
                ++it;
            } else {
                it = validPatterns.erase(it);
            }
        }
    }
    if (frame.obj->has<IndexableData>()) {
        for (auto &i : frame.obj->get<IndexableData>().iter()) {
            IndexNode::Ptr *next = nullptr;
            if (frame.index && *frame.index) {
                if ((*frame.index)->mapper) {
                    (*frame.index)->mapper->registerVal(i.value, i.key);
                }
                auto it = (*frame.index)->children.find(i.key);
                if (it != (*frame.index)->children.end()) {
                    next = &it->second;
                }
            }
            frames.emplace_back(Frame{&i.value, next, i.key});
            recurseHandleCreate(frames, validPatterns);
            frames.pop_back();
        }
    }
}

void IndexManager::recurseHandleDelete(deque<IndexManager::Frame> &frames, Vec<int> validPatterns) {
    auto &frame = frames.back();
    int depth = int(frames.size()) - 2;  // frames has root key
    eassert(frame.obj);

    for (auto it = validPatterns.begin(); it != validPatterns.end();) {
        auto &pat = mappings[*it].pattern;
        if (depth < pat.size() && (depth < 0 || patternMatches(pat[depth], frame.key))) {
            if (depth == int(pat.size()) - 1) {
                eassert(mappings[*it].node->mapper);
                mappings[*it].node->mapper->unregisterVal(*frame.obj, toDotted(extractLocation(frames, true)));
            }
            ++it;
        } else {
            it = validPatterns.erase(it);
        }
    }

    if (frame.obj->has<IndexableData>()) {  // TODO: Handle when a subkey is a mapping
        auto data = frame.obj->get<IndexableData>().iter();
        for (auto &i : data) {
            IndexNode::Ptr *next = nullptr;
            if (frame.index && *frame.index) {
                if ((*frame.index)->mapper) {
                    (*frame.index)->mapper->unregisterVal(i.value, i.key);
                }
                auto key2 = i.key;
                IndexNode::Ptr &node = *frame.index;
                auto &children = node->children;

                auto it = children.find(key2);
                if (it != (*frame.index)->children.end()) {
                    next = &it->second;
                }
            }
            frames.emplace_back(Frame{&i.value, next, i.key});
            recurseHandleDelete(frames, validPatterns);
            frames.pop_back();
        }
    }
}

Vec<Str> IndexManager::extractLocation(const deque<IndexManager::Frame> &frames, bool ignoreFirst, bool error) {
    auto it = frames.begin();
    if (ignoreFirst && !frames.empty()) {
        ++it;
    }
    Vec<Str> location;
    location.reserve(frames.size() - ignoreFirst + error);
    for (; it != frames.end(); ++it) {
        location.emplace_back(it->key);
    }
    if (error) {
        location.emplace_back("");
    }
    return location;
}

Str IndexManager::resolveIndexKey(Str key, deque<IndexManager::Frame> &frames, Object &dbRoot) {
    auto indexName = extractIndexName(key);
    // Traverse index to current key
    for (auto last = frames.begin(), it = last + 1; it != frames.end(); ++it) {
        if (!it->index) {
            it->index = &(*last->index)->children.emplace(it->key, std::make_unique<IndexNode>()).first->second;
        }
        last = it;
    }
    auto &index = *frames.back().index;
    auto &mapper = index->getMapper();
    if (!mapper.hasIndex(indexName)) {
        if (!frames.back().obj) {  // Must be a mapping
            bool found = false;
            auto location = extractLocation(frames, true);
            for (auto &mapping : mappings) {  // Could be multiple mappings
                if (mapping.location == location) {
                    mapper.keyIndices.emplace(indexName, Map<Str, Str>());
                    updateStarThing(mapper, mapping.pattern, dbRoot);
                    found = true;
                }
            }
            if (!found) {
                return "";
            }
        } else {
            mapper.createIndex(indexName, *frames.back().obj);
        }
    }
    return mapper.get(indexName, key);
}

deque<IndexManager::Frame> IndexManager::traverseTo(Vec<Str> keys, Object &dbRoot, bool createIndices, bool createObj) {
    deque<Frame> frames;
    frames.emplace_back(Frame{&dbRoot, &root, "[root]"});

    for (auto &key : keys) {
        if (key == "---") {
            if (frames.size() > 1) {
                frames.pop_back();
            }
            continue;
        }
        if (key.find(':') != Str::npos && key.find(':') != key.size() - 1) {  // Index key
            key = resolveIndexKey(key, frames, dbRoot);
            if (key.find('\31') != Str::npos) {  // Absolute key
                frames = traverseTo(fromDotted(key), dbRoot, createIndices, createObj);
                continue;
            }
        }

        frames.emplace_back(frames.back().traverse(key, createIndices, createObj));
        if (frames.back().invalid(createIndices)) {
            return frames;
        }
    }
    return frames;
}

Str IndexManager::extractIndexName(Str &key) {
    auto splitPos = key.find(':');
    Str byIndex;
    if (splitPos != Str::npos) {
        byIndex = key.substr(0, splitPos);
        key = key.substr(splitPos + 1, key.size() - (splitPos + 1));
    }
    return byIndex;
}

Str IndexManager::toDotted(const Vec<Str> &keys) {
    Str dottedLocation;
    for (auto &k : keys) {
        dottedLocation += k + "\31";
    }
    dottedLocation.pop_back();
    return dottedLocation;
}

Vec<Str> IndexManager::fromDotted(const Str &key) {
    return split(key, '\31');
}

void IndexManager::updateStarThing(KeyMapper &mapper, const Vec<Str> &filter, Object &dbRoot) {
    struct Node {
        const Object *object;
        Str key;
    };
    std::vector<Node> nodesToVisit;
    nodesToVisit.emplace_back(Node{&dbRoot, ""});
    for (const auto &pattern : filter) {
        auto nodesCopy = nodesToVisit;
        nodesToVisit.clear();
        for (auto &node : nodesCopy) {
            if (node.object->has<IndexableData>()) {
                for (auto &entry : node.object->get<IndexableData>().iter()) {
                    if (patternMatches(pattern, entry.key)) {
                        nodesToVisit.emplace_back(Node{&entry.value, node.key + entry.key + "\31"});
                    }
                }
            }
        }
    }
    for (auto &node : nodesToVisit) {
        node.key.pop_back();  // Remove trailing period
        mapper.registerVal(*node.object, node.key);
    }
}

bool IndexManager::createMapping(const Vec<Str> &rawLocation, const Vec<Str> &filter, Object &dbRoot) {
    const Object *obj = &dbRoot;
    const IndexNode::Ptr *index = &root;
    auto location = convertKeys(rawLocation, dbRoot);
    for (auto &i : mappings) {
        if (i.location == location && i.pattern == filter) {
            return false;
        }
    }
    if (!isValid(location)) {
        return false;
    }
    for (auto &key : location) {
        index = &(*index)->children.emplace(key, std::make_unique<IndexNode>()).first->second;
    }
    mappings.emplace_back(location, filter, **index);

    KeyMapper::Ptr &mapper = (*index)->mapper;
    if (!mapper) {
        mapper = std::make_unique<KeyMapper>();
        return true;
    }
    updateStarThing(*mapper, filter, dbRoot);  // There could already be another mapping at the same location
    return false;
}

bool IndexManager::deleteMapping(const Vec<Str> &rawLocation, Object &dbRoot) {
    auto location = convertKeys(rawLocation, dbRoot);
    if (!isValid(location)) {
        return false;
    }
    const IndexNode::Ptr *index = &root;
    for (int i = 0; i < location.size() - 1; ++i) {
        if ((*index)->children.find(location[i]) != (*index)->children.end()) {
            index = &(*index)->children.at(location[i]);
        } else {
            return false;
        }
    }
    auto it = (*index)->children.find(location[location.size() - 1]);
    if (it != (*index)->children.end()) {
        (*index)->children.erase(it);

        bool found = false;
        for (auto it2 = mappings.begin(); it2 != mappings.end();) {
            if (it2->location == location) {
                it2 = mappings.erase(it2);
                found = true;  // Could have multiple mappings for the same location
            } else {
                ++it2;
            }
        }
        if (found) {
            return true;
        } else {
            throw std::runtime_error("Deleted index but could not find pattern");
        }
    }
    return false;
}

Object IndexManager::getMappings() const {
    Object obj(LIST_ID);
    auto &list = obj.get<ListData>().list;
    for (auto &mapping : mappings) {
        list.emplace_back(LIST_ID);
        auto &list2 = list.back().get<ListData>().list;
        list2.emplace_back(toDotted(mapping.location));
        list2.emplace_back(toDotted(mapping.pattern));
    }
    return obj;
}

Object IndexManager::getMappingRepresentation(deque<IndexManager::Frame> &frames, const Str &index, Object &dbRoot) {
    auto loc = extractLocation(frames, true);
    Object res = Object(DICT_ID);
    auto &valMap = res.get<DictData>().map;
    bool found = false;
    for (auto &i : mappings) {
        if (i.location == loc) {
            found = true;
            auto &indexO = i.node;
            auto &mapper = indexO->getMapper();
            if (!mapper.hasIndex(index)) {
                mapper.keyIndices.emplace(index, Map<Str, Str>());
                updateStarThing(mapper, i.pattern, dbRoot);
            }
            auto it = mapper.keyIndices.find(index);
            if (it != mapper.keyIndices.end()) {
                Map<Str, Str> &map = it->second;
                for (auto &e : map) {
                    Object v;
                    if (e.second.find('\31') != Str::npos) {
                        v = *traverseTo(fromDotted(e.second), dbRoot).back().obj;
                    } else {
                        v = frames.back().obj->get<IndexableData>()[e.second];
                    }
                    valMap.emplace(e.first, v);
                }
            }
        }
    }
    if (!found) {
        return Object();
    }
    return res;
}

IndexManager::IndexManager(const Byte *data, size_t &p) :
        root(deserialize<IndexNode::Ptr>(data, p)), mappings(deserialize<Vec<StarIndex>>(data, p)) {}

Bytes IndexManager::toBytes() {
    return concat(serialize(root), serialize(mappings));
}

bool IndexManager::isValid(const Vec<Str> &keys) {
    return !keys.empty() && !keys[keys.size() - 1].empty();
}
