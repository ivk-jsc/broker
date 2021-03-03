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

#ifndef _DECAF_INTERNAL_UTIL_CONCURRENT_UNIX_PLATFORMDEFS_H_
#define _DECAF_INTERNAL_UTIL_CONCURRENT_UNIX_PLATFORMDEFS_H_

#include <decaf/util/Config.h>

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_SCHED_H
#include <sched.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif

namespace decaf {
namespace internal {
namespace util {
namespace concurrent {

typedef void *PLATFORM_THREAD_ENTRY_ARG;
#define PLATFORM_THREAD_RETURN() return 0;
#define PLATFORM_THREAD_CALLBACK_TYPE void *
#define PLATFORM_MIN_STACK_SIZE 0x8000
#define PLATFORM_CALLING_CONV

typedef pthread_t decaf_thread_t;
typedef pthread_key_t decaf_tls_key;
struct decaf_condition_t {
  pthread_cond_t *value;
  pthread_condattr_t *attr;
};
typedef pthread_mutex_t *decaf_mutex_t;
typedef pthread_rwlock_t *decaf_rwmutex_t;
}  // namespace concurrent
}  // namespace util
}  // namespace internal
}  // namespace decaf

#endif /* _DECAF_INTERNAL_UTIL_CONCURRENT_UNIX_PLATFORMDEFS_H_ */
