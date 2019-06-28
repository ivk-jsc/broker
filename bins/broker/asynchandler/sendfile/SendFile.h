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

#ifndef __BROKER_SENDFILE_H
#define __BROKER_SENDFILE_H

#include <fstream>
#include <Poco/Net/Socket.h>
#include <string>

#ifdef _WIN32
#include <windows.h>
#define file_descr_t HANDLE
#else
#define file_descr_t int
#endif

namespace upmq {
namespace broker {

// Sending a file presented as std::fstream to the (poco-)socket using system dependent api
class SendFile {
 public:
  explicit SendFile(Poco::Net::Socket& socket, std::fstream& stream, size_t partSize);
  // send whole file
  bool operator()();

  inline std::streamoff offset() const { return _offset; }

  inline std::streamoff fileSize() const { return _fileSize; }

  inline int lastError() const { return _error; }

  inline const std::string& lastErrorStr() const { return _errstr; }

 private:
#ifndef _WIN32
  bool fileSizeFits_off_t() const;
  bool isComplete() const;
  bool sendPart();
#endif
  void clearError();
  void setError(int error);
  static std::streamoff getFileSize(std::fstream& stream);

 private:
  poco_socket_t _sd;
  file_descr_t _fd;
  size_t _partSize;
  std::streamoff _offset;
  std::streamoff _fileSize;
  int _error;
  std::string _errstr;
};

}  // namespace broker
}  // namespace upmq

#endif  // __BROKER_SENDFILE_H
