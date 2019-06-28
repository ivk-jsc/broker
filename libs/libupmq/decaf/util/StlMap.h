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

#ifndef _DECAF_UTIL_STLMAP_H_
#define _DECAF_UTIL_STLMAP_H_

#include <decaf/lang/Pointer.h>
#include <decaf/lang/exceptions/UnsupportedOperationException.h>
#include <decaf/util/AbstractCollection.h>
#include <decaf/util/AbstractSet.h>
#include <decaf/util/Collection.h>
#include <decaf/util/ConcurrentModificationException.h>
#include <decaf/util/Iterator.h>
#include <decaf/util/Map.h>
#include <decaf/util/NoSuchElementException.h>
#include <decaf/util/Set.h>
#include <decaf/util/comparators/Less.h>
#include <decaf/util/concurrent/Mutex.h>
#include <decaf/util/concurrent/Synchronizable.h>
#include <map>
#include <memory>

namespace decaf {
namespace util {

/**
 * Map template that wraps around a std::map to provide
 * a more user-friendly interface and to provide common
 * functions that do not exist in std::map.
 *
 * @since 1.0
 */
template <typename K, typename V, typename COMPARATOR = decaf::util::comparators::Less<K> >
class StlMap : public Map<K, V> {
 private:
  std::map<K, V, COMPARATOR> valueMap;
  mutable concurrent::Mutex mutex;
  int modCount;

 private:
  class AbstractMapIterator {
   protected:
    mutable int position;
    int expectedModCount;
    typename std::map<K, V, COMPARATOR>::iterator futureEntry;
    typename std::map<K, V, COMPARATOR>::iterator currentEntry;

    StlMap *associatedMap;

   private:
    AbstractMapIterator(const AbstractMapIterator &);
    AbstractMapIterator &operator=(const AbstractMapIterator &);

   public:
    AbstractMapIterator(StlMap *parent) : position(0), expectedModCount(parent->modCount), futureEntry(parent->valueMap.begin()), currentEntry(parent->valueMap.end()), associatedMap(parent) {}

    virtual ~AbstractMapIterator() {}

    virtual bool checkHasNext() const {
      if (futureEntry != this->associatedMap->valueMap.end()) {
        return true;
      }
      return false;
    }

    void checkConcurrentMod() const {
      if (expectedModCount != this->associatedMap->modCount) {
        throw ConcurrentModificationException(__FILE__, __LINE__, "StlMap modified outside this iterator");
      }
    }

    void makeNext() {
      checkConcurrentMod();

      if (!checkHasNext()) {
        throw NoSuchElementException(__FILE__, __LINE__, "No next element");
      }

      currentEntry = futureEntry;
      ++futureEntry;
    }

    virtual void doRemove() {
      checkConcurrentMod();

      if (currentEntry == this->associatedMap->valueMap.end()) {
        throw decaf::lang::exceptions::IllegalStateException(__FILE__, __LINE__, "Remove called before call to next()");
      }

      this->associatedMap->valueMap.erase(currentEntry);
      currentEntry = this->associatedMap->valueMap.end();

      ++expectedModCount;
      ++associatedMap->modCount;
    }
  };

  class EntryIterator : public Iterator<MapEntry<K, V> >, public AbstractMapIterator {
   private:
    EntryIterator(const EntryIterator &);
    EntryIterator &operator=(const EntryIterator &);

   public:
    EntryIterator(StlMap *parent) : AbstractMapIterator(parent) {}

    virtual ~EntryIterator() {}

    bool hasNext() const override { return this->checkHasNext(); }

    MapEntry<K, V> next() override {
      this->makeNext();
      return MapEntry<K, V>(this->currentEntry->first, this->currentEntry->second);
    }

    void remove() override { this->doRemove(); }
  };

  class KeyIterator : public Iterator<K>, public AbstractMapIterator {
   private:
    KeyIterator(const KeyIterator &);
    KeyIterator &operator=(const KeyIterator &);

   public:
    KeyIterator(StlMap *parent) : AbstractMapIterator(parent) {}

    virtual ~KeyIterator() {}

    bool hasNext() const override { return this->checkHasNext(); }

    K next() override {
      this->makeNext();
      return this->currentEntry->first;
    }

    void remove() override { this->doRemove(); }
  };

  class ValueIterator : public Iterator<V>, public AbstractMapIterator {
   private:
    ValueIterator(const ValueIterator &);
    ValueIterator &operator=(const ValueIterator &);

