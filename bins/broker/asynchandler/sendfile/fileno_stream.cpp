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
 
#include "fileno_stream.h"
#include <cstdio>

#include <cerrno>
#ifdef __APPLE__
#include <ciso646>  // just for __LIBCPP_VERSION

#ifdef _LIBCPP_VERSION
#ifdef private
#error "why on earth is private #defined to something already?!"
#endif
// this is a very bad idea, never do this
#define private public
// no, seriously.
#include <fstream>
#undef private
#undef class
#endif

#endif  // __APPLE__

#include <fstream>

#if defined(__GLIBCXX__) || (defined(__GLIBCPP__) && __GLIBCPP__ >= 20020514)  // GCC >= 3.1.0
#include <ext/stdio_filebuf.h>
#endif
#if defined(__GLIBCXX__)  // GCC >= 3.4.0
#include <ext/stdio_sync_filebuf.h>
#endif

//! Similar to fileno(3), but taking a C++ stream as argument instead of a
//! FILE*.  Note that there is no way for the library to track what you do with
//! the descriptor, so be careful.
//! \return  The integer file descriptor associated with the stream, or -1 if
//!   that stream is invalid.  In the latter case, for the sake of keeping the
//!   code as similar to fileno(3), errno is set to EBADF.
//! \see  The <A HREF="https://www.ginac.de/~kreckel/fileno/">upstream page at
//!   https://www.ginac.de/~kreckel/fileno/</A> of this code provides more
//!   detailed information.
template <typename charT, typename traits>
inline int fileno_hack(const std::basic_ios<charT, traits>& stream) {
// Some C++ runtime libraries shipped with ancient GCC, Sun Pro,
// Sun WS/Forte 5/6, Compaq C++ supported non-standard file descriptor
// access basic_filebuf<>::fd().  Alas, starting from GCC 3.1, the GNU C++
// runtime removes all non-standard std::filebuf methods and provides an
// extension template class __gnu_cxx::stdio_filebuf on all systems where
// that appears to make sense (i.e. at least all Unix systems).  Starting
// from GCC 3.4, there is an __gnu_cxx::stdio_sync_filebuf, in addition.
#if defined(__GLIBCXX__) || defined(__GLIBCPP__)
// OK, stop reading here, because it's getting obscene.  Cross fingers!
#if defined(__GLIBCXX__)  // >= GCC 3.4.0
  // This applies to cin, cout and cerr when not synced with stdio:
  typedef __gnu_cxx::stdio_filebuf<charT, traits> unix_filebuf_t;
  unix_filebuf_t* fbuf = dynamic_cast<unix_filebuf_t*>(stream.rdbuf());
  if (fbuf != nullptr) {
    return fbuf->fd();
  }

  // This applies to filestreams:
  typedef std::basic_filebuf<charT, traits> filebuf_t;
  filebuf_t* bbuf = dynamic_cast<filebuf_t*>(stream.rdbuf());
  if (bbuf != nullptr) {
    // This subclass is only there for accessing the FILE*.  Ouuwww, sucks!
    struct my_filebuf : public std::basic_filebuf<charT, traits> {
      int fd() { return this->_M_file.fd(); }
    };
    return static_cast<my_filebuf*>(bbuf)->fd();
  }

  // This applies to cin, cout and cerr when synced with stdio:
  typedef __gnu_cxx::stdio_sync_filebuf<charT, traits> sync_filebuf_t;
  sync_filebuf_t* sbuf = dynamic_cast<sync_filebuf_t*>(stream.rdbuf());
  if (sbuf != nullptr) {
#if (__GLIBCXX__ < 20040906)  // GCC < 3.4.2
    // This subclass is only there for accessing the FILE*.
    // See GCC PR#14600 and PR#16411.
    struct my_filebuf : public sync_filebuf_t {
      my_filebuf();  // Dummy ctor keeps the compiler happy.
      // Note: stdio_sync_filebuf has a FILE* as its first (but private)
      // member variable.  However, it is derived from basic_streambuf<>
      // and the FILE* is the first non-inherited member variable.
      FILE* c_file() { return *(FILE**)((char*)this + sizeof(std::basic_streambuf<charT, traits>)); }
    };
    return ::fileno(static_cast<my_filebuf*>(sbuf)->c_file());
#else
    return ::fileno(sbuf->file());
#endif
  }
#else                          // GCC < 3.4.0 used __GLIBCPP__
#if (__GLIBCPP__ >= 20020514)  // GCC >= 3.1.0
  // This applies to cin, cout and cerr:
  typedef __gnu_cxx::stdio_filebuf<charT, traits> unix_filebuf_t;
  unix_filebuf_t* buf = dynamic_cast<unix_filebuf_t*>(stream.rdbuf());
  if (buf != nullptr) {
    return buf->fd();
  }

  // This applies to filestreams:
  typedef std::basic_filebuf<charT, traits> filebuf_t;
  filebuf_t* bbuf = dynamic_cast<filebuf_t*>(stream.rdbuf());
  if (bbuf != nullptr) {
    // This subclass is only there for accessing the FILE*.  Ouuwww, sucks!
    struct my_filebuf : public std::basic_filebuf<charT, traits> {
      // Note: _M_file is of type __basic_file<char> which has a
      // FILE* as its first (but private) member variable.
      FILE* c_file() { return *(FILE**)(&this->_M_file); }
    };
    FILE* c_file = static_cast<my_filebuf*>(bbuf)->c_file();
    if (c_file != nullptr) {  // Could be nullptr for failed ifstreams.
      return ::fileno(c_file);
    }
  }
#else                          // GCC 3.0.x
  typedef std::basic_filebuf<charT, traits> filebuf_t;
  filebuf_t* fbuf = dynamic_cast<filebuf_t*>(stream.rdbuf());
  if (fbuf != nullptr) {
    struct my_filebuf : public filebuf_t {
      // Note: basic_filebuf<charT, traits> has a __basic_file<charT>* as
      // its first (but private) member variable.  Since it is derived
      // from basic_streambuf<charT, traits> we can guess its offset.
      // __basic_file<charT> in turn has a FILE* as its first (but
      // private) member variable.  Get it by brute force.  Oh, geez!
      FILE* c_file() {
        std::__basic_file<charT>* ptr_M_file = *(std::__basic_file<charT>**)((char*)this + sizeof(std::basic_streambuf<charT, traits>));
#if _GLIBCPP_BASIC_FILE_INHERITANCE
        // __basic_file<charT> inherits from __basic_file_base<charT>
        return *(FILE**)((char*)ptr_M_file + sizeof(std::__basic_file_base<charT>));
#else
        // __basic_file<charT> is base class, but with vptr.
        return *(FILE**)((char*)ptr_M_file + sizeof(void*));
#endif
      }
    };
    return ::fileno(static_cast<my_filebuf*>(fbuf)->c_file());
  }
#endif
#endif
#elif defined _MSC_VER                           // MSVC compiler version
  typedef std::basic_filebuf<charT, traits> filebuf_t;
  filebuf_t* bbuf = dynamic_cast<filebuf_t*>(stream.rdbuf());
  if (bbuf != nullptr) {
    struct my_filebuf : public std::basic_filebuf<charT, traits> {
#if ((_MSC_VER >= 1900) && (_MSC_VER <= 1920))   // MSVC Toolset Version 14.0-14.20
      FILE* c_file() { return *(FILE**)((char*)this + sizeof(filebuf_t) - sizeof(charT*) - sizeof(charT*) - sizeof(FILE*)); }
#elif ((_MSC_VER >= 1700) && (_MSC_VER < 1900))  // MSVC Toolset Version 11.0, 12.0
      FILE* c_file() { return *(FILE**)((char*)this + sizeof(filebuf_t) - sizeof(FILE*)); }
#else
#error "not implemented for current MSVC version"
#endif
    };
    FILE* c_file = static_cast<my_filebuf*>(bbuf)->c_file();
    if (c_file != nullptr) {
      return ::_fileno(c_file);
    }
  }
#elif defined(_LIBCPP_VERSION)
  auto *fbuf = stream.rdbuf();
  struct my_filebuf : public std::basic_filebuf<charT, traits> {
    FILE *c_file() { return this->__file_; }
  };
  my_filebuf *my_fbuf = static_cast<my_filebuf *>(fbuf);
  if (my_fbuf != nullptr && my_fbuf->c_file() != nullptr) {
    return ::fileno(my_fbuf->c_file());
  }
#else
//#  error "not implemented for current compiler"
#endif
  errno = EBADF;
  return -1;
}

namespace upmq {
namespace broker {

//! 8-Bit character instantiation: fileno(ios).
template <>
int fileno_stream<char>(const std::ios& stream) {
  return fileno_hack(stream);
}

#if !(defined(__GLIBCXX__) || defined(__GLIBCPP__)) || (defined(_GLIBCPP_USE_WCHAR_T) || defined(_GLIBCXX_USE_WCHAR_T))
//! Wide character instantiation: fileno(wios).
template <>
int fileno_stream<wchar_t>(const std::wios& stream) {
  return fileno_hack(stream);
}
#endif

}  // namespace broker
}  // namespace upmq
