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

#ifndef _DECAF_UTIL_HASHMAP_H_
#define _DECAF_UTIL_HASHMAP_H_

#include <decaf/util/Config.h>

#include <decaf/lang/ArrayPointer.h>
#include <decaf/lang/Pointer.h>
#include <decaf/lang/exceptions/UnsupportedOperationException.h>
#include <decaf/util/AbstractMap.h>
#include <decaf/util/AbstractSet.h>
#include <decaf/util/ConcurrentModificationException.h>
#include <decaf/util/HashCode.h>

namespace decaf {
namespace util {

/**
 * Hash table based implementation of the Map interface. This implementation provides all
 * of the optional map operations, and permits null values and the null key.  This class
 * makes no guarantees as to the order of the map; in particular, it does not guarantee
 * that the order will remain constant over time.
 *
 * This implementation provides constant-time performance for the basic operations (get
 * and put), assuming the hash function disperses the elements properly among the buckets.
 * Iteration over collection views requires time proportional to the "capacity" of the
 * HashMap instance (the number of buckets) plus its size (the number of key-value mappings).
 * Thus, it's very important not to set the initial capacity too high (or the load factor too
 * low) if iteration performance is important.
 *
 * An instance of HashMap has two parameters that affect its performance: initial capacity
 * and load factor. The capacity is the number of buckets in the hash table, and the initial
 * capacity is simply the capacity at the time the hash table is created. The load factor is
 * a measure of how full the hash table is allowed to get before its capacity is automatically
 * increased. When the number of entries in the hash table exceeds the product of the load
 * factor and the current capacity, the hash table is rehashed (that is, internal data
 * structures are rebuilt) so that the hash table has approximately twice the number of buckets.
 *
 * As a general rule, the default load factor (.75) offers a good tradeoff between time and
 * space costs. Higher values decrease the space overhead but increase the lookup cost
 * (reflected in most of the operations of the HashMap class, including get and put). The
 * expected number of entries in the map and its load factor should be taken into account
 * when setting its initial capacity, so as to minimize the number of rehash operations. If
 * the initial capacity is greater than the maximum number of entries divided by the load
 * factor, no rehash operations will ever occur.
 *
 * If many mappings are to be stored in a HashMap instance, creating it with a sufficiently
 * large capacity will allow the mappings to be stored more efficiently than letting it
 * perform automatic rehashing as needed to grow the table.
 *
 * Note that this implementation is not synchronized. If multiple threads access a hash map
 * concurrently, and at least one of the threads modifies the map structurally, it must be
 * synchronized externally. (A structural modification is any operation that adds or deletes
 * one or more mappings; merely changing the value associated with a key that an instance
 * already contains is not a structural modification.) This is typically accomplished by
 * synchronizing on some object that naturally encapsulates the map. If no such object
 * exists, the map should be "wrapped" using the Collections::synchronizedMap method.
 * This is best done at creation time, to prevent accidental unsynchronized access to the map:
 *
 *   Map<K, V>* map = Collections::synchronizedMap(new HashMap<K, V>());
 *
 * The iterators returned by all of this class's "collection view methods" are fail-fast:
 * if the map is structurally modified at any time after the iterator is created, in any
 * way except through the iterator's own remove method, the iterator will throw a
 * ConcurrentModificationException. Thus, in the face of concurrent modification, the
 * iterator fails quickly and cleanly, rather than risking arbitrary, non-deterministic
 * behavior at an undetermined time in the future.
 *
 * Note that the fail-fast behavior of an iterator cannot be guaranteed as it is, generally
 * speaking, impossible to make any hard guarantees in the presence of unsynchronized
 * concurrent modification. Fail-fast iterators throw ConcurrentModificationException on a
 * best-effort basis. Therefore, it would be wrong to write a program that depended on this
 * exception for its correctness: the fail-fast behavior of iterators should be used only
 * to detect bugs.
 *
 * @since 1.0
 */
template <typename K, typename V, typename HASHCODE = HashCode<K> >
class HashMap : public AbstractMap<K, V> {
 protected:
  class HashMapEntry : public MapEntry<K, V> {
   public:
    int origKeyHash;

    HashMapEntry *next;

    HashMapEntry(const HashMapEntry &) = delete;
    HashMapEntry &operator=(const HashMapEntry &) = delete;