   public:
    ValueIterator(StlMap *parent) : AbstractMapIterator(parent) {}

    virtual ~ValueIterator() = default;

    bool hasNext() const override { return this->checkHasNext(); }

    V next() override {
      this->makeNext();
      return this->currentEntry->second;
    }

    void remove() override { this->doRemove(); }
  };

 private:
  class ConstAbstractMapIterator {
   protected:
    mutable int position;
    int expectedModCount;
    typename std::map<K, V, COMPARATOR>::const_iterator futureEntry;
    typename std::map<K, V, COMPARATOR>::const_iterator currentEntry;

    const StlMap *associatedMap;

   private:
    ConstAbstractMapIterator(const ConstAbstractMapIterator &);
    ConstAbstractMapIterator &operator=(const ConstAbstractMapIterator &);

   public:
    ConstAbstractMapIterator(const StlMap *parent)
        : position(0), expectedModCount(parent->modCount), futureEntry(parent->valueMap.begin()), currentEntry(parent->valueMap.end()), associatedMap(parent) {}

    virtual ~ConstAbstractMapIterator() {}

    virtual bool checkHasNext() const {
      if (futureEntry != this->associatedMap->valueMap.end()) {
        return true;
      }
      return false;
    }

    void checkConcurrentMod() const {
      if (expectedModCount != this->associatedMap->modCount) {
        throw ConcurrentModificationException(__FILE__, __LINE__, "StlMap modified outside this iterator");
      }
    }

    void makeNext() {
      checkConcurrentMod();

      if (!checkHasNext()) {
        throw NoSuchElementException(__FILE__, __LINE__, "No next element");
      }

      currentEntry = futureEntry;
      ++futureEntry;
    }
  };

  class ConstEntryIterator : public Iterator<MapEntry<K, V> >, public ConstAbstractMapIterator {
   private:
    ConstEntryIterator(const ConstEntryIterator &);
    ConstEntryIterator &operator=(const ConstEntryIterator &);

   public:
    ConstEntryIterator(const StlMap *parent) : ConstAbstractMapIterator(parent) {}

    virtual ~ConstEntryIterator() {}

    bool hasNext() const override { return this->checkHasNext(); }

    MapEntry<K, V> next() override {
      this->makeNext();
      return MapEntry<K, V>(this->currentEntry->first, this->currentEntry->second);
    }

    void remove() override { throw lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Cannot write to a const Iterator."); }
  };

  class ConstKeyIterator : public Iterator<K>, public ConstAbstractMapIterator {
   private:
    ConstKeyIterator(const ConstKeyIterator &);
    ConstKeyIterator &operator=(const ConstKeyIterator &);

   public:
    ConstKeyIterator(const StlMap *parent) : ConstAbstractMapIterator(parent) {}

    virtual ~ConstKeyIterator() {}

    bool hasNext() const override { return this->checkHasNext(); }

    K next() override {
      this->makeNext();
      return this->currentEntry->first;
    }

    void remove() override { throw lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Cannot write to a const Iterator."); }
  };

  class ConstValueIterator : public Iterator<V>, public ConstAbstractMapIterator {
   private:
    ConstValueIterator(const ConstValueIterator &);
    ConstValueIterator &operator=(const ConstValueIterator &);

   public:
    ConstValueIterator(const StlMap *parent) : ConstAbstractMapIterator(parent) {}

    virtual ~ConstValueIterator() {}

    bool hasNext() const override { return this->checkHasNext(); }

    V next() override {
      this->makeNext();
      return this->currentEntry->second;
    }

    void remove() override { throw lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Cannot write to a const Iterator."); }
  };

 private:
  // Special Set implementation that is backed by this HashMap
  class StlMapEntrySet : public AbstractSet<MapEntry<K, V> > {
   private:
    StlMap *associatedMap;

   private:
    StlMapEntrySet(const StlMapEntrySet &);
    StlMapEntrySet &operator=(const StlMapEntrySet &);

   public:
    StlMapEntrySet(StlMap *parent) : AbstractSet<MapEntry<K, V> >(), associatedMap(parent) {}

    virtual ~StlMapEntrySet() {}

    int size() const override { return associatedMap->size(); }

    void clear() override { associatedMap->clear(); }

    bool remove(const MapEntry<K, V> &entry) override {
      if (this->associatedMap->containsKey(entry.getKey()) && this->associatedMap->get(entry.getKey()) == entry.getValue()) {
        associatedMap->remove(entry.getKey());
        return true;
      }

      return false;
    }

