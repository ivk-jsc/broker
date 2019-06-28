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

#ifndef _DECAF_UTIL_LINKEDLIST_H_
#define _DECAF_UTIL_LINKEDLIST_H_

#include <decaf/lang/Integer.h>
#include <decaf/lang/System.h>
#include <decaf/lang/exceptions/IndexOutOfBoundsException.h>
#include <decaf/lang/exceptions/UnsupportedOperationException.h>
#include <decaf/util/AbstractSequentialList.h>
#include <decaf/util/ArrayList.h>
#include <decaf/util/Config.h>
#include <decaf/util/Deque.h>
#include <decaf/util/Iterator.h>
#include <decaf/util/ListIterator.h>
#include <decaf/util/NoSuchElementException.h>
#include <list>
#include <memory>

namespace decaf {
namespace util {

using decaf::lang::System;

/**
 * A complete implementation of the List interface using a doubly linked list data structure.
 *
 * This class also implements the Deque interface providing a common interface for additions
 * into the list at the front and end as well as allowing insertions anywhere in between.
 * This class can be used then to implement other data structures such as Stacks, Queue's or
 * double ended Queue's.
 *
 * The operations on this List object that index a particular element involve iterating over
 * the links of the list from beginning to end, starting from whichever end is closer to the
 * location the operation is to be performed on.
 *
 * @since 1.0
 */
template <typename E>
class LinkedList : public AbstractSequentialList<E>, public Deque<E> {
 private:
  template <typename U>
  class ListNode {
   public:
    U value;
    ListNode<U> *prev;
    ListNode<U> *next;

   private:
    ListNode(const ListNode &);
    ListNode &operator=(const ListNode &);

   public:
    ListNode() : value(), prev(nullptr), next(nullptr) {}

    ListNode(const U &value) : value(value), prev(nullptr), next(nullptr) {}

    ListNode(ListNode<U> *prev, ListNode<U> *next, const U &value) : value(value), prev(prev), next(next) {}
  };

 private:
  int listSize;
  ListNode<E> head;
  ListNode<E> tail;

 public:
  LinkedList() : AbstractSequentialList<E>(), listSize(0), head(), tail() {
    this->head.next = &this->tail;
    this->tail.prev = &this->head;
  }

  LinkedList(const LinkedList<E> &list) : AbstractSequentialList<E>(), listSize(0), head(), tail() {
    this->head.next = &this->tail;
    this->tail.prev = &this->head;

    this->addAllAtLocation(0, list);
  }

  LinkedList(const Collection<E> &collection) : AbstractSequentialList<E>(), listSize(0), head(), tail() {
    this->head.next = &this->tail;
    this->tail.prev = &this->head;

    this->addAllAtLocation(0, collection);
  }

  virtual ~LinkedList() {
    try {
      this->purgeList();
    } catch (...) {
    }
  }

 public:
  LinkedList<E> &operator=(const LinkedList<E> &list) {
    this->clear();
    this->addAllAtLocation(0, list);
    return *this;
  }

  LinkedList<E> &operator=(const Collection<E> &collection) {
    this->clear();
    this->addAllAtLocation(0, collection);
    return *this;
  }

  bool operator==(const LinkedList<E> &other) const { return this->equals(other); }

  bool operator!=(const LinkedList<E> &other) const { return !this->equals(other); }

 public:
  E get(int index) const override {
    if (index < 0 || index >= this->listSize) {
      throw decaf::lang::exceptions::IndexOutOfBoundsException(__FILE__, __LINE__, "Index given is outside bounds of this list {%d}", index);
    }

    const ListNode<E> *location = nullptr;

    if (index < this->listSize / 2) {
      location = &this->head;
      for (int i = 0; i <= index; ++i) {
        location = location->next;
      }
    } else {
      location = &this->tail;
      for (int i = this->listSize; i > index; --i) {
        location = location->prev;
      }
    }

    return location->value;
  }