    HashMapEntry(const K &key_, const V &value_, int hash_) : MapEntry<K, V>(), origKeyHash(hash_), next(nullptr) {
      this->setKey(key_);
      this->setValue(value_);
      this->origKeyHash = hash_;
    }

    HashMapEntry(const K &key_, const V &value_) : MapEntry<K, V>(key_, value_), origKeyHash(0), next(nullptr) {
      this->origKeyHash = HASHCODE()(key_);
    }
  };

 private:
  class AbstractMapIterator {
   protected:
    mutable int position;
    int expectedModCount;
    HashMapEntry *futureEntry;
    HashMapEntry *currentEntry;
    HashMapEntry *prevEntry;

    HashMap *associatedMap;

   public:
    AbstractMapIterator(const AbstractMapIterator &) = delete;
    AbstractMapIterator &operator=(const AbstractMapIterator &) = delete;

    AbstractMapIterator(HashMap *parent)
        : position(0), expectedModCount(parent->modCount), futureEntry(nullptr), currentEntry(nullptr), prevEntry(nullptr), associatedMap(parent) {}

    virtual ~AbstractMapIterator() {}

    virtual bool checkHasNext() const {
      if (futureEntry != nullptr) {
        return true;
      }
      while (position < associatedMap->elementData.length()) {
        if (associatedMap->elementData[position] == nullptr) {
          position++;
        } else {
          return true;
        }
      }
      return false;
    }

    void checkConcurrentMod() const {
      if (expectedModCount != associatedMap->modCount) {
        throw ConcurrentModificationException(__FILE__, __LINE__, "HashMap modified outside this iterator");
      }
    }

    void makeNext() {
      checkConcurrentMod();

      if (!checkHasNext()) {
        throw NoSuchElementException(__FILE__, __LINE__, "No next element");
      }

      if (futureEntry == nullptr) {
        currentEntry = associatedMap->elementData[position++];
        futureEntry = currentEntry->next;
        prevEntry = nullptr;
      } else {
        if (currentEntry != nullptr) {
          prevEntry = currentEntry;
        }
        currentEntry = futureEntry;
        futureEntry = futureEntry->next;
      }
    }

    virtual void doRemove() {
      checkConcurrentMod();

      if (currentEntry == nullptr) {
        throw decaf::lang::exceptions::IllegalStateException(__FILE__, __LINE__, "Remove called before call to next()");
      }

      if (prevEntry == nullptr) {
        int index = currentEntry->origKeyHash & (associatedMap->elementData.length() - 1);
        associatedMap->elementData[index] = associatedMap->elementData[index]->next;
      } else {
        prevEntry->next = currentEntry->next;
      }

      delete currentEntry;
      currentEntry = nullptr;

      ++expectedModCount;
      ++(associatedMap->modCount);
      --(associatedMap->elementCount);
    }
  };

  class EntryIterator : public Iterator<MapEntry<K, V> >, public AbstractMapIterator {
   public:
    EntryIterator(const EntryIterator &) = delete;
    EntryIterator &operator=(const EntryIterator &) = delete;

    EntryIterator(HashMap *parent) : AbstractMapIterator(parent) {}

    virtual ~EntryIterator() {}

    bool hasNext() const override { return this->checkHasNext(); }

    MapEntry<K, V> next() override {
      this->makeNext();
      return *(this->currentEntry);
    }

    void remove() override { this->doRemove(); }
  };

  class KeyIterator : public Iterator<K>, public AbstractMapIterator {
   public:
    KeyIterator(const KeyIterator &) = delete;
    KeyIterator &operator=(const KeyIterator &) = delete;

    KeyIterator(HashMap *parent) : AbstractMapIterator(parent) {}

    virtual ~KeyIterator() {}

    bool hasNext() const override { return this->checkHasNext(); }

    K next() override {
      this->makeNext();
      return this->currentEntry->getKey();
    }

    void remove() override { this->doRemove(); }
  };

  class ValueIterator : public Iterator<V>, public AbstractMapIterator {
   public:
    ValueIterator(const ValueIterator &) = delete;
    ValueIterator &operator=(const ValueIterator &) = delete;

    ValueIterator(HashMap *parent) : AbstractMapIterator(parent) {}

    virtual ~ValueIterator() {}

