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

#ifndef _DECAF_UTIL_ABSTRACTLIST_H_
#define _DECAF_UTIL_ABSTRACTLIST_H_

#include <decaf/lang/Iterable.h>
#include <decaf/lang/exceptions/IllegalArgumentException.h>
#include <decaf/lang/exceptions/NullPointerException.h>
#include <decaf/lang/exceptions/UnsupportedOperationException.h>
#include <decaf/util/AbstractCollection.h>
#include <decaf/util/ConcurrentModificationException.h>
#include <decaf/util/Config.h>
#include <decaf/util/Iterator.h>
#include <decaf/util/List.h>
#include <decaf/util/NoSuchElementException.h>
#include <memory>

namespace decaf {
namespace util {

/**
 * This class provides a skeletal implementation of the List  interface to minimize
 * the effort required to implement this interface backed by a "random access" data
 * store (such as an array). For sequential access data (such as a linked list),
 * AbstractSequentialList should be used in preference to this class.
 *
 * To implement an unmodifiable list, the programmer needs only to extend this class
 * and provide implementations for the get(int) and size() methods.
 *
 * To implement a modifiable list, the programmer must additionally override the
 * set(int, E) method (which otherwise throws an UnsupportedOperationException). If
 * the list is variable-size the programmer must additionally override the add(int, E)
 * and remove(int) methods.
 *
 * The programmer should generally provide a void (no argument) and collection
 * constructor, as per the recommendation in the Collection interface specification.
 *
 * Unlike the other abstract collection implementations, the programmer does not have
 * to provide an iterator implementation; the iterator and list iterator are implemented
 * by this class, on top of the "random access" methods: get(int), set(int, E),
 * add(int, E) and remove(int).
 *
 * The documentation for each non-abstract method in this class describes its
 * implementation in detail. Each of these methods may be overridden if the collection
 * being implemented admits a more efficient implementation.
 *
 * @since 1.0
 */
template <typename E>
class AbstractList : public decaf::util::List<E>, public decaf::util::AbstractCollection<E> {
 protected:
  int modCount;

 private:
  class SimpleListIterator : public ListIterator<E> {
   protected:
    AbstractList<E> *parent;
    int numLeft;
    int expectedModCount;
    int lastPosition;

   public:
    SimpleListIterator(const SimpleListIterator &) = delete;
    SimpleListIterator operator=(const SimpleListIterator &) = delete;

    SimpleListIterator(AbstractList<E> *parent_, int start) : ListIterator<E>(), parent(parent_), numLeft(0), expectedModCount(0), lastPosition(-1) {
      if (parent == nullptr) {
        throw decaf::lang::exceptions::NullPointerException(__FILE__, __LINE__, "List Iterator constructed with NULL parent");
      }

      if (start < 0 || start > parent->size()) {
        throw decaf::lang::exceptions::IndexOutOfBoundsException(__FILE__, __LINE__, "start index passed was negative or greater than size()");
      }

      this->numLeft = parent->size() - start;
      this->parent = parent_;
      this->expectedModCount = parent->modCount;
    }

    virtual ~SimpleListIterator() = default;

    bool hasNext() const override { return this->numLeft > 0; }

    E next() override {
      if (this->expectedModCount != this->parent->modCount) {
        throw ConcurrentModificationException(__FILE__, __LINE__, "Concurrent Modification of Parent List detected.");
      }

      try {
        int index = this->parent->size() - this->numLeft;
        E result = this->parent->get(index);
        this->lastPosition = index;
        --(this->numLeft);

        return result;
      } catch (decaf::lang::exceptions::IndexOutOfBoundsException &) {
        throw decaf::util::NoSuchElementException(__FILE__, __LINE__, "Next called without a next element to process.");
      }
    }