  E set(int index, const E &element) override {
    if (index < 0 || index >= this->listSize) {
      throw decaf::lang::exceptions::IndexOutOfBoundsException(__FILE__, __LINE__, "Index given is outside bounds of this list {%d}", index);
    }

    ListNode<E> *location = nullptr;

    if (index < this->listSize / 2) {
      location = &this->head;
      for (int i = 0; i <= index; ++i) {
        location = location->next;
      }
    } else {
      location = &this->tail;
      for (int i = this->listSize; i > index; --i) {
        location = location->prev;
      }
    }

    E oldValue = location->value;
    location->value = element;

    return oldValue;
  }

  bool add(const E &value) override {
    this->addToEnd(value);
    return true;
  }

  void add(int index, const E &value) override {
    if (index < 0 || index > this->listSize) {
      throw decaf::lang::exceptions::IndexOutOfBoundsException(__FILE__, __LINE__, "Index given is outside bounds of this list {%d}", index);
    }

    this->addAtLocation(index, value);
  }

  bool addAll(const Collection<E> &collection) override { return this->addAllAtLocation(this->listSize, collection); }

  bool addAll(int index, const Collection<E> &collection) override { return this->addAllAtLocation(index, collection); }

  void copy(const Collection<E> &collection) override {
    this->clear();
    this->addAllAtLocation(0, collection);
  }

  bool remove(const E &value) override { return this->removeFirstOccurrence(value); }

  bool isEmpty() const override { return this->listSize == 0; }

  int size() const override { return this->listSize; }

  void clear() override {
    this->purgeList();
    this->head.next = &this->tail;
    this->tail.prev = &this->head;
    this->listSize = 0;
    ++(AbstractList<E>::modCount);
  }

  bool contains(const E &value) const override { return this->indexOf(value) != -1; }

  int indexOf(const E &value) const override {
    if (this->listSize == 0) {
      return -1;
    }

    const ListNode<E> *location = this->head.next;

    for (int i = 0; location != &this->tail; ++i, location = location->next) {
      if (location->value == value) {
        return i;
      }
    }

    return -1;
  }

  int lastIndexOf(const E &value) const override {
    if (this->listSize == 0) {
      return -1;
    }

    const ListNode<E> *location = this->tail.prev;

    for (int i = this->listSize - 1; location != &this->head; --i, location = location->prev) {
      if (location->value == value) {
        return i;
      }
    }

    return -1;
  }

  std::vector<E> toArray() const override {
    std::vector<E> result;
    result.reserve(this->listSize);

    const ListNode<E> *current = this->head.next;

    while (current != &this->tail) {
      result.push_back(current->value);
      current = current->next;
    }

    return result;
  }

 public:  // Deque interface implementation.
  bool offer(const E &value) override {
    this->addLast(value);
    return true;
  }

  bool poll(E &result) override {
    if (this->listSize == 0) {
      return false;
    }

    result = this->head.next->value;
    this->removeAtFront();
    return true;
  }

  E remove() override { return this->removeAtFront(); }

  bool peek(E &result) const override {
    if (this->listSize == 0) {
      return false;
    }

    result = this->head.next->value;
    return true;
  }

  E element() const override {
    if (this->listSize == 0) {
      throw decaf::util::NoSuchElementException(__FILE__, __LINE__, "The list is Empty");
    }

    return this->head.next->value;
  }

  void addFirst(const E &value) override { this->addToFront(value); }

  void addLast(const E &value) override { this->addToEnd(value); }

  E &getFirst() override {
    if (this->listSize == 0) {
      throw decaf::util::NoSuchElementException(__FILE__, __LINE__, "The list is Empty");
    }

    return this->head.next->value;
  }

  const E &getFirst() const override {
    if (this->listSize == 0) {
      throw decaf::util::NoSuchElementException(__FILE__, __LINE__, "The list is Empty");
    }

    return this->head.next->value;
  }

  E &getLast() override {
    if (this->listSize == 0) {
      throw decaf::util::NoSuchElementException(__FILE__, __LINE__, "The list is Empty");
    }

    return this->tail.prev->value;
  }