    bool hasNext() const override { return this->checkHasNext(); }

    V next() override {
      this->makeNext();
      return this->currentEntry->getValue();
    }

    void remove() override { this->doRemove(); }
  };

 private:
  class ConstAbstractMapIterator {
   protected:
    mutable int position;
    int expectedModCount;
    const HashMapEntry *futureEntry;
    const HashMapEntry *currentEntry;
    const HashMapEntry *prevEntry;

    const HashMap *associatedMap;

   public:
    ConstAbstractMapIterator(const ConstAbstractMapIterator &) = delete;
    ConstAbstractMapIterator &operator=(const ConstAbstractMapIterator &) = delete;

    ConstAbstractMapIterator(const HashMap *parent)
        : position(0), expectedModCount(parent->modCount), futureEntry(nullptr), currentEntry(nullptr), prevEntry(nullptr), associatedMap(parent) {}

    virtual ~ConstAbstractMapIterator() {}

    virtual bool checkHasNext() const {
      if (futureEntry != nullptr) {
        return true;
      }
      while (position < associatedMap->elementData.length()) {
        if (associatedMap->elementData[position] == nullptr) {
          position++;
        } else {
          return true;
        }
      }
      return false;
    }

    void checkConcurrentMod() const {
      if (expectedModCount != associatedMap->modCount) {
        throw ConcurrentModificationException(__FILE__, __LINE__, "HashMap modified outside this iterator");
      }
    }

    void makeNext() {
      checkConcurrentMod();

      if (!checkHasNext()) {
        throw NoSuchElementException(__FILE__, __LINE__, "No next element");
      }

      if (futureEntry == nullptr) {
        currentEntry = associatedMap->elementData[position++];
        futureEntry = currentEntry->next;
        prevEntry = nullptr;
      } else {
        if (currentEntry != nullptr) {
          prevEntry = currentEntry;
        }
        currentEntry = futureEntry;
        futureEntry = futureEntry->next;
      }
    }
  };

  class ConstEntryIterator : public Iterator<MapEntry<K, V> >, public ConstAbstractMapIterator {
   public:
    ConstEntryIterator(const ConstEntryIterator &) = delete;
    ConstEntryIterator &operator=(const ConstEntryIterator &) = delete;

    ConstEntryIterator(const HashMap *parent) : ConstAbstractMapIterator(parent) {}

    virtual ~ConstEntryIterator() {}

    bool hasNext() const override { return this->checkHasNext(); }

    MapEntry<K, V> next() override {
      this->makeNext();
      return *(this->currentEntry);
    }

    void remove() override { throw lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Cannot write to a const Iterator."); }
  };

  class ConstKeyIterator : public Iterator<K>, public ConstAbstractMapIterator {
   public:
    ConstKeyIterator(const ConstKeyIterator &) = delete;
    ConstKeyIterator &operator=(const ConstKeyIterator &) = delete;

    ConstKeyIterator(const HashMap *parent) : ConstAbstractMapIterator(parent) {}

    virtual ~ConstKeyIterator() {}

    bool hasNext() const override { return this->checkHasNext(); }

    K next() override {
      this->makeNext();
      return this->currentEntry->getKey();
    }

    void remove() override { throw lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Cannot write to a const Iterator."); }
  };

  class ConstValueIterator : public Iterator<V>, public ConstAbstractMapIterator {
   public:
    ConstValueIterator(const ConstValueIterator &) = delete;
    ConstValueIterator &operator=(const ConstValueIterator &) = delete;

    ConstValueIterator(const HashMap *parent) : ConstAbstractMapIterator(parent) {}

    virtual ~ConstValueIterator() {}

    bool hasNext() const override { return this->checkHasNext(); }

    V next() override {
      this->makeNext();
      return this->currentEntry->getValue();
    }

    void remove() override { throw lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Cannot write to a const Iterator."); }
  };

 protected:
  // Special Set implementation that is backed by this HashMap
  class HashMapEntrySet : public AbstractSet<MapEntry<K, V> > {
   private:
    HashMap *associatedMap;

   public:
    HashMapEntrySet(const HashMapEntrySet &) = delete;
    HashMapEntrySet &operator=(const HashMapEntrySet &) = delete;

    HashMapEntrySet(HashMap *parent) : AbstractSet<MapEntry<K, V> >(), associatedMap(parent) {}

