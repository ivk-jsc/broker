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

#ifndef _DECAF_UTIL_ABSTRACTSEQUENTIALLIST_H_
#define _DECAF_UTIL_ABSTRACTSEQUENTIALLIST_H_

#include <decaf/lang/Iterable.h>
#include <decaf/lang/exceptions/IllegalArgumentException.h>
#include <decaf/lang/exceptions/NullPointerException.h>
#include <decaf/lang/exceptions/UnsupportedOperationException.h>
#include <decaf/util/AbstractList.h>
#include <decaf/util/Config.h>
#include <decaf/util/Iterator.h>
#include <memory>

namespace decaf {
namespace util {

/**
 * This class provides a skeletal implementation of the List  interface to minimize
 * the effort required to implement this interface backed by a "sequential access"
 * data store (such as a linked list). For random access data (such as an array),
 * AbstractList should be used in preference to this class.
 *
 * This class is the opposite of the AbstractList class in the sense that it implements
 * the "random access" methods (get(int index), set(int index, E element), add(int index,
 * E element) and remove(int index)) on top of the list's list iterator, instead of the
 * other way around.
 *
 * To implement a list the programmer needs only to extend this class and provide
 * implementations for the listIterator and size methods. For an unmodifiable list, the
 * programmer need only implement the list iterator's hasNext, next, hasPrevious,
 * previous and index methods.
 *
 * For a modifiable list the programmer should additionally implement the list iterator's
 * set method. For a variable-size list the programmer should additionally implement the
 * list iterator's remove and add methods.
 *
 * The programmer should generally provide a void (no argument) and collection constructor,
 * as per the recommendation in the Collection interface specification.
 *
 * @since 1.0
 */
template <typename E>
class AbstractSequentialList : public decaf::util::AbstractList<E> {
 public:
  using AbstractList<E>::add;
  using AbstractList<E>::addAll;

 public:
  virtual ~AbstractSequentialList() = default;

  Iterator<E> *iterator() override { return this->listIterator(0); }
  Iterator<E> *iterator() const override { return this->listIterator(0); }

  ListIterator<E> *listIterator() override { return this->listIterator(0); }
  ListIterator<E> *listIterator() const override { return this->listIterator(0); }

  ListIterator<E> *listIterator(int index DECAF_UNUSED) override {
    DECAF_UNUSED_VAR(index);
    throw decaf::lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Abstract sequential list does not implement the listIterator.");
  }

  ListIterator<E> *listIterator(int index DECAF_UNUSED) const override {
    DECAF_UNUSED_VAR(index);
    throw decaf::lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Abstract sequential list does not implement the listIterator.");
  }

  /**
   * {@inheritDoc}
   *
   * This implementation first gets a list iterator pointing to the indexed
   * element (with listIterator(index)). Then, it gets the element using
   * ListIterator.next and returns it.
   */
  E get(int index) const override {
    try {
      std::unique_ptr<ListIterator<E> > iter(this->listIterator(index));
      return iter->next();
    } catch (decaf::util::NoSuchElementException &) {
      throw decaf::lang::exceptions::IndexOutOfBoundsException(__FILE__, __LINE__, "get called with invalid index.");
    }
  }

  /**
   * {@inheritDoc}
   *
   * This implementation first gets a list iterator pointing to the indexed element
   * (with listIterator(index)). Then, it gets the current element using
   * ListIterator.next and replaces it with ListIterator.set.
   */
  E set(int index, const E &element) override {
    try {
      std::unique_ptr<ListIterator<E> > iter(this->listIterator(index));
      E result = iter->next();
      iter->set(element);
      return result;
    } catch (decaf::util::NoSuchElementException &) {
      throw decaf::lang::exceptions::IndexOutOfBoundsException(__FILE__, __LINE__, "set called with invalid index.");
    }
  }

  /**
   * {@inheritDoc}
   *
   * This implementation first gets a list iterator pointing to the indexed element
   * (with listIterator(index)). Then, it inserts the specified element with
   * ListIterator.add.
   */
  void add(int index, const E &element) override {
    try {
      std::unique_ptr<ListIterator<E> > iter(this->listIterator(index));
      iter->add(element);
    } catch (decaf::util::NoSuchElementException &) {
      throw decaf::lang::exceptions::IndexOutOfBoundsException(__FILE__, __LINE__, "add called with invalid index.");
    }
  }

  /**
   * {@inheritDoc}
   *
   * This implementation gets an iterator over the specified collection and a list
   * iterator over this list pointing to the indexed element (with listIterator(index)).
   * Then, it iterates over the specified collection, inserting the elements obtained
   * from the iterator into this list, one at a time, using ListIterator.add
   * (to skip over the added element).
   */
  bool addAll(int index, const Collection<E> &source) override {
    std::unique_ptr<ListIterator<E> > iter(this->listIterator(index));
    std::unique_ptr<Iterator<E> > srcIter(source.iterator());
    int next = iter->nextIndex();
    while (srcIter->hasNext()) {
      iter->add(srcIter->next());
    }
    return next != iter->nextIndex();
  }

  /**
   * {@inheritDoc}
   *
   * This implementation first gets a list iterator pointing to the indexed element
   * (with listIterator(index)). Then, it removes the element with ListIterator.remove.
   */
  E removeAt(int index) override {
    try {
      std::unique_ptr<ListIterator<E> > iter(this->listIterator(index));
      E result = iter->next();
      iter->remove();
      return result;
    } catch (decaf::util::NoSuchElementException &) {
      throw decaf::lang::exceptions::IndexOutOfBoundsException(__FILE__, __LINE__, "set called with invalid index.");
    }
  }
};
}  // namespace util
}  // namespace decaf

#endif /* _DECAF_UTIL_ABSTRACTSEQUENTIALLIST_H_ */