  const E &getLast() const override {
    if (this->listSize == 0) {
      throw decaf::util::NoSuchElementException(__FILE__, __LINE__, "The list is Empty");
    }

    return this->tail.prev->value;
  }

  bool offerFirst(const E &element) override {
    this->addToFront(element);
    return true;
  }

  bool offerLast(const E &element) override {
    this->addToEnd(element);
    return true;
  }

  E removeFirst() override { return this->removeAtFront(); }

  E removeLast() override { return this->removeAtEnd(); }

  bool pollFirst(E &result) override {
    if (this->listSize == 0) {
      return false;
    }

    result = this->head.next->value;
    this->removeAtFront();
    return true;
  }

  bool pollLast(E &result) override {
    if (this->listSize == 0) {
      return false;
    }

    result = this->tail.prev->value;
    this->removeAtEnd();
    return true;
  }

  bool peekFirst(E &result) const override {
    if (this->listSize == 0) {
      return false;
    }

    result = this->head.next->value;
    return true;
  }

  bool peekLast(E &result) const override {
    if (this->listSize == 0) {
      return false;
    }

    result = this->tail.prev->value;
    return true;
  }

  E pop() override { return this->removeAtFront(); }

  void push(const E &element) override { this->addToFront(element); }

  bool removeFirstOccurrence(const E &value) override {
    std::unique_ptr<Iterator<E> > iter(this->iterator());
    while (iter->hasNext()) {
      if (iter->next() == value) {
        iter->remove();
        return true;
      }
    }

    return false;
  }

  bool removeLastOccurrence(const E &value) override {
    std::unique_ptr<Iterator<E> > iter(this->descendingIterator());
    while (iter->hasNext()) {
      if (iter->next() == value) {
        iter->remove();
        return true;
      }
    }

    return false;
  }

 private:
  class LinkedListIterator : public ListIterator<E> {
   private:
    mutable LinkedList<E> *list;
    ListNode<E> *current;
    ListNode<E> *lastReturned;
    int index;
    int expectedModCount;

   private:
    LinkedListIterator(const LinkedListIterator &);
    LinkedListIterator operator=(const LinkedListIterator &);

   public:
    LinkedListIterator(LinkedList<E> *list, int index) : ListIterator<E>(), list(list), current(nullptr), lastReturned(nullptr), index(index), expectedModCount(0) {
      if (list == nullptr) {
        throw decaf::lang::exceptions::NullPointerException(__FILE__, __LINE__, "Parent LinkedList pointer was Null.");
      }

      if (index < 0 || index > list->listSize) {
        throw decaf::lang::exceptions::IndexOutOfBoundsException(__FILE__, __LINE__, "Given index {%d} is out of range.", index);
      }

      this->expectedModCount = list->modCount;

      // index starts at -1 to indicate that we are before begin or that the
      // list is empty.  We always want to start out one before so that the call
      // to next moves us onto the element in question;

      if (index < this->list->listSize / 2) {
        this->current = &this->list->head;
        for (this->index = -1; this->index + 1 < index; ++this->index) {
          this->current = this->current->next;
        }
      } else {
        this->current = &this->list->tail;
        for (this->index = this->list->listSize; this->index >= index; --this->index) {
          this->current = this->current->prev;
        }
      }
    }

    virtual ~LinkedListIterator() {}

    E next() override {
      if (this->expectedModCount != this->list->modCount) {
        throw ConcurrentModificationException(__FILE__, __LINE__, "List modified outside of this Iterator.");
      }

      if (this->current->next == &(this->list->tail)) {
        throw NoSuchElementException(__FILE__, __LINE__, "No more elements to return from next()");
      }

      this->current = this->current->next;
      this->lastReturned = this->current;
      ++this->index;

      return this->current->value;
    }

    bool hasNext() const override { return (this->current->next != &this->list->tail); }

