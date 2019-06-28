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

#ifndef _DECAF_IO_FILTERINPUTSTREAM_H_
#define _DECAF_IO_FILTERINPUTSTREAM_H_

#include <decaf/io/IOException.h>
#include <decaf/io/InputStream.h>
#include <decaf/lang/exceptions/IndexOutOfBoundsException.h>
#include <decaf/lang/exceptions/NullPointerException.h>

namespace decaf {
namespace io {

/**
 * A FilterInputStream contains some other input stream, which it uses
 * as its basic source of data, possibly transforming the data along the
 * way or providing additional functionality. The class FilterInputStream
 * itself simply overrides all methods of InputStream with versions
 * that pass all requests to the contained input stream. Subclasses of
 * FilterInputStream  may further override some of these methods and may
 * also provide additional methods and fields.
 */
class DECAF_API FilterInputStream : public InputStream {
 protected:
  // The input stream to wrap
  InputStream *inputStream;

  // Indicates if we own the wrapped stream
  bool own;

  // Indicates that this stream was closed
  volatile bool closed;

 private:
  FilterInputStream(const FilterInputStream &);
  FilterInputStream &operator=(const FilterInputStream &);

 public:
  /**
   * Constructor to create a wrapped InputStream
   *
   * @param inputStream
   *      The stream to wrap and filter.
   * @param own
   *      Indicates if we own the stream object, defaults to false.
   */
  FilterInputStream(InputStream *inputStream, bool own = false);

  ~FilterInputStream() override;

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
  bool markSupported() const override;

 protected:
  int doReadByte() override;

  int doReadArray(unsigned char *buffer, int size) override;

  int doReadArrayBounded(unsigned char *pbuffer, int size, int offset, int length) override;

  /**
   * @return true if this stream has been closed.
   */
  virtual bool isClosed() const;
};
}  // namespace io
}  // namespace decaf

#endif /*_DECAF_IO_FILTERINPUTSTREAM_H_*/
