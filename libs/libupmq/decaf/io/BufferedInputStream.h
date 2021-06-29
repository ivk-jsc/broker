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

#ifndef _DECAF_IO_BUFFEREDINPUTSTREAM_H_
#define _DECAF_IO_BUFFEREDINPUTSTREAM_H_

#include <decaf/io/FilterInputStream.h>
#include <decaf/lang/exceptions/IllegalArgumentException.h>
#include <decaf/util/Config.h>

namespace decaf {
namespace io {

/**
 * A wrapper around another input stream that performs
 * a buffered read, where it reads more data than it needs
 * in order to reduce the number of io operations on the
 * input stream.
 */
class DECAF_API BufferedInputStream : public FilterInputStream {
 private:
  int pos;
  int count;
  int markLimit;
  int markPos;
  int bufferSize;
  unsigned char *buff;

  // Proxy to the actual buffer, when NULL it signals this stream is closed.
  // the actual buffer is deleted in the destructor.
  unsigned char *proxyBuffer;

 private:
  BufferedInputStream(const BufferedInputStream &);
  BufferedInputStream &operator=(const BufferedInputStream &);

 public:
  /**
   * Constructor
   *
   * @param stream
   *      The target input stream to buffer.
   * @param own_
   *      Indicates if we own the stream object, defaults to false.
   */
  explicit BufferedInputStream(InputStream *stream, bool own_ = false);

  /**
   * Constructor
   *
   * @param stream
   *      The target input stream to buffer.
   * @param bufferSize_
   *      The size in bytes to allocate for the internal buffer.
   * @param own_
   *      Indicates if we own the stream object, defaults to false.
   *
   * @throws IllegalArgumentException is the size is zero or negative.
   */
  BufferedInputStream(InputStream *stream, int bufferSize_, bool own_ = false);

  ~BufferedInputStream() override;

  /**
   * {@inheritDoc}
   */
  int available() const override;

  /**
   * {@inheritDoc}
   */
  void close() override;

  /**
   * {@inheritDoc}
   */
  long long skip(long long num) override;

  /**
   * {@inheritDoc}
   */
  void mark(int readLimit) override;

  /**
   * {@inheritDoc}
   */
  void reset() override;

  /**
   * {@inheritDoc}
   */
  bool markSupported() const override { return true; }

 protected:
  int doReadByte() override;

  int doReadArrayBounded(unsigned char *pbuffer, int size, int offset, int length) override;

 private:
  int bufferData(InputStream *inStream, unsigned char *&buffer);
};
}  // namespace io
}  // namespace decaf

#endif /*_DECAF_IO_BUFFEREDINPUTSTREAM_H_*/