    virtual ~HashMapEntrySet() {}

    int size() const override { return associatedMap->elementCount; }

    void clear() override { associatedMap->clear(); }

    bool remove(const MapEntry<K, V> &entry) override {
      HashMapEntry *result = associatedMap->getEntry(entry.getKey());
      if (result != nullptr && entry.getValue() == result->getValue()) {
        associatedMap->removeEntry(result);
        return true;
      }

      return false;
    }

    bool contains(const MapEntry<K, V> &entry) const override {
      HashMapEntry *result = associatedMap->getEntry(entry.getKey());
      return result != nullptr && entry.getValue() == result->getValue();
    }

    Iterator<MapEntry<K, V> > *iterator() override { return new EntryIterator(associatedMap); }

    Iterator<MapEntry<K, V> > *iterator() const override { return new ConstEntryIterator(associatedMap); }
  };

  // Special Set implementation that is backed by this HashMap
  class ConstHashMapEntrySet : public AbstractSet<MapEntry<K, V> > {
   private:
    const HashMap *associatedMap;

   public:
    ConstHashMapEntrySet(const ConstHashMapEntrySet &) = delete;
    ConstHashMapEntrySet &operator=(const ConstHashMapEntrySet &) = delete;

    ConstHashMapEntrySet(const HashMap *parent) : AbstractSet<MapEntry<K, V> >(), associatedMap(parent) {}

    virtual ~ConstHashMapEntrySet() {}

    int size() const override { return associatedMap->elementCount; }

    void clear() override { throw decaf::lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Can't clear a const collection"); }

    bool remove(const MapEntry<K, V> &entry DECAF_UNUSED) override {
      DECAF_UNUSED_VAR(entry);
      throw decaf::lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Can't remove from const collection");
    }

    bool contains(const MapEntry<K, V> &entry) const override {
      HashMapEntry *result = associatedMap->getEntry(entry.getKey());
      return result != nullptr && entry.getValue() == result->getValue() || false;
    }

    Iterator<MapEntry<K, V> > *iterator() override {
      throw decaf::lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Can't return a non-const iterator for a const collection");
    }

    Iterator<MapEntry<K, V> > *iterator() const override { return new ConstEntryIterator(associatedMap); }
  };

 protected:
  class HashMapKeySet : public AbstractSet<K> {
   private:
    HashMap *associatedMap;

   public:
    HashMapKeySet(const HashMapKeySet &) = delete;
    HashMapKeySet &operator=(const HashMapKeySet &) = delete;

    HashMapKeySet(HashMap *parent) : AbstractSet<K>(), associatedMap(parent) {}

    virtual ~HashMapKeySet() {}

    bool contains(const K &key) const override { return this->associatedMap->containsKey(key); }

    int size() const override { return this->associatedMap->size(); }

    void clear() override { this->associatedMap->clear(); }

    bool remove(const K &key) override {
      HashMapEntry *entry = this->associatedMap->removeEntry(key);
      if (entry != nullptr) {
        delete entry;
        return true;
      }
      return false;
    }

    Iterator<K> *iterator() override { return new KeyIterator(this->associatedMap); }

    Iterator<K> *iterator() const override { return new ConstKeyIterator(this->associatedMap); }
  };

  class ConstHashMapKeySet : public AbstractSet<K> {
   private:
    const HashMap *associatedMap;

   public:
    ConstHashMapKeySet(const ConstHashMapKeySet &) = delete;
    ConstHashMapKeySet &operator=(const ConstHashMapKeySet &) = delete;

    ConstHashMapKeySet(const HashMap *parent) : AbstractSet<K>(), associatedMap(parent) {}

    virtual ~ConstHashMapKeySet() {}

    bool contains(const K &key) const override { return this->associatedMap->containsKey(key); }

    int size() const override { return this->associatedMap->size(); }

    void clear() override { throw decaf::lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Can't modify a const collection"); }

    bool remove(const K &key DECAF_UNUSED) override {
      DECAF_UNUSED_VAR(key);
      throw decaf::lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Can't modify a const collection");
    }

    Iterator<K> *iterator() override {
      throw decaf::lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Can't return a non-const iterator for a const collection");
    }

    Iterator<K> *iterator() const override { return new ConstKeyIterator(this->associatedMap); }
  };