    E previous() override {
      if (this->expectedModCount != this->list->modCount) {
        throw ConcurrentModificationException(__FILE__, __LINE__, "List modified outside of this Iterator.");
      }

      if (this->current == &(this->list->head)) {
        throw decaf::lang::exceptions::IllegalStateException(__FILE__, __LINE__, "No previous element, must call next() before calling previous().");
      }

      this->lastReturned = this->current;
      this->current = this->current->prev;
      --this->index;

      return this->lastReturned->value;
    }

    bool hasPrevious() const override { return (this->current != &this->list->head); }

    void remove() override {
      if (this->expectedModCount != this->list->modCount) {
        throw ConcurrentModificationException(__FILE__, __LINE__, "List modified outside of this Iterator.");
      }

      if (this->lastReturned == nullptr) {
        throw lang::exceptions::IllegalStateException(__FILE__, __LINE__, "Invalid State to call remove, must call next() before remove()");
      }

      ListNode<E> *next = this->lastReturned->next;
      ListNode<E> *previous = this->lastReturned->prev;

      next->prev = previous;
      previous->next = next;

      // When iterating in reverse this would not be true
      if (this->current == this->lastReturned) {
        --this->index;
      }
      this->current = previous;

      delete this->lastReturned;
      this->lastReturned = nullptr;

      --this->list->listSize;
      ++this->list->modCount;

      ++this->expectedModCount;
    }

    void add(const E &e) override {
      if (this->expectedModCount != this->list->modCount) {
        throw ConcurrentModificationException(__FILE__, __LINE__, "List modified outside of this Iterator.");
      }

      ListNode<E> *newNode = new ListNode<E>(this->current, this->current->next, e);

      this->current->next->prev = newNode;
      this->current->next = newNode;

      this->current = newNode;
      this->lastReturned = nullptr;

      ++this->index;
      ++this->expectedModCount;
      ++this->list->modCount;
      ++this->list->listSize;
    }

    void set(const E &e) override {
      if (this->expectedModCount != this->list->modCount) {
        throw ConcurrentModificationException(__FILE__, __LINE__, "List modified outside of this Iterator.");
      }

      if (this->lastReturned != nullptr) {
        this->lastReturned->value = e;
      } else {
        throw decaf::lang::exceptions::IllegalStateException(__FILE__, __LINE__, "Iterator next has not been called.");
      }
    }

    int nextIndex() const override { return this->index + 1; }

    int previousIndex() const override { return this->index; }
  };

  class ConstLinkedListIterator : public ListIterator<E> {
   private:
    const LinkedList<E> *list;
    const ListNode<E> *current;
    const ListNode<E> *lastReturned;
    int index;

   private:
    ConstLinkedListIterator(const ConstLinkedListIterator &);
    ConstLinkedListIterator operator=(const ConstLinkedListIterator &);

   public:
    ConstLinkedListIterator(const LinkedList<E> *list, int index) : ListIterator<E>(), list(list), current(nullptr), lastReturned(nullptr), index(index) {
      if (list == nullptr) {
        throw decaf::lang::exceptions::NullPointerException(__FILE__, __LINE__, "Parent LinkedList pointer was Null.");
      }

      if (index < 0 || index > list->listSize) {
        throw decaf::lang::exceptions::IndexOutOfBoundsException(__FILE__, __LINE__, "Given index {%d} is out of range.", index);
      }

      // index starts at -1 to indicate that we are before begin or that the
      // list is empty.  We always want to start out one before so that the call
      // to next moves us onto the element in question;

      if (index < this->list->listSize / 2) {
        this->current = &this->list->head;
        for (this->index = -1; this->index + 1 < index; ++this->index) {
          this->current = this->current->next;
        }
      } else {
        this->current = &this->list->tail;
        for (this->index = this->list->listSize; this->index >= index; --this->index) {
          this->current = this->current->prev;
        }
      }
    }

    virtual ~ConstLinkedListIterator() {}

    E next() override {
      if (this->current->next == &(this->list->tail)) {
        throw NoSuchElementException(__FILE__, __LINE__, "No more elements to return from this ListIterator");
      }

      this->current = this->current->next;
      this->lastReturned = this->current;
      ++this->index;

      return this->current->value;
    }

    bool hasNext() const override { return (this->current->next != &(this->list->tail)); }