    void remove() override {
      if (this->lastPosition == -1) {
        throw decaf::lang::exceptions::IllegalStateException(__FILE__, __LINE__, "Remove called before next() was called.");
      }

      if (this->expectedModCount != this->parent->modCount) {
        throw ConcurrentModificationException(__FILE__, __LINE__, "Concurrent Modification of Parent List detected.");
      }

      try {
        if (this->lastPosition == this->parent->size() - this->numLeft) {
          --(this->numLeft);  // we're removing after a call to previous()
        }

        this->parent->removeAt(lastPosition);

      } catch (decaf::lang::exceptions::IndexOutOfBoundsException &) {
        throw ConcurrentModificationException(__FILE__, __LINE__, "Concurrent Modification detected.");
      }

      this->expectedModCount = this->parent->modCount;
      this->lastPosition = -1;
    }

    void add(const E &value) override {
      if (this->expectedModCount != this->parent->modCount) {
        throw ConcurrentModificationException(__FILE__, __LINE__, "Concurrent Modification of Parent List detected.");
      }

      try {
        this->parent->add(this->parent->size() - this->numLeft, value);
        this->expectedModCount = this->parent->modCount;
        this->lastPosition = -1;
      } catch (decaf::lang::exceptions::IndexOutOfBoundsException &) {
        throw decaf::util::NoSuchElementException(__FILE__, __LINE__, "Add called without a next element to process.");
      }
    }

    bool hasPrevious() const override { return this->numLeft < this->parent->size(); }

    int nextIndex() const override { return this->parent->size() - this->numLeft; }

    E previous() override {
      if (this->expectedModCount != this->parent->modCount) {
        throw ConcurrentModificationException(__FILE__, __LINE__, "Concurrent Modification detected.");
      }

      try {
        int index = this->parent->size() - this->numLeft - 1;
        E result = this->parent->get(index);
        ++(this->numLeft);
        this->lastPosition = index;

        return result;
      } catch (decaf::lang::exceptions::IndexOutOfBoundsException &) {
        throw decaf::util::NoSuchElementException(__FILE__, __LINE__, "No previous element exists.");
      }
    }

    int previousIndex() const override { return this->parent->size() - this->numLeft - 1; }

    void set(const E &value) override {
      if (this->expectedModCount != this->parent->modCount) {
        throw ConcurrentModificationException(__FILE__, __LINE__, "Concurrent Modification detected.");
      }

      try {
        this->parent->set(this->lastPosition, value);
      } catch (decaf::lang::exceptions::IndexOutOfBoundsException &) {
        throw decaf::lang::exceptions::IllegalStateException();
      }
    }
  };

  class ConstSimpleListIterator : public ListIterator<E> {
   protected:
    const AbstractList<E> *parent;
    int numLeft;
    int expectedModCount;
    int lastPosition;

   public:
    ConstSimpleListIterator(const ConstSimpleListIterator &) = delete;
    ConstSimpleListIterator operator=(const ConstSimpleListIterator &) = delete;

    ConstSimpleListIterator(const AbstractList<E> *parent_, int start)
        : ListIterator<E>(), parent(parent_), numLeft(0), expectedModCount(0), lastPosition(-1) {
      if (parent == nullptr) {
        throw decaf::lang::exceptions::NullPointerException(__FILE__, __LINE__, "List Iterator constructed with NULL parent");
      }

      if (start < 0 || start > parent->size()) {
        throw decaf::lang::exceptions::IndexOutOfBoundsException(__FILE__, __LINE__, "start index passed was negative or greater than size()");
      }

      this->numLeft = parent->size() - start;
      this->parent = parent_;
      this->expectedModCount = parent->modCount;
    }

    virtual ~ConstSimpleListIterator() = default;

    bool hasNext() const override { return this->numLeft > 0; }

    E next() override {
      if (this->expectedModCount != this->parent->modCount) {
        throw ConcurrentModificationException(__FILE__, __LINE__, "Concurrent Modification of Parent List detected.");
      }

      try {
        int index = this->parent->size() - this->numLeft;
        E result = this->parent->get(index);
        this->lastPosition = index;
        --(this->numLeft);

        return result;
      } catch (decaf::lang::exceptions::IndexOutOfBoundsException &) {
        throw decaf::util::NoSuchElementException(__FILE__, __LINE__, "Next called without a next element to process.");
      }
    }