 protected:
  class HashMapValueCollection : public AbstractCollection<V> {
   private:
    HashMap *associatedMap;

   public:
    HashMapValueCollection(const HashMapValueCollection &) = delete;
    HashMapValueCollection &operator=(const HashMapValueCollection &) = delete;

    HashMapValueCollection(HashMap *parent) : AbstractCollection<V>(), associatedMap(parent) {}

    virtual ~HashMapValueCollection() {}

    bool contains(const V &value) const override { return this->associatedMap->containsValue(value); }

    int size() const override { return this->associatedMap->size(); }

    void clear() override { this->associatedMap->clear(); }

    Iterator<V> *iterator() override { return new ValueIterator(this->associatedMap); }

    Iterator<V> *iterator() const override { return new ConstValueIterator(this->associatedMap); }
  };

  class ConstHashMapValueCollection : public AbstractCollection<V> {
   private:
    const HashMap *associatedMap;

   public:
    ConstHashMapValueCollection(const ConstHashMapValueCollection &) = delete;
    ConstHashMapValueCollection &operator=(const ConstHashMapValueCollection &) = delete;

    ConstHashMapValueCollection(const HashMap *parent) : AbstractCollection<V>(), associatedMap(parent) {}

    virtual ~ConstHashMapValueCollection() {}

    bool contains(const V &value) const override { return this->associatedMap->containsValue(value); }

    int size() const override { return this->associatedMap->size(); }

    void clear() override { throw decaf::lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Can't modify a const collection"); }

    Iterator<V> *iterator() override {
      throw decaf::lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Can't return a non-const iterator for a const collection");
    }

    Iterator<V> *iterator() const override { return new ConstValueIterator(this->associatedMap); }
  };

 protected:
  /**
   * The Hash Code generator for this map's keys.
   */
  HASHCODE hashFunc;

  /*
   * Actual count of entries
   */
  int elementCount;

  /*
   * The internal data structure to hold Entries, Array of MapEntry pointers.
   */
  decaf::lang::ArrayPointer<HashMapEntry *> elementData;

  /*
   * modification count, to keep track of structural modifications between the
   * HashMap and the iterator
   */
  int modCount;

  /*
   * maximum ratio of (stored elements)/(storage size) which does not lead to rehash
   */
  float loadFactor;

  /*
   * maximum number of elements that can be put in this map before having to rehash
   */
  int threshold;

  // Cached values that are only initialized once a request for them is made.
  decaf::lang::Pointer<HashMapEntrySet> cachedEntrySet;
  decaf::lang::Pointer<HashMapKeySet> cachedKeySet;
  decaf::lang::Pointer<HashMapValueCollection> cachedValueCollection;

  // Cached values that are only initialized once a request for them is made.
  mutable decaf::lang::Pointer<ConstHashMapEntrySet> cachedConstEntrySet;
  mutable decaf::lang::Pointer<ConstHashMapKeySet> cachedConstKeySet;
  mutable decaf::lang::Pointer<ConstHashMapValueCollection> cachedConstValueCollection;

 private:
  void computeThreshold() { threshold = static_cast<int>(static_cast<float>(elementData.length()) * loadFactor); }

  static int calculateCapacity(int x) {
    if (x >= 1 << 30) {
      return 1 << 30;
    }

    if (x == 0) {
      return 16;
    }
    x = x - 1;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
  }

 public:
  /**
   * Creates a new empty HashMap with default configuration settings.
   */
  HashMap()
      : AbstractMap<K, V>(),
        hashFunc(),
        elementCount(0),
        elementData(),
        modCount(0),
        loadFactor(0.75),
        threshold(0),
        cachedEntrySet(),
        cachedKeySet(),
        cachedValueCollection(),
        cachedConstEntrySet(),
        cachedConstKeySet(),
        cachedConstValueCollection() {
    int capacity = calculateCapacity(12);
    elementCount = 0;
    elementData = decaf::lang::ArrayPointer<HashMapEntry *>(capacity);
    computeThreshold();
  }