    E previous() override {
      if (this->current == &(this->list->head)) {
        throw decaf::lang::exceptions::IllegalStateException(__FILE__, __LINE__, "No previous element, must call next() before calling previous().");
      }

      this->lastReturned = this->current;
      this->current = this->current->prev;
      --this->index;

      return this->lastReturned->value;
    }

    bool hasPrevious() const override { return (this->current != &(this->list->head)); }

    void remove() override { throw lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Cannot write to a const ListIterator."); }

    void add(const E &e DECAF_UNUSED) override {
      DECAF_UNUSED_VAR(e);
      throw lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Cannot write to a const ListIterator.");
    }

    void set(const E &e DECAF_UNUSED) override {
      DECAF_UNUSED_VAR(e);
      throw lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Cannot write to a const ListIterator.");
    }

    int nextIndex() const override { return this->index + 1; }

    int previousIndex() const override { return this->index; }
  };

  class ReverseIterator : public Iterator<E> {
   private:
    LinkedList<E> *list;
    ListNode<E> *current;
    int expectedModCount;
    bool canRemove;

   private:
    ReverseIterator(const ReverseIterator &);
    ReverseIterator operator=(const ReverseIterator &);

   public:
    ReverseIterator(LinkedList<E> *list) : Iterator<E>(), list(list), current(nullptr), expectedModCount(0), canRemove(false) {
      if (list == nullptr) {
        throw decaf::lang::exceptions::NullPointerException(__FILE__, __LINE__, "Parent LinkedList pointer was Null.");
      }

      this->expectedModCount = this->list->modCount;
      this->current = &list->tail;
    }

    virtual ~ReverseIterator() {}

    bool hasNext() const override { return this->current->prev != &(this->list->head); }

    E next() override {
      if (this->expectedModCount != this->list->modCount) {
        throw ConcurrentModificationException(__FILE__, __LINE__, "List modified outside of this Iterator.");
      }

      if (this->current->prev == &(this->list->head)) {
        throw NoSuchElementException(__FILE__, __LINE__, "No more elements to return from next()");
      }

      this->current = this->current->prev;
      this->canRemove = true;

      return this->current->value;
    }

    void remove() override {
      if (this->expectedModCount != this->list->modCount) {
        throw ConcurrentModificationException(__FILE__, __LINE__, "List modified outside of this Iterator.");
      }

      if (!this->canRemove) {
        throw lang::exceptions::IllegalStateException(__FILE__, __LINE__, "Invalid State to call remove, must call next() before remove()");
      }

      ListNode<E> *next = this->current->prev;
      ListNode<E> *prev = this->current->next;

      next->next = prev;
      prev->prev = next;

      delete this->current;

      this->current = prev;

      --this->list->listSize;
      ++this->list->modCount;
      ++this->expectedModCount;
      this->canRemove = false;
    }
  };

  class ConstReverseIterator : public Iterator<E> {
   private:
    const LinkedList<E> *list;
    const ListNode<E> *current;

   private:
    ConstReverseIterator(const ConstReverseIterator &);
    ConstReverseIterator operator=(const ConstReverseIterator &);

   public:
    ConstReverseIterator(const LinkedList<E> *list) : Iterator<E>(), list(list), current(nullptr) {
      if (list == nullptr) {
        throw decaf::lang::exceptions::NullPointerException(__FILE__, __LINE__, "Parent LinkedList pointer was Null.");
      }

      this->current = &list->tail;
    }

    virtual ~ConstReverseIterator() {}

    bool hasNext() const override { return this->current->prev != &(this->list->head); }

    E next() override {
      if (this->current->prev == &(this->list->head)) {
        throw NoSuchElementException(__FILE__, __LINE__, "No more elements to return from next()");
      }

      this->current = this->current->prev;

      return this->current->value;
    }

    void remove() override { throw lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "Cannot write to a const Iterator."); }
  };

 public:
  using AbstractSequentialList<E>::listIterator;

