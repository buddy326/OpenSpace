/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014-2025                                                               *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/

#ifndef __OPENSPACE_MODULE_VOLUME___LRUCACHE___H__
#define __OPENSPACE_MODULE_VOLUME___LRUCACHE___H__

#include <ghoul/glm.h>
#include <list>
#include <iterator>

namespace openspace::volume {

template<
    typename KeyType,
    typename ValueType,
    template<typename...> class ContainerType
>
class LruCache {
public:
    explicit LruCache(size_t capacity);

    bool has(const KeyType& key);
    void set(const KeyType& key, ValueType value);
    ValueType& use(const KeyType& key);
    ValueType& get(const KeyType& key);
    void evict();
    size_t capacity() const;

private:
    void insert(const KeyType& key, const ValueType& value);
    ContainerType<
        KeyType, std::pair<ValueType, typename std::list<KeyType>::iterator>
    > _cache;
    std::list<KeyType> _tracker;
    size_t _capacity;
};

} // namespace openspace::volume

#endif // __OPENSPACE_MODULE_VOLUME___LRUCACHE___H__
