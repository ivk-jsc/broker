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

#ifndef _DECAF_IO_BUFFEREDOUTPUTSTREAM_H_
#define _DECAF_IO_BUFFEREDOUTPUTSTREAM_H_

#include <decaf/io/FilterOutputStream.h>
#include <decaf/lang/exceptions/IllegalArgumentException.h>

namespace decaf {
namespace io {

/**
 * Wrapper around another output stream that buffers
 * output before writing to the target output stream.
 */
class DECAF_API BufferedOutputStream : public FilterOutputStream {
 private:
  /**
   * The internal buffer.
   */
  unsigned char *buffer;

  /**
   * The size of the internal buffer.
   */
  int bufferSize;

  /**
   * The current head of the buffer.
   */
  int head;

  /**
   * The current tail of the buffer.
   */
  int tail;

 private:
  BufferedOutputStream(const BufferedOutputStream &);
  BufferedOutputStream &operator=(const BufferedOutputStream &);

 public:
  /**
   * Constructor.
   *
   * @param stream
   *      The target output stream.
   * @param own_
   *      Indicates if this class owns the stream pointer.
   */
  explicit BufferedOutputStream(OutputStream *stream, bool own_ = false);

  /**
   * Constructor.
   *
   * @param stream
   *      The target output stream.
   * @param bufferSize
   *      The size for the internal buffer.
   * @param own_
   *      Indicates if this class owns the stream pointer.
   *
   * @throws IllegalArgumentException if the bufferSize given is negative.
   */
  BufferedOutputStream(OutputStream *stream, int bufferSize, bool own_ = false);

  ~BufferedOutputStream() override;

  /**
   * @{inheritDoc}
   */
  void flush() override;

 protected:
  void doWriteByte(unsigned char c) override;

  void doWriteArray(const unsigned char *pbuffer, int size) override;

  void doWriteArrayBounded(const unsigned char *pbuffer, int size, int offset, int length) override;

 private:
  /**
   * Initializes the internal structures.
   *
   * @param bufferSize
   *      How large to make the initial buffer when creating it.
   */
  void init(int bufferSize);

  /**
   * Writes the contents of the buffer to the output stream.
   */
  void emptyBuffer();
};
}  // namespace io
}  // namespace decaf

#endif /*_DECAF_IO_BUFFEREDOUTPUTSTREAM_H_*/