  ListIterator<E> *listIterator(int index) override { return new LinkedListIterator(this, index); }
  ListIterator<E> *listIterator(int index) const override { return new ConstLinkedListIterator(this, index); }

  Iterator<E> *descendingIterator() override { return new ReverseIterator(this); }
  Iterator<E> *descendingIterator() const override { return new ConstReverseIterator(this); }

 private:
  E removeAtFront() {
    if (this->head.next == &this->tail) {
      throw NoSuchElementException(__FILE__, __LINE__, "The Collection is empty.");
    }

    ListNode<E> *oldNode = this->head.next;
    E result = oldNode->value;

    this->head.next = oldNode->next;
    this->head.next->prev = &this->head;

    delete oldNode;

    --this->listSize;
    ++AbstractList<E>::modCount;

    return result;
  }

  E removeAtEnd() {
    if (this->head.next == &this->tail) {
      throw NoSuchElementException(__FILE__, __LINE__, "The Collection is empty.");
    }

    ListNode<E> *oldNode = this->tail.prev;
    E result = oldNode->value;

    this->tail.prev = oldNode->prev;
    this->tail.prev->next = &this->tail;

    delete oldNode;

    --this->listSize;
    ++(AbstractList<E>::modCount);

    return result;
  }

  void addToFront(const E &value) {
    ListNode<E> *newHead = new ListNode<E>(&this->head, this->head.next, value);

    (this->head.next)->prev = newHead;
    this->head.next = newHead;

    ++this->listSize;
    ++(AbstractList<E>::modCount);
  }

  void addToEnd(const E &value) {
    ListNode<E> *newTail = new ListNode<E>(this->tail.prev, &this->tail, value);

    (this->tail.prev)->next = newTail;
    this->tail.prev = newTail;

    ++this->listSize;
    ++(AbstractList<E>::modCount);
  }

  void addAtLocation(int index, const E &value) {
    ListNode<E> *location = nullptr;

    if (index <= this->listSize / 2) {
      location = this->head.next;
      for (int i = 0; i < index; ++i) {
        location = location->next;
      }
    } else {
      location = &this->tail;
      for (int i = this->listSize; i > index; --i) {
        location = location->prev;
      }
    }

    ListNode<E> *newNode = new ListNode<E>(location->prev, location, value);

    (location->prev)->next = newNode;
    location->prev = newNode;

    ++this->listSize;
    ++(AbstractList<E>::modCount);
  }

  bool addAllAtLocation(int index, const Collection<E> &collection) {
    if (index < 0 || index > this->listSize) {
      throw decaf::lang::exceptions::IndexOutOfBoundsException(__FILE__, __LINE__, "Index for add is outside bounds of this LinkedList.");
    }

    int csize = collection.size();
    if (csize == 0) {
      return false;
    }

    std::unique_ptr<ArrayList<E> > copy;
    std::unique_ptr<Iterator<E> > iter;

    if (this == &collection) {
      copy.reset(new ArrayList<E>(collection));
      iter.reset(copy->iterator());
    } else {
      iter.reset(collection.iterator());
    }

    ListNode<E> *newNode = nullptr;
    ListNode<E> *previous = nullptr;

    if (index < this->listSize / 2) {
      previous = &this->head;
      for (int i = 0; i < index; ++i) {
        previous = previous->next;
      }
    } else {
      previous = &this->tail;
      for (int i = this->listSize; i >= index; --i) {
        previous = previous->prev;
      }
    }

    while (iter->hasNext()) {
      newNode = new ListNode<E>(previous, previous->next, iter->next());
      previous->next->prev = newNode;
      previous->next = newNode;
      previous = newNode;
    }

    this->listSize += csize;
    ++(AbstractList<E>::modCount);

    return true;
  }

  void purgeList() {
    ListNode<E> *current = this->head.next;
    ListNode<E> *temp = nullptr;
    while (current != &this->tail) {
      temp = current;
      current = current->next;
      delete temp;
    }
  }
};
}  // namespace util
}  // namespace decaf

#endif /* _DECAF_UTIL_LINKEDLIST_H_ */
