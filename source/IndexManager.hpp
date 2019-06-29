#include <memory>

#pragma once

#include <cassert>
#include <memory>
#include <algorithm>
#include <numeric>
#include <stack>
#include "utils.hpp"
#include "objects/Object.hpp"
#include "IndexNode.hpp"
#include "StarIndex.hpp"
#include "KeyMapper.hpp"

Vec<Str> split(const Str &s, char delim);

using std::deque;

class IndexManager {
public:
    struct Frame {
        Object *obj;
        IndexNode::Ptr *index;
        Str key;

        Frame traverse(const Str &key, bool createIndex = false, bool createObj = false);
        bool invalid(bool requireIndex = false, bool requireObj = false);
    };

    Vec<Str> convertKeys(const Vec<Str> &keys, Object &obj);
    Vec<Str> convertKeys(const Vec<Str> &keys, Object &&obj);
    bool patternMatches(const Str &pattern, const Str &key);
    Vec<int> findValidPatterns(const Vec<Str> &keys);

    /**
     * Update indices when adding a new value
     * @param keys The location of the new value
     * @param dbRoot The root database object
     */
    void handleCreate(const Vec<Str> &keys, Object &dbRoot);
    void handleCreate(deque<Frame> &frames, Object &dbRoot);

    /**
     * Delete indices associated with the object at the key address for the given root
     * Must be called before deletion
     */
    void handleDelete(const Vec<Str> &keys, Object &dbRoot);
    void handleDelete(deque<Frame> &frames, Object &dbRoot);

    /**
     * Recursively handle the creation of an object
     * @param frame The frame of the object to handle the creation of
     */
    void recurseHandleCreate(deque<Frame> &frames, Vec<int> validPatterns);

    /**
     * Recursively handle the deletion of an object
     * @param frame The frame of the object to handle the deletion of
     */
    void recurseHandleDelete(deque<Frame> &frames, Vec<int> validPatterns);

    /**
     * Extract the keys out of a vector of frames
     * @param frames A vector of the frames in the tree where the first is the root
     * @param ignoreFirst Whether to ignore the first node (commonly the root)
     * @param error Whether to append an empty string to the back to indicate an error
     * @return The keys within the frames
     */
    Vec<Str> extractLocation(const deque<Frame> &frames, bool ignoreFirst, bool error = false);

    /**
     * Convert a key prefixed with an index to a key of the parent
     * If an index doesn't already exist for the indexName, it is created
     * @param key Key with embedded index (ie. "name:john")
     * @param frames Frames previously traversed
     * @param dbRoot Reference to root object used to resolve absolute mappings
     * @return Key that belongs to the parent dictionary
     */
    Str resolveIndexKey(Str key, deque<Frame> &frames, Object &dbRoot);

    /**
     * Traverse to the given location, keeping track of as much as possible
     * @param keys List of strings that represents the key location
     * @param dbRoot Root database object
     * @param createIndices If true, indices are created when they don't exist
     * @param createObj If true, objects are created when they don't exist as dicts and empty and the back
     * @return List of frames that contain data about the tree that leads to the location
     */
    deque<Frame> traverseTo(Vec<Str> keys, Object &dbRoot, bool createIndices = false, bool createObj = false);

    /**
     * Extract the index name in a key encoded like INDEX_NAME:INDEX_VALUE
     * @param key Grouped index and value (ie. "name:john")
     * @return Index part of key (ie. "name") or "" if no such part exists
     */
    static Str extractIndexName(Str &key);

    /**
     * Convert a vector of keys to a dotted single key
     * @param keys Key location
     * @return Single dotted string representing the same location
     */
    static Str toDotted(const Vec<Str> &keys);

    /**
     * Convert a vector of keys from a dotted single key
     * @param key A dotted key
     * @return A vector of keys representing the location encoded in the dotted key
     */
    static Vec<Str> fromDotted(const Str &key);

    /**
     * Update a mapping against the whole database
     * @param mapper The index at the mapping's location
     * @param filter The filter associated with the mapping to use to update
     * @param dbRoot The root database object
     */
    void updateStarThing(KeyMapper &mapper, const Vec<Str> &filter, Object &dbRoot);

    /**
     * Create a new mapping. A mapping is like a virtual node whose values
     * contain anything in the databse that matches a wildcard expression
     * @param rawLocation The raw location that this mapping will be placed at
     * @param filter The wildcard pattern that will define which nodes this location maps to
     * @param dbRoot The root database object
     * @return Whether a new mapping was created
     */
    bool createMapping(const Vec<Str> &rawLocation, const Vec<Str> &filter, Object &dbRoot);

    /**
     * Remote a mapping
     * @param rawLocation The raw location of the mapping
     * @param dbRoot The root database object
     * @return Whether a mapping was deleted
     */
    bool deleteMapping(const Vec<Str> &rawLocation, Object &dbRoot);

    /**
     * Get an object representing all the mappings
     * @return An object representing all mappings
     */
    Object getMappings() const;

    /**
     * Return a representing of the mapping at the current node
     * If no mapping exists there, an empty object is returned
     * @param frames Frames collected from traversing to node
     * @param index The index by which to visualize
     * @param dbRoot Root database object
     * @return An object representing the mapping
     */
    Object getMappingRepresentation(deque<Frame> &frames, const Str &index, Object &dbRoot);

    /**
     * Create an empty index manager
     */
    IndexManager() = default;

    /**
     * Create an index manager from bytes
     */
    IndexManager(const Byte *data, size_t &p);

    /**
     * Serialize this index manager into bytes
     */
    Bytes toBytes();

    /**
     * Check if there were errors in key conversion
     */
    static bool isValid(const Vec<Str> &keys);

private:
    IndexNode::Ptr root{std::make_unique<IndexNode>()};
    Vec<StarIndex> mappings;
};
