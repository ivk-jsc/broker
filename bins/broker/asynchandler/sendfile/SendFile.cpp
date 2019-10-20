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

#include "SendFile.h"
#include "fileno_stream.h"
#include <Poco/Thread.h>

#ifdef _WIN32
#include <Mswsock.h>
#include <Winsock2.h>
#include <io.h>
#else
#ifdef __linux__
#include <sys/sendfile.h>
#endif
#ifdef __APPLE__
#include <sys/uio.h>
#include <sys/types.h>
using sighandler_t = sig_t;
#endif
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <csignal>
#include <limits>
#include <cerrno>
#include <cstring>
#endif

constexpr size_t DEFAULT_PART_SIZE = 1024;

namespace upmq {
namespace broker {

SendFile::SendFile(Poco::Net::Socket &socket, std::fstream &stream, size_t partSize)
    : _sd(socket.impl()->sockfd()),
#ifdef _WIN32
      _fd(reinterpret_cast<file_descr_t>(_get_osfhandle(fileno_stream(stream)))),
#else
      _fd(upmq::broker::fileno_stream(stream)),
#endif
      _partSize((partSize > 0) ? partSize : DEFAULT_PART_SIZE),
      _offset(0),
      _fileSize(getFileSize(stream)),
      _error(0) {
}

#ifdef _WIN32
bool SendFile::operator()() {
  clearError();
  if (TransmitFile(_sd, _fd, 0, static_cast<DWORD>(_partSize), nullptr, nullptr, 0) == TRUE) {
    _offset = _fileSize;
    return true;
  }
  // TODO: ERROR_IO_PENDING or WSA_IO_PENDING
  setError(WSAGetLastError());
  return false;
}

void SendFile::setError(int error) {
  _error = error;
  /*
    LPTSTR pstr = nullptr;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, _error, 0, pstr, 0, NULL);
    if (pstr != nullptr) {
      _errstr.assign(*pstr);
      LocalFree(s);
    } else {
      _errstr.clear();
    }
  */
}
#else  // not _WIN32
bool SendFile::operator()() {
  clearError();
  if (!fileSizeFits_off_t()) {
    setError(EFBIG);  // file too large
    return false;
  }
  while (!isComplete()) {
    if (!sendPart()) {
      setError(errno);
      return false;
    }
  }
  return true;
}

void SendFile::setError(int error) {
  _error = error;
  _errstr.assign(strerror(_error));
}

bool SendFile::isComplete() const { return _offset >= _fileSize; }

#ifdef __USE_LARGEFILE64
typedef __off64_t sendfile_off_t;
#else
typedef off_t sendfile_off_t;
#endif

bool SendFile::fileSizeFits_off_t() const { return _fileSize <= std::numeric_limits<sendfile_off_t>::max(); }

bool SendFile::sendPart() {
  sendfile_off_t offs = static_cast<sendfile_off_t>(_offset);
  std::streamoff remSize = _fileSize - _offset;
  size_t psize = (_partSize < static_cast<size_t>(remSize)) ? _partSize : static_cast<size_t>(remSize);
  off_t sent = 0;
  sighandler_t sigPrev = signal(SIGPIPE, SIG_IGN);
  while (sent == 0) {
    errno = 0;
#ifdef __USE_LARGEFILE64
    sent = sendfile64(_sd, _fd, &offs, psize);
#else
#ifdef __linux__
    sent = sendfile(_sd, _fd, &offs, psize);
#elif defined(__APPLE__)
    sent = static_cast<off_t>(psize);
    int result = sendfile(_fd, _sd, offs, &sent, nullptr, 0);
    if (result < 0) {
      sent = -1;
    }
#endif
#endif
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      sent = 0;
      Poco::Thread::yield();
    }
  }
  signal(SIGPIPE, sigPrev != SIG_ERR ? sigPrev : SIG_DFL);
  if (sent < 0) {
    return false;
  }
  _offset += sent;
  return true;
}
#endif  // not _WIN32

void SendFile::clearError() {
  _error = 0;
  _errstr.clear();
}

// static
std::streamoff SendFile::getFileSize(std::fstream &stream) {
  auto origpos = stream.tellg();
  stream.seekg(0, std::ios_base::end);
  std::streamoff size = stream.tellg();
  if ((std::streamoff)origpos != -1) {
    stream.seekg(origpos);
  } else {
    stream.seekg(0, std::ios_base::beg);
  }
  return size;
}

}  // namespace broker
}  // namespace upmq
