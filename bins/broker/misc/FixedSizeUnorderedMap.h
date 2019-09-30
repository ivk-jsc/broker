/*
 * Copyright 2014-present IVK JSC. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FIXED_SIZE_UNORDERD_MAP_H
#define FIXED_SIZE_UNORDERD_MAP_H
#include <vector>
#include <list>
#include <algorithm>
#include "MoveableRWLock.h"
#include <Poco/Hash.h>
#include <Exception.h>

namespace upmq {

template <typename Value>
class FSReadLockedValue {
  MRWLock *_rwLock = nullptr;
  const Value *_value = nullptr;
  std::atomic_bool _wasMoved{false};

  void unlock() noexcept {
    if (_rwLock && _rwLock->isValid()) {
      try {
        if (!_wasMoved) {
          _rwLock->unlockRead();
        }
      } catch (...) {
      }
    }
  }

 public:
  FSReadLockedValue() = default;
  FSReadLockedValue(MRWLock &mrwLock, const Value &value) : _rwLock(&mrwLock), _value(&value) {}
  FSReadLockedValue(const FSReadLockedValue &) = delete;
  FSReadLockedValue(FSReadLockedValue &&o) noexcept : _rwLock(o._rwLock), _value(o._value), _wasMoved(false) { o._wasMoved = true; }
  FSReadLockedValue &operator=(const FSReadLockedValue &) = delete;
  FSReadLockedValue &operator=(FSReadLockedValue &&o) noexcept {
    unlock();
    _rwLock = o._rwLock;
    _value = o._value;
    _wasMoved = false;
    o._wasMoved = true;
    return *this;
  }
  ~FSReadLockedValue() noexcept { unlock(); }
  bool hasValue() const { return _value != nullptr; }
  const Value &operator*() const { return *_value; }
  const Value *operator->() const { return _value; }
  Value &operator*() { return const_cast<Value &>(*_value); }
  Value *operator->() { return const_cast<Value *>(_value); }
};
template <typename Value>
class FSWriteLockedValue {
  MRWLock *_rwLock = nullptr;
  Value *_value = nullptr;
  std::atomic_bool _wasMoved{false};
  void unlock() noexcept {
    if (_rwLock && _rwLock->isValid()) {
      try {
        if (!_wasMoved) {
          _rwLock->unlockWrite();
        }
      } catch (...) {
      }
    }
  }

 public:
  FSWriteLockedValue() = default;
  FSWriteLockedValue(MRWLock &mrwLock, Value &value) : _rwLock(&mrwLock), _value(&value) {}
  FSWriteLockedValue(const FSWriteLockedValue &) = delete;
  FSWriteLockedValue(FSWriteLockedValue &&o) noexcept : _rwLock(o._rwLock), _value(o._value), _wasMoved(false) { o._wasMoved = true; }
  FSWriteLockedValue &operator=(const FSWriteLockedValue &) = delete;
  FSWriteLockedValue &operator=(FSWriteLockedValue &&o) noexcept {
    unlock();
    _rwLock = o._rwLock;
    _value = o._value;
    _wasMoved = false;
    o._wasMoved = true;
    return *this;
  }
  ~FSWriteLockedValue() noexcept { unlock(); }
  const Value &operator*() const { return _value; }
  Value &operator*() { return _value; }
  const Value *operator->() const { return &_value; }
  Value *operator->() { return &_value; }
};
template <typename Key, typename Value>
class FSUnorderedNode {
 public:
  using KVPair = std::pair<Key, Value>;

 private:
  mutable MRWLock _rwLock;
  std::list<KVPair> _items;

 public:
  FSUnorderedNode() = default;
  FSReadLockedValue<Value> find(const Key &key) const {
    _rwLock.readLock();
    auto item = std::find_if(_items.begin(), _items.end(), [&key](const KVPair &pair) { return pair.first == key; });
    if (item != _items.end()) {
      FSReadLockedValue<Value> fs(_rwLock, item->second);
      return fs;
    }
    _rwLock.unlockRead();
    return {};
  }
  bool contains(const Key &key) const {
    ScopedReadRWLock readRWLock(_rwLock);
    auto item = std::find_if(_items.begin(), _items.end(), [&key](const KVPair &pair) { return pair.first == key; });
    return (item != _items.end());
  }
  bool append(const KVPair &pair) {
    _rwLock.writeLock();
    auto item = std::find_if(_items.begin(), _items.end(), [&pair](const KVPair &p) { return p.first == pair.first; });
    if (item != _items.end()) {
      _rwLock.unlockWrite();
      return false;
    }
    try {
      _items.push_back(pair);
    } catch (...) {
      _rwLock.unlockWrite();
      return false;
    }
    _rwLock.unlockWrite();
    return true;
  }
  bool append(KVPair &&pair) {
    _rwLock.writeLock();
    auto item = std::find_if(_items.begin(), _items.end(), [&pair](const KVPair &p) { return p.first == pair.first; });
    if (item != _items.end()) {
      _rwLock.unlockWrite();
      return false;
    }
    try {
      _items.emplace_back(std::move(pair));
    } catch (...) {
      _rwLock.unlockWrite();
      return false;
    }
    _rwLock.unlockWrite();
    return true;
  }
  bool erase(const Key &key) {
    ScopedWriteRWLock writeRWLock(_rwLock);
    auto item = std::find_if(_items.begin(), _items.end(), [&key](const KVPair &p) { return p.first == key; });
    if (item != _items.end()) {
      _items.erase(item);
      return true;
    }
    return false;
  }
  template <typename F>
  size_t eraseIf(const F &f) {
    ScopedWriteRWLock writeRWLock(_rwLock);
    size_t sz = _items.size();
    auto item = std::remove_if(_items.begin(), _items.end(), f);
    if (item != _items.end()) {
      _items.erase(item, _items.end());
      return sz - _items.size();
    }
    return 0;
  }
  void clear() {
    ScopedWriteRWLock writeRWLock(_rwLock);
    _items.clear();
  }
  size_t size() const {
    ScopedReadRWLock readRWLock(_rwLock);
    return _items.size();
  }
  bool empty() const {
    ScopedReadRWLock readRWLock(_rwLock);
    return _items.empty();
  }
  template <typename F>
  void applyForEach(const F &f) const {
    ScopedReadRWLock readRWLock(_rwLock);
    for (const auto &item : _items) {
      f(item);
    }
  }
  template <typename F>
  void changeForEach(const F &f) {
    ScopedReadRWLock readRWLock(_rwLock);
    for (auto &item : _items) {
      f(item);
    }
  }
};

template <typename Key, typename Value>
class FSUnorderedMap {
 public:
  using ItemType = FSUnorderedNode<Key, Value>;
  using ValidItemsValue = size_t;
  using ValidItemsType = std::set<ValidItemsValue>;

 private:
  std::vector<ItemType> _items;
  using ItemsType = std::vector<ItemType>;

  const size_t _size;
  std::atomic<size_t> _realSize{0};
  mutable upmq::MRWLock _validIndexesLock;
  std::set<size_t> _validIndexes;

  void incValidIndex(size_t index) {
    ++_realSize;
    upmq::ScopedWriteRWLock writeRWLock(_validIndexesLock);
    _validIndexes.emplace(index);
  }

  void decValidIndex(size_t index) {
    --_realSize;
    upmq::ScopedWriteRWLock writeRWLock(_validIndexesLock);
    if (_items[index].empty()) {
      _validIndexes.erase(index);
    }
  }

  void clearValidIndex() {
    _realSize = 0;
    upmq::ScopedWriteRWLock writeRWLock(_validIndexesLock);
    _validIndexes.clear();
  }

  void checkSize() const {
    if (_realSize + 1 == _size) {
      throw EXCEPTION("FSUnorderedMap is full", std::to_string(_realSize), ERROR_UNKNOWN);
    }
  }

 public:
  explicit FSUnorderedMap(size_t size) : _items(size), _size(size) {}
  FSUnorderedMap(FSUnorderedMap &&o) noexcept : _items(std::move(o._items)), _size(std::move(o._size)), _realSize(o._realSize.load()) {}
  FSReadLockedValue<Value> find(const Key &key) const {
    size_t index = Poco::hash(key) % _size;
    return _items.at(index).find(key);
  }
  bool contains(const Key &key) const {
    size_t index = Poco::hash(key) % _size;
    return _items.at(index).contains(key);
  }
  void insert(const std::pair<Key, Value> &pair) {
    checkSize();
    size_t index = Poco::hash(pair.first) % _size;
    if (_items.at(index).append(pair)) {
      incValidIndex(index);
    }
  }
  void insert(std::pair<Key, Value> &&pair) {
    checkSize();
    size_t index = Poco::hash(pair.first) % _size;
    if (_items.at(index).append(std::move(pair))) {
      incValidIndex(index);
    }
  }
  void emplace(Key &&key, Value &&value) {
    checkSize();
    size_t index = Poco::hash(key) % _size;
    if (_items.at(index).append(std::pair<Key, Value>(std::move(key), std::move(value)))) {
      incValidIndex(index);
    }
  }
  void erase(const Key &key) {
    size_t index = Poco::hash(key) % _size;
    if (_items.at(index).erase(key)) {
      decValidIndex(index);
    }
  }
  template <typename F>
  void eraseIf(const F &f) {
    upmq::ScopedWriteRWLock writeRWLock(_validIndexesLock);
    for (auto item = _validIndexes.begin(); item != _validIndexes.end();) {
      size_t erased = _items.at(*item).eraseIf(f);
      if (erased > 0) {
        _realSize -= erased;
        if (_items.at(*item).empty()) {
          item = _validIndexes.erase(item);
          continue;
        }
      }
      ++item;
    }
  }
  void clear() {
    clearValidIndex();
    for (auto &item : _items) {
      item.clear();
    }
  }
  size_t size() const { return _realSize; }
  const ItemType &at(size_t index) { return _items.at(index); }
  template <typename F>
  void applyForEach(const F &f) const {
    upmq::ScopedReadRWLock readRWLock(_validIndexesLock);
    for (auto index : _validIndexes) {
      _items.at(index).applyForEach(f);
    }
  }
  template <typename F>
  ValidItemsValue applyForOnce(ValidItemsValue startFrom, const F &f) const {
    upmq::ScopedReadRWLockWithUnlock readRWLock(_validIndexesLock);
    auto index = _validIndexes.find(startFrom);
    const auto endIt = _validIndexes.end();
    if (index == endIt) {
      index = _validIndexes.begin();
    }
    if (index != endIt) {
      size_t i = *index;
      const auto next = std::next(index);
      const ValidItemsValue result = (next == endIt) ? *_validIndexes.begin() : *next;
      readRWLock.unlock();

      _items.at(i).applyForEach(f);
      return result;
    }
    return 0;
  }
  template <typename F>
  ValidItemsValue applyForOnceBackward(ValidItemsValue startFrom, const F &f) const {
    upmq::ScopedReadRWLockWithUnlock readRWLock(_validIndexesLock);
    ValidItemsType::const_reverse_iterator index(_validIndexes.find(startFrom));
    const auto rendIt = _validIndexes.rend();
    if (index == rendIt) {
      index = _validIndexes.rbegin();
    }
    if (index != rendIt) {
      size_t i = *index;
      const auto next = std::next(index);
      const ValidItemsValue result = (next == rendIt) ? *_validIndexes.rbegin() : *next;
      readRWLock.unlock();

      _items.at(i).applyForEach(f);
      return result;
    }
    return 0;
  }
  template <typename F>
  void applyForEachBackward(const F &f) const {
    upmq::ScopedReadRWLock readRWLock(_validIndexesLock);
    for (auto index = _validIndexes.rbegin(); index != _validIndexes.rend(); ++index) {
      _items.at(*index).applyForEach(f);
    }
  }
  template <typename F>
  void changeForEach(const F &f) {
    upmq::ScopedReadRWLock readRWLock(_validIndexesLock);
    for (auto index : _validIndexes) {
      _items.at(index).changeForEach(f);
    }
  }
  size_t indexOf(const Key &key) { return Poco::hash(key) % _size; }
};
}  // namespace upmq

#endif  // FIXED_SIZE_UNORDERD_MAP_H