    void remove() override {
      throw lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "AbstractList::Iterator::remove - Const Iterator.");
    }

    void add(const E &value DECAF_UNUSED) override {
      DECAF_UNUSED_VAR(value);
      throw lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "AbstractList::ListIterator::radd - Const Iterator.");
    }

    bool hasPrevious() const override { return this->numLeft < this->parent->size(); }

    int nextIndex() const override { return this->parent->size() - this->numLeft; }

    E previous() override {
      if (this->expectedModCount != this->parent->modCount) {
        throw ConcurrentModificationException(__FILE__, __LINE__, "Concurrent Modification detected.");
      }

      try {
        int index = this->parent->size() - this->numLeft - 1;
        E result = this->parent->get(index);
        ++(this->numLeft);
        this->lastPosition = index;

        return result;
      } catch (decaf::lang::exceptions::IndexOutOfBoundsException &) {
        throw decaf::util::NoSuchElementException(__FILE__, __LINE__, "No previous element exists.");
      }
    }

    int previousIndex() const override { return this->parent->size() - this->numLeft - 1; }

    void set(const E &value DECAF_UNUSED) override {
      DECAF_UNUSED_VAR(value);
      throw lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "AbstractList::ListIterator::set - Const Iterator.");
    }
  };

 public:
  AbstractList() : modCount(0) {}

  virtual ~AbstractList() = default;

  Iterator<E> *iterator() override { return new SimpleListIterator(this, 0); }
  Iterator<E> *iterator() const override { return new ConstSimpleListIterator(this, 0); }

  ListIterator<E> *listIterator() override { return new SimpleListIterator(this, 0); }
  ListIterator<E> *listIterator() const override { return new ConstSimpleListIterator(this, 0); }

  ListIterator<E> *listIterator(int index) override { return new SimpleListIterator(this, index); }
  ListIterator<E> *listIterator(int index) const override { return new ConstSimpleListIterator(this, index); }

  void clear() override { this->removeRange(0, this->size()); }

  bool add(const E &value) override {
    this->add(this->size(), value);
    return true;
  }

  void add(int index DECAF_UNUSED, const E &element DECAF_UNUSED) override {
    DECAF_UNUSED_VAR(index);
    DECAF_UNUSED_VAR(element);
    throw decaf::lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Abstract list does not implement the add method.");
  }

  // Use this method since our own addAll will hide the base class version.
  using AbstractCollection<E>::addAll;

  bool addAll(int index, const Collection<E> &source) override {
    std::unique_ptr<decaf::util::Iterator<E> > iter(source.iterator());
    while (iter->hasNext()) {
      this->add(index++, iter->next());
    }

    return !source.isEmpty();
  }

  E removeAt(int index DECAF_UNUSED) override {
    DECAF_UNUSED_VAR(index);
    throw decaf::lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Abstract list does not implement the removeAt method.");
  }

  E set(int index DECAF_UNUSED, const E &element DECAF_UNUSED) override {
    DECAF_UNUSED_VAR(index);
    DECAF_UNUSED_VAR(element);
    throw decaf::lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Abstract list does not implement the set method.");
  }

  int indexOf(const E &value) const override {
    std::unique_ptr<decaf::util::ListIterator<E> > iter(this->listIterator());

    while (iter->hasNext()) {
      if (value == iter->next()) {
        return iter->previousIndex();
      }
    }

    return -1;
  }

  int lastIndexOf(const E &value) const override {
    std::unique_ptr<decaf::util::ListIterator<E> > iter(this->listIterator(this->size()));

    while (iter->hasPrevious()) {
      if (value == iter->previous()) {
        return iter->nextIndex();
      }
    }

    return -1;
  }

 protected:
  void removeRange(int start, int end) {
    std::unique_ptr<decaf::util::Iterator<E> > iter(this->listIterator(start));
    for (int i = start; i < end; i++) {
      iter->next();
      iter->remove();
    }
  }
};
}  // namespace util
}  // namespace decaf

#endif /* _DECAF_UTIL_ABSTRACTLIST_H_ */
