// Provides a C++11 implementation of a multi-producer, multi-consumer lock-free queue.
// An overview, including benchmark results, is provided here:
//     http://moodycamel.com/blog/2014/a-fast-general-purpose-lock-free-queue-for-c++
// The full design is also described in excruciating detail at:
//    http://moodycamel.com/blog/2014/detailed-design-of-a-lock-free-queue

// Simplified BSD license:
// Copyright (c) 2013-2016, Cameron Desrochers.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
// - Redistributions of source code must retain the above copyright notice, this list of
// conditions and the following disclaimer.
// - Redistributions in binary form must reproduce the above copyright notice, this list of
// conditions and the following disclaimer in the documentation and/or other materials
// provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
// OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
#pragma once

//#define MCDBGQ_TRACKMEM 1
//#define MCDBGQ_NOLOCKFREE_FREELIST 1
//#define MCDBGQ_USEDEBUGFREELIST 1
//#define MCDBGQ_NOLOCKFREE_IMPLICITPRODBLOCKINDEX 1
//#define MCDBGQ_NOLOCKFREE_IMPLICITPRODHASH 1

#if defined(_WIN32) || defined(__WINDOWS__) || defined(__WIN32__)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
namespace moodycamel { namespace debug {
	struct DebugMutex {
		DebugMutex() { InitializeCriticalSectionAndSpinCount(&cs, 0x400); }
		~DebugMutex() { DeleteCriticalSection(&cs); }
		
		void lock() { EnterCriticalSection(&cs); }
		void unlock() { LeaveCriticalSection(&cs); }
		
	private:
		CRITICAL_SECTION cs;
	};
} }
#else
#include <mutex>
namespace moodycamel { namespace debug {
	struct DebugMutex {
		void lock() { m.lock(); }
		void unlock() { m.unlock(); }
		
	private:
		std::mutex m;
	};
} }
#define
#endif

namespace moodycamel { namespace debug {
	struct DebugLock {
		explicit DebugLock(DebugMutex& mutex)
			: mutex(mutex)
		{
			mutex.lock();
		}
		
		~DebugLock()
		{
			mutex.unlock();
		}
		
	private:
		DebugMutex& mutex;
	};
	
	
	template<typename N>
	struct DebugFreeList {
		DebugFreeList() : head(nullptr) { }
		DebugFreeList(DebugFreeList&& other) : head(other.head) { other.head = nullptr; }
		void swap(DebugFreeList& other) { std::swap(head, other.head); }
		
		inline void add(N* node)
		{
			DebugLock lock(mutex);
			node->freeListNext = head;
			head = node;
		}
		
		inline N* try_get()
		{
			DebugLock lock(mutex);
			if (head == nullptr) {
				return nullptr;
			}
			
			auto prevHead = head;
			head = head->freeListNext;
			return prevHead;
		}
		
		N* head_unsafe() const { return head; }
		
	private:
		N* head;
		DebugMutex mutex;
	};
} }