  /**
   * Constructs a new HashMap instance with the specified capacity.
   *
   * @param capacity
   *    The initial capacity of this hash map.
   *
   * @throws IllegalArgumentException when the capacity is less than zero.
   */
  HashMap(int capacity)
      : AbstractMap<K, V>(),
        hashFunc(),
        elementCount(0),
        elementData(),
        modCount(0),
        loadFactor(0.75),
        threshold(0),
        cachedEntrySet(),
        cachedKeySet(),
        cachedValueCollection(),
        cachedConstEntrySet(),
        cachedConstKeySet(),
        cachedConstValueCollection() {
    if (capacity >= 0) {
      capacity = calculateCapacity(capacity);
      elementCount = 0;
      elementData = decaf::lang::ArrayPointer<HashMapEntry *>(capacity);
      computeThreshold();
    } else {
      throw decaf::lang::exceptions::IllegalArgumentException(__FILE__, __LINE__, "Invalid capacity configuration");
    }
  }

  /**
   * Constructs a new HashMap instance with the specified capacity.
   *
   * @param capacity
   *    The initial capacity of this hash map.
   * @param loadFactor
   *    The load factor to use for this hash map.
   *
   * @throws IllegalArgumentException when the capacity is less than zero.
   */
  HashMap(int capacity, float loadFactor_)
      : AbstractMap<K, V>(),
        hashFunc(),
        elementCount(0),
        elementData(),
        modCount(0),
        loadFactor(0.75),
        threshold(0),
        cachedEntrySet(),
        cachedKeySet(),
        cachedValueCollection(),
        cachedConstEntrySet(),
        cachedConstKeySet(),
        cachedConstValueCollection() {
    if (capacity >= 0 && loadFactor_ > 0) {
      capacity = calculateCapacity(capacity);
      elementCount = 0;
      elementData = decaf::lang::ArrayPointer<HashMapEntry *>(capacity);
      this->loadFactor = loadFactor_;
      computeThreshold();
    } else {
      throw decaf::lang::exceptions::IllegalArgumentException(__FILE__, __LINE__, "Invalid configuration");
    }
  }

  /**
   * Creates a new HashMap with default configuration settings and fills it with the contents
   * of the given source Map instance.
   *
   * @param map
   *      The Map instance whose elements are copied into this HashMap instance.
   */
  HashMap(const HashMap<K, V> &map)
      : AbstractMap<K, V>(),
        hashFunc(),
        elementCount(0),
        elementData(),
        modCount(0),
        loadFactor(0.75),
        threshold(0),
        cachedEntrySet(),
        cachedKeySet(),
        cachedValueCollection(),
        cachedConstEntrySet(),
        cachedConstKeySet(),
        cachedConstValueCollection() {
    int capacity = calculateCapacity(map.size());
    elementCount = 0;
    elementData = decaf::lang::ArrayPointer<HashMapEntry *>(capacity);
    computeThreshold();
    putAll(map);
  }

  /**
   * Creates a new HashMap with default configuration settings and fills it with the contents
   * of the given source Map instance.
   *
   * @param map
   *    The Map instance whose elements are copied into this HashMap instance.
   */
  HashMap(const Map<K, V> &map)
      : AbstractMap<K, V>(),
        hashFunc(),
        elementCount(0),
        elementData(),
        modCount(0),
        loadFactor(0.75),
        threshold(0),
        cachedEntrySet(),
        cachedKeySet(),
        cachedValueCollection(),
        cachedConstEntrySet(),
        cachedConstKeySet(),
        cachedConstValueCollection() {
    int capacity = calculateCapacity(map.size());
    elementCount = 0;
    elementData = decaf::lang::ArrayPointer<HashMapEntry *>(capacity);
    computeThreshold();
    putAll(map);
  }

  virtual ~HashMap() {
    for (int i = 0; i < elementData.length(); i++) {
      HashMapEntry *entry = elementData[i];
      while (entry != nullptr) {
        HashMapEntry *temp = entry;
        entry = entry->next;
        delete temp;
      }
    }
  }

 public:
  HashMap<K, V> &operator=(const Map<K, V> &other) {
    this->copy(other);
    return *this;
  }

  HashMap<K, V> &operator=(const HashMap<K, V> &other) {
    this->copy(other);
    return *this;
  }

  bool operator==(const Map<K, V> &other) const { return this->equals(other); }

  bool operator!=(const Map<K, V> &other) const { return !this->equals(other); }