    bool contains(const MapEntry<K, V> &entry) const override {
      if (this->associatedMap->containsKey(entry.getKey()) && this->associatedMap->get(entry.getKey()) == entry.getValue()) {
        return true;
      }
      return false;
    }

    Iterator<MapEntry<K, V> > *iterator() override { return new EntryIterator(associatedMap); }

    Iterator<MapEntry<K, V> > *iterator() const override { return new ConstEntryIterator(associatedMap); }
  };

  // Special Set implementation that is backed by this HashMap
  class ConstStlMapEntrySet : public AbstractSet<MapEntry<K, V> > {
   private:
    const StlMap *associatedMap;

   private:
    ConstStlMapEntrySet(const ConstStlMapEntrySet &);
    ConstStlMapEntrySet &operator=(const ConstStlMapEntrySet &);

   public:
    ConstStlMapEntrySet(const StlMap *parent) : AbstractSet<MapEntry<K, V> >(), associatedMap(parent) {}

    virtual ~ConstStlMapEntrySet() {}

    int size() const override { return associatedMap->size(); }

    void clear() override { throw decaf::lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Can't clear a const collection"); }

    bool remove(const MapEntry<K, V> &entry DECAF_UNUSED) override {
      DECAF_UNUSED_VAR(entry);
      throw decaf::lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Can't remove from const collection");
    }

    bool contains(const MapEntry<K, V> &entry) const override {
      if (this->associatedMap->containsKey(entry.getKey()) && this->associatedMap->get(entry.getKey()) == entry.getValue()) {
        return true;
      }
      return false;
    }

    Iterator<MapEntry<K, V> > *iterator() override { throw decaf::lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Can't return a non-const iterator for a const collection"); }

    Iterator<MapEntry<K, V> > *iterator() const override { return new ConstEntryIterator(associatedMap); }
  };

 private:
  class StlMapKeySet : public AbstractSet<K> {
   private:
    StlMap *associatedMap;

   private:
    StlMapKeySet(const StlMapKeySet &);
    StlMapKeySet &operator=(const StlMapKeySet &);

   public:
    StlMapKeySet(StlMap *parent) : AbstractSet<K>(), associatedMap(parent) {}

    virtual ~StlMapKeySet() {}

    bool contains(const K &key) const override { return this->associatedMap->containsKey(key); }

    int size() const override { return this->associatedMap->size(); }

    void clear() override { this->associatedMap->clear(); }

    bool remove(const K &key) override {
      if (this->associatedMap->containsKey(key)) {
        associatedMap->remove(key);
        return true;
      }
      return false;
    }

    Iterator<K> *iterator() override { return new KeyIterator(this->associatedMap); }

    Iterator<K> *iterator() const override { return new ConstKeyIterator(this->associatedMap); }
  };

  class ConstStlMapKeySet : public AbstractSet<K> {
   private:
    const StlMap *associatedMap;

   private:
    ConstStlMapKeySet(const ConstStlMapKeySet &);
    ConstStlMapKeySet &operator=(const ConstStlMapKeySet &);

   public:
    ConstStlMapKeySet(const StlMap *parent) : AbstractSet<K>(), associatedMap(parent) {}

    virtual ~ConstStlMapKeySet() {}

    bool contains(const K &key) const override { return this->associatedMap->containsKey(key); }

    int size() const override { return this->associatedMap->size(); }

    void clear() override { throw decaf::lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Can't modify a const collection"); }

    bool remove(const K &key DECAF_UNUSED) override {
      DECAF_UNUSED_VAR(key);
      throw decaf::lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Can't modify a const collection");
    }

    Iterator<K> *iterator() override { throw decaf::lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Can't return a non-const iterator for a const collection"); }

    Iterator<K> *iterator() const override { return new ConstKeyIterator(this->associatedMap); }
  };

 private:
  class StlMapValueCollection : public AbstractCollection<V> {
   private:
    StlMap *associatedMap;

   private:
    StlMapValueCollection(const StlMapValueCollection &);
    StlMapValueCollection &operator=(const StlMapValueCollection &);

   public:
    StlMapValueCollection(StlMap *parent) : AbstractCollection<V>(), associatedMap(parent) {}

    virtual ~StlMapValueCollection() {}

    bool contains(const V &value) const override { return this->associatedMap->containsValue(value); }

    int size() const override { return this->associatedMap->size(); }

    void clear() override { this->associatedMap->clear(); }

    Iterator<V> *iterator() override { return new ValueIterator(this->associatedMap); }

    Iterator<V> *iterator() const override { return new ConstValueIterator(this->associatedMap); }
  };

  class ConstStlMapValueCollection : public AbstractCollection<V> {
   private:
    const StlMap *associatedMap;

   private:
    ConstStlMapValueCollection(const ConstStlMapValueCollection &);
    ConstStlMapValueCollection &operator=(const ConstStlMapValueCollection &);

   public:
    ConstStlMapValueCollection(const StlMap *parent) : AbstractCollection<V>(), associatedMap(parent) {}

    virtual ~ConstStlMapValueCollection() {}

    bool contains(const V &value) const override { return this->associatedMap->containsValue(value); }

    int size() const override { return this->associatedMap->size(); }

    void clear() override { throw decaf::lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Can't modify a const collection"); }

    Iterator<V> *iterator() override { throw decaf::lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Can't return a non-const iterator for a const collection"); }

    Iterator<V> *iterator() const override { return new ConstValueIterator(this->associatedMap); }
  };

 private:
  // Cached values that are only initialized once a request for them is made.
  decaf::lang::Pointer<StlMapEntrySet> cachedEntrySet;
  decaf::lang::Pointer<StlMapKeySet> cachedKeySet;
  decaf::lang::Pointer<StlMapValueCollection> cachedValueCollection;

  // Cached values that are only initialized once a request for them is made.
  mutable decaf::lang::Pointer<ConstStlMapEntrySet> cachedConstEntrySet;
  mutable decaf::lang::Pointer<ConstStlMapKeySet> cachedConstKeySet;
  mutable decaf::lang::Pointer<ConstStlMapValueCollection> cachedConstValueCollection;

 public:
  /**
   * Default constructor - does nothing.
   */
  StlMap() : Map<K, V>(), valueMap(), mutex(), modCount(0), cachedEntrySet(), cachedKeySet(), cachedValueCollection(), cachedConstEntrySet(), cachedConstKeySet(), cachedConstValueCollection() {}

  /**
   * Copy constructor - copies the content of the given map into this one.
   *
   * @param source
   *      The source StlMap whose entries are copied into this Map.
   */
  StlMap(const StlMap &source)
      : Map<K, V>(), valueMap(), mutex(), modCount(0), cachedEntrySet(), cachedKeySet(), cachedValueCollection(), cachedConstEntrySet(), cachedConstKeySet(), cachedConstValueCollection() {
    copy(source);
  }

  /**
   * Copy constructor - copies the content of the given map into this one.
   *
   * @param source
   *      The source ma whose entries are copied into this Map..
   */
  StlMap(const Map<K, V> &source)
      : Map<K, V>(), valueMap(), mutex(), modCount(0), cachedEntrySet(), cachedKeySet(), cachedValueCollection(), cachedConstEntrySet(), cachedConstKeySet(), cachedConstValueCollection() {
    copy(source);
  }

  virtual ~StlMap() {}

  /**
   * {@inheritDoc}
   */
  virtual bool equals(const StlMap &source) const { return this->valueMap == source.valueMap; }

  /**
   * {@inheritDoc}
   */
  bool equals(const Map<K, V> &source) const override {
    typename std::unique_ptr<Iterator<K> > iterator(this->keySet().iterator());
    while (iterator->hasNext()) {
      K key = iterator->next();
      if (!this->containsKey(key)) {
        return false;
      }

      if (!(this->get(key) == source.get(key))) {
        return false;
      }
    }

    return true;
  }

  /**
   * {@inheritDoc}
   */
  virtual void copy(const StlMap &source) {
    this->valueMap.clear();
    this->valueMap.insert(source.valueMap.begin(), source.valueMap.end());
  }

  /**
   * {@inheritDoc}
   */
  void copy(const Map<K, V> &source) override {
    this->clear();
    this->putAll(source);
  }

  /**
   * {@inheritDoc}
   */
  void clear() override { valueMap.clear(); }

  /**
   * {@inheritDoc}
   */
  bool containsKey(const K &key) const override {
    if (valueMap.empty()) {
      return false;
    }

    typename std::map<K, V, COMPARATOR>::const_iterator iter = valueMap.find(key);
    return iter != valueMap.end();
  }

  /**
   * {@inheritDoc}
   */
  bool containsValue(const V &value) const override {
    if (valueMap.empty()) {
      return false;
    }

    typename std::map<K, V, COMPARATOR>::const_iterator iter = valueMap.begin();
    for (; iter != valueMap.end(); ++iter) {
      if ((*iter).second == value) {
        return true;
      }
    }

    return false;
  }

  /**
   * {@inheritDoc}
   */
  bool isEmpty() const override { return valueMap.empty(); }

  /**
   * {@inheritDoc}
   */
  int size() const override { return (int)valueMap.size(); }

  /**
   * {@inheritDoc}
   */
  V &get(const K &key) override {
    typename std::map<K, V, COMPARATOR>::iterator iter;
    iter = valueMap.find(key);
    if (iter == valueMap.end()) {
      throw NoSuchElementException(__FILE__, __LINE__, "Key does not exist in map");
    }

    return iter->second;
  }

  /**
   * {@inheritDoc}
   */
  const V &get(const K &key) const override {
    typename std::map<K, V, COMPARATOR>::const_iterator iter;
    iter = valueMap.find(key);
    if (iter == valueMap.end()) {
      throw NoSuchElementException(__FILE__, __LINE__, "Key does not exist in map");
    }

    return iter->second;
  }

  /**
   * {@inheritDoc}
   */
  bool put(const K &key, const V &value) override {
    bool result = false;
    if (this->containsKey(key)) {
      result = true;
    }
    valueMap[key] = value;
    modCount++;
    return result;
  }

  /**
   * {@inheritDoc}
   */
  bool put(const K &key, const V &value, V &oldValue) override {
    bool result = false;
    if (this->containsKey(key)) {
      result = true;
      oldValue = valueMap[key];
    }
    valueMap[key] = value;
    ++modCount;
    return result;
  }

  /**
   * {@inheritDoc}
   */
  virtual void putAll(const StlMap<K, V, COMPARATOR> &other) {
    this->valueMap.insert(other.valueMap.begin(), other.valueMap.end());
    ++this->modCount;
  }

  /**
   * {@inheritDoc}
   */
  void putAll(const Map<K, V> &other) override {
    typename std::unique_ptr<Iterator<K> > iterator(other.keySet().iterator());
    while (iterator->hasNext()) {
      K key = iterator->next();
      this->put(key, other.get(key));
    }
  }

  /**
   * {@inheritDoc}
   */
  V remove(const K &key) override {
    typename std::map<K, V, COMPARATOR>::iterator iter = valueMap.find(key);
    if (iter == valueMap.end()) {
      throw NoSuchElementException(__FILE__, __LINE__, "Key is not present in this Map.");
    }

    V result = iter->second;
    valueMap.erase(iter);
    modCount++;
    return result;
  }

  Set<MapEntry<K, V> > &entrySet() override {
    if (this->cachedEntrySet == nullptr) {
      this->cachedEntrySet.reset(new StlMapEntrySet(this));
    }
    return *(this->cachedEntrySet);
  }

  const Set<MapEntry<K, V> > &entrySet() const override {
    if (this->cachedConstEntrySet == nullptr) {
      this->cachedConstEntrySet.reset(new ConstStlMapEntrySet(this));
    }
    return *(this->cachedConstEntrySet);
  }

  Set<K> &keySet() override {
    if (this->cachedKeySet == nullptr) {
      this->cachedKeySet.reset(new StlMapKeySet(this));
    }
    return *(this->cachedKeySet);
  }

  const Set<K> &keySet() const override {
    if (this->cachedConstKeySet == nullptr) {
      this->cachedConstKeySet.reset(new ConstStlMapKeySet(this));
    }
    return *(this->cachedConstKeySet);
  }

  Collection<V> &values() override {
    if (this->cachedValueCollection == nullptr) {
      this->cachedValueCollection.reset(new StlMapValueCollection(this));
    }
    return *(this->cachedValueCollection);
  }

  const Collection<V> &values() const override {
    if (this->cachedConstValueCollection == nullptr) {
      this->cachedConstValueCollection.reset(new ConstStlMapValueCollection(this));
    }
    return *(this->cachedConstValueCollection);
  }

 public:
  void lock() override { mutex.lock(); }

  bool tryLock() override { return mutex.tryLock(); }

  void unlock() override { mutex.unlock(); }

  void wait() override { mutex.wait(); }

  void wait(long long millisecs) override { mutex.wait(millisecs); }

  void wait(long long millisecs, int nanos) override { mutex.wait(millisecs, nanos); }

  void notify() override { mutex.notify(); }

  void notifyAll() override { mutex.notifyAll(); }
};
}  // namespace util
}  // namespace decaf

#endif /*_DECAF_UTIL_STLMAP_H_*/