 public:
  void clear() override {
    if (elementCount > 0) {
      elementCount = 0;
      for (int i = 0; i < elementData.length(); ++i) {
        HashMapEntry *entry = elementData[i];
        elementData[i] = nullptr;
        while (entry != nullptr) {
          HashMapEntry *temp = entry;
          entry = entry->next;
          delete temp;
        }
      }
      ++modCount;
    }
  }

  bool isEmpty() const override { return elementCount == 0; }

  int size() const override { return elementCount; }

  bool containsKey(const K &key) const override {
    const HashMapEntry *entry = getEntry(key);
    return entry != nullptr;
  }

  bool containsValue(const V &value) const override {
    for (int i = 0; i < elementData.length(); i++) {
      const HashMapEntry *entry = elementData[i];
      while (entry != nullptr) {
        if (value == entry->getValue()) {
          return true;
        }
        entry = entry->next;
      }
    }
    return false;
  }

  V &get(const K &key) override {
    HashMapEntry *entry = getEntry(key);
    if (entry != nullptr) {
      return entry->getValue();
    }

    throw NoSuchElementException(__FILE__, __LINE__, "The specified key is not present in the Map");
  }

  const V &get(const K &key) const override {
    const HashMapEntry *entry = getEntry(key);
    if (entry != nullptr) {
      return entry->getValue();
    }

    throw NoSuchElementException(__FILE__, __LINE__, "The specified key is not present in the Map");
  }

  bool put(const K &key, const V &value) override { return this->putImpl(key, value); }

  bool put(const K &key, const V &value, V &oldValue) override { return this->putImpl(key, value, oldValue); }

  void putAll(const Map<K, V> &map) override {
    if (!map.isEmpty()) {
      putAllImpl(map);
    }
  }

  V remove(const K &key) override {
    HashMapEntry *entry = removeEntry(key);
    if (entry != nullptr) {
      V oldValue = entry->getValue();
      delete entry;
      return oldValue;
    }

    throw NoSuchElementException(__FILE__, __LINE__, "Specified key not present in the Map.");
  }

  Set<MapEntry<K, V> > &entrySet() override {
    if (this->cachedEntrySet == nullptr) {
      this->cachedEntrySet.reset(new HashMapEntrySet(this));
    }
    return *(this->cachedEntrySet);
  }

  const Set<MapEntry<K, V> > &entrySet() const override {
    if (this->cachedConstEntrySet == nullptr) {
      this->cachedConstEntrySet.reset(new ConstHashMapEntrySet(this));
    }
    return *(this->cachedConstEntrySet);
  }

  Set<K> &keySet() override {
    if (this->cachedKeySet == nullptr) {
      this->cachedKeySet.reset(new HashMapKeySet(this));
    }
    return *(this->cachedKeySet);
  }

  const Set<K> &keySet() const override {
    if (this->cachedConstKeySet == nullptr) {
      this->cachedConstKeySet.reset(new ConstHashMapKeySet(this));
    }
    return *(this->cachedConstKeySet);
  }

  Collection<V> &values() override {
    if (this->cachedValueCollection == nullptr) {
      this->cachedValueCollection.reset(new HashMapValueCollection(this));
    }
    return *(this->cachedValueCollection);
  }

  const Collection<V> &values() const override {
    if (this->cachedConstValueCollection == nullptr) {
      this->cachedConstValueCollection.reset(new ConstHashMapValueCollection(this));
    }
    return *(this->cachedConstValueCollection);
  }

  bool equals(const Map<K, V> &source) const override {
    if (this == &source) {
      return true;
    }

    if (size() != source.size()) {
      return false;
    }

    try {
      decaf::lang::Pointer<Iterator<MapEntry<K, V> > > iter(entrySet().iterator());
      while (iter->hasNext()) {
        MapEntry<K, V> entry = iter->next();
        K key = entry.getKey();
        V mine = entry.getValue();

        if (!source.containsKey(key)) {
          return false;
        }

        if (source.get(key) != mine) {
          return false;
        }
      }
    } catch (decaf::lang::exceptions::NullPointerException &) {
      return false;
    } catch (decaf::lang::exceptions::ClassCastException &) {
      return false;
    }
    return true;
  }

  void copy(const Map<K, V> &source) override {
    int capacity = calculateCapacity(source.size());
    this->clear();
    if (capacity > elementData.length()) {
      elementData = decaf::lang::ArrayPointer<HashMapEntry *>(capacity);
    }
    computeThreshold();
    putAll(source);
  }

  virtual std::string toString() const { return "HashMap"; }

 protected:
  virtual HashMapEntry *getEntry(const K &key) const {
    HashMapEntry *result = nullptr;

    int hash = hashFunc(key);
    int index = hash & (elementData.length() - 1);
    result = findKeyEntry(key, index, hash);

    return result;
  }

  virtual bool putImpl(const K &key, const V &value) {
    V oldValue;
    return putImpl(key, value, oldValue);
  }

  virtual bool putImpl(const K &key, const V &value, V &oldValue) {
    bool replaced = true;
    HashMapEntry *entry = nullptr;

    int hash = hashFunc(key);
    int index = hash & (elementData.length() - 1);

    entry = findKeyEntry(key, index, hash);

    if (entry == nullptr) {
      ++modCount;
      entry = createHashedEntry(key, index, hash);
      if (++elementCount > threshold) {
        rehash();
      }
      replaced = false;
    } else {
      oldValue = entry->getValue();
    }

    entry->setValue(value);

    return replaced;
  }

  virtual HashMapEntry *createEntry(const K &key, int index, const V &value) {
    HashMapEntry *entry = new HashMapEntry(key, value);
    entry->next = elementData[index];
    elementData[index] = entry;
    return entry;
  }

  virtual HashMapEntry *createHashedEntry(const K &key, int index, int hash) {
    HashMapEntry *entry = new HashMapEntry(key, V(), hash);
    entry->next = elementData[index];
    elementData[index] = entry;
    return entry;
  }

 protected:
  void putAllImpl(const Map<K, V> &map) {
    int capacity = elementCount + map.size();
    if (capacity > threshold) {
      rehash(capacity);
    }

    decaf::lang::Pointer<Iterator<MapEntry<K, V> > > iterator(map.entrySet().iterator());
    while (iterator->hasNext()) {
      MapEntry<K, V> entry = iterator->next();
      this->putImpl(entry.getKey(), entry.getValue());
    }
  }

  HashMapEntry *findKeyEntry(const K &key, int index, int keyHash) const {
    HashMapEntry *entry = elementData[index];
    while (entry != nullptr && (entry->origKeyHash != keyHash || !(key == entry->getKey()))) {
      entry = entry->next;
    }
    return entry;
  }

  void rehash(int capacity) {
    int length = calculateCapacity((capacity == 0 ? 1 : capacity << 1));

    decaf::lang::ArrayPointer<HashMapEntry *> newData(length);
    for (int i = 0; i < elementData.length(); i++) {
      HashMapEntry *entry = elementData[i];
      elementData[i] = nullptr;
      while (entry != nullptr) {
        int index = entry->origKeyHash & (length - 1);
        HashMapEntry *next = entry->next;
        entry->next = newData[index];
        newData[index] = entry;
        entry = next;
      }
    }
    elementData = newData;
    computeThreshold();
  }

  void rehash() { rehash(elementData.length()); }

  // Removes the given entry from the map and deletes it
  void removeEntry(HashMapEntry *entry) {
    int index = entry->origKeyHash & (elementData.length() - 1);
    HashMapEntry *current = elementData[index];
    if (current == entry) {
      elementData[index] = entry->next;
    } else {
      while (current->next != entry) {
        current = current->next;
      }
      current->next = entry->next;
    }
    delete entry;
    ++modCount;
    --elementCount;
  }

  // Removes but doesn't delete the entry in the map with the given key.
  HashMapEntry *removeEntry(const K &key) {
    int index = 0;
    HashMapEntry *current = nullptr;
    HashMapEntry *last = nullptr;

    int hash = hashFunc(key);
    index = hash & (elementData.length() - 1);
    current = elementData[index];
    while (current != nullptr && !(current->origKeyHash == hash && key == current->getKey())) {
      last = current;
      current = current->next;
    }

    if (current == nullptr) {
      return nullptr;
    }

    if (last == nullptr) {
      elementData[index] = current->next;
    } else {
      last->next = current->next;
    }

    ++modCount;
    --elementCount;
    return current;
  }
};
}  // namespace util
}  // namespace decaf

#endif /* _DECAF_UTIL_HASHMAP_H_ */
