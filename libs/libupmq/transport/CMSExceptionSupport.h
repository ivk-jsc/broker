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

#ifndef _CMSEXCEPTIONSUPPORT_H_
#define _CMSEXCEPTIONSUPPORT_H_

#include <transport/Config.h>

#include <cms/CMSException.h>
#include <cms/CMSSecurityException.h>
#include <cms/IllegalStateException.h>
#include <cms/InvalidClientIdException.h>
#include <cms/InvalidDestinationException.h>
#include <cms/InvalidSelectorException.h>
#include <cms/MessageEOFException.h>
#include <cms/MessageFormatException.h>
#include <cms/MessageNotReadableException.h>
#include <cms/MessageNotWriteableException.h>
#include <cms/ResourceAllocationException.h>
#include <cms/TransactionInProgressException.h>
#include <cms/TransactionRolledBackException.h>
#include <cms/UnsupportedOperationException.h>

#include <decaf/lang/Exception.h>

#include <string>

namespace upmq {
namespace transport {

class UPMQCPP_API CMSExceptionSupport {
 private:
  CMSExceptionSupport();

 public:
  virtual ~CMSExceptionSupport();

  static cms::CMSException create(const std::string& msg, const decaf::lang::Exception& cause);

  static cms::CMSException create(const decaf::lang::Exception& cause);

  static cms::MessageEOFException createMessageEOFException(const decaf::lang::Exception& cause);

  static cms::MessageFormatException createMessageFormatException(const decaf::lang::Exception& cause);
};
}  // namespace transport
}  // namespace upmq

/**
 * Macro for catching an exception of one type and then re-throwing
 * as a Basic CMSException, good for cases where the method isn't specific
 * about what CMS Exceptions are thrown, bad if you need to throw an
 * exception of MessageNotReadableException for instance.
 */
#define AMQ_CATCH_ALL_THROW_CMSEXCEPTION()                                                                                      \
  catch (cms::CMSSecurityException & ex) {                                                                                      \
    ex.setMark(__FILE__, __LINE__);                                                                                             \
    throw;                                                                                                                      \
  }                                                                                                                             \
  catch (cms::IllegalStateException & ex) {                                                                                     \
    ex.setMark(__FILE__, __LINE__);                                                                                             \
    throw;                                                                                                                      \
  }                                                                                                                             \
  catch (cms::InvalidClientIdException & ex) {                                                                                  \
    ex.setMark(__FILE__, __LINE__);                                                                                             \
    throw;                                                                                                                      \
  }                                                                                                                             \
  catch (cms::InvalidDestinationException & ex) {                                                                               \
    ex.setMark(__FILE__, __LINE__);                                                                                             \
    throw;                                                                                                                      \
  }                                                                                                                             \
  catch (cms::InvalidSelectorException & ex) {                                                                                  \
    ex.setMark(__FILE__, __LINE__);                                                                                             \
    throw;                                                                                                                      \
  }                                                                                                                             \
  catch (cms::MessageEOFException & ex) {                                                                                       \
    ex.setMark(__FILE__, __LINE__);                                                                                             \
    throw;                                                                                                                      \
  }                                                                                                                             \
  catch (cms::MessageFormatException & ex) {                                                                                    \
    ex.setMark(__FILE__, __LINE__);                                                                                             \
    throw;                                                                                                                      \
  }                                                                                                                             \
  catch (cms::MessageNotReadableException & ex) {                                                                               \
    ex.setMark(__FILE__, __LINE__);                                                                                             \
    throw;                                                                                                                      \
  }                                                                                                                             \
  catch (cms::MessageNotWriteableException & ex) {                                                                              \
    ex.setMark(__FILE__, __LINE__);                                                                                             \
    throw;                                                                                                                      \
  }                                                                                                                             \
  catch (cms::ResourceAllocationException & ex) {                                                                               \
    ex.setMark(__FILE__, __LINE__);                                                                                             \
    throw;                                                                                                                      \
  }                                                                                                                             \
  catch (cms::TransactionInProgressException & ex) {                                                                            \
    ex.setMark(__FILE__, __LINE__);                                                                                             \
    throw;                                                                                                                      \
  }                                                                                                                             \
  catch (cms::TransactionRolledBackException & ex) {                                                                            \
    ex.setMark(__FILE__, __LINE__);                                                                                             \
    throw;                                                                                                                      \
  }                                                                                                                             \
  catch (cms::UnsupportedOperationException & ex) {                                                                             \
    ex.setMark(__FILE__, __LINE__);                                                                                             \
    throw;                                                                                                                      \
  }                                                                                                                             \
  catch (cms::CMSException & ex) {                                                                                              \
    ex.setMark(__FILE__, __LINE__);                                                                                             \
    throw;                                                                                                                      \
  }                                                                                                                             \
  catch (upmq::transport::UPMQException & ex) {                                                                                 \
    if (ex.getCause() != nullptr) {                                                                                             \
      const std::exception* cause = ex.getCause();                                                                              \
      {                                                                                                                         \
        const cms::CMSSecurityException* exception = dynamic_cast<const cms::CMSSecurityException*>(cause);                     \
        if (exception != nullptr) {                                                                                             \
          throw cms::CMSSecurityException(*exception);                                                                          \
        }                                                                                                                       \
      }                                                                                                                         \
      {                                                                                                                         \
        const cms::IllegalStateException* exception = dynamic_cast<const cms::IllegalStateException*>(cause);                   \
        if (exception != nullptr) {                                                                                             \
          throw cms::IllegalStateException(*exception);                                                                         \
        }                                                                                                                       \
      }                                                                                                                         \
      {                                                                                                                         \
        const cms::InvalidClientIdException* exception = dynamic_cast<const cms::InvalidClientIdException*>(cause);             \
        if (exception != nullptr) {                                                                                             \
          throw cms::InvalidClientIdException(*exception);                                                                      \
        }                                                                                                                       \
      }                                                                                                                         \
      {                                                                                                                         \
        const cms::InvalidDestinationException* exception = dynamic_cast<const cms::InvalidDestinationException*>(cause);       \
        if (exception != nullptr) {                                                                                             \
          throw cms::InvalidDestinationException(*exception);                                                                   \
        }                                                                                                                       \
      }                                                                                                                         \
      {                                                                                                                         \
        const cms::InvalidSelectorException* exception = dynamic_cast<const cms::InvalidSelectorException*>(cause);             \
        if (exception != nullptr) {                                                                                             \
          throw cms::InvalidSelectorException(*exception);                                                                      \
        }                                                                                                                       \
      }                                                                                                                         \
      {                                                                                                                         \
        const cms::MessageEOFException* exception = dynamic_cast<const cms::MessageEOFException*>(cause);                       \
        if (exception != nullptr) {                                                                                             \
          throw cms::MessageEOFException(*exception);                                                                           \
        }                                                                                                                       \
      }                                                                                                                         \
      {                                                                                                                         \
        const cms::MessageFormatException* exception = dynamic_cast<const cms::MessageFormatException*>(cause);                 \
        if (exception != nullptr) {                                                                                             \
          throw cms::MessageFormatException(*exception);                                                                        \
        }                                                                                                                       \
      }                                                                                                                         \
      {                                                                                                                         \
        const cms::MessageNotReadableException* exception = dynamic_cast<const cms::MessageNotReadableException*>(cause);       \
        if (exception != nullptr) {                                                                                             \
          throw cms::MessageNotReadableException(*exception);                                                                   \
        }                                                                                                                       \
      }                                                                                                                         \
      {                                                                                                                         \
        const cms::MessageNotWriteableException* exception = dynamic_cast<const cms::MessageNotWriteableException*>(cause);     \
        if (exception != nullptr) {                                                                                             \
          throw cms::MessageNotWriteableException(*exception);                                                                  \
        }                                                                                                                       \
      }                                                                                                                         \
      {                                                                                                                         \
        const cms::ResourceAllocationException* exception = dynamic_cast<const cms::ResourceAllocationException*>(cause);       \
        if (exception != nullptr) {                                                                                             \
          throw cms::ResourceAllocationException(*exception);                                                                   \
        }                                                                                                                       \
      }                                                                                                                         \
      {                                                                                                                         \
        const cms::TransactionInProgressException* exception = dynamic_cast<const cms::TransactionInProgressException*>(cause); \
        if (exception != nullptr) {                                                                                             \
          throw cms::TransactionInProgressException(*exception);                                                                \
        }                                                                                                                       \
      }                                                                                                                         \
      {                                                                                                                         \
        const cms::TransactionRolledBackException* exception = dynamic_cast<const cms::TransactionRolledBackException*>(cause); \
        if (exception != nullptr) {                                                                                             \
          throw cms::TransactionRolledBackException(*exception);                                                                \
        }                                                                                                                       \
      }                                                                                                                         \
      {                                                                                                                         \
        const cms::UnsupportedOperationException* exception = dynamic_cast<const cms::UnsupportedOperationException*>(cause);   \
        if (exception != nullptr) {                                                                                             \
          throw cms::UnsupportedOperationException(*exception);                                                                 \
        }                                                                                                                       \
      }                                                                                                                         \
      {                                                                                                                         \
        const cms::CMSException* exception = dynamic_cast<const cms::CMSException*>(cause);                                     \
        if (exception != nullptr) {                                                                                             \
          throw cms::CMSException(*exception);                                                                                  \
        }                                                                                                                       \
      }                                                                                                                         \
    }                                                                                                                           \
    ex.setMark(__FILE__, __LINE__);                                                                                             \
    throw ex.convertToCMSException();                                                                                           \
  }                                                                                                                             \
  catch (decaf::lang::Exception & ex) {                                                                                         \
    ex.setMark(__FILE__, __LINE__);                                                                                             \
    upmq::transport::UPMQException amqEx(ex);                                                                                   \
    throw amqEx.convertToCMSException();                                                                                        \
  }                                                                                                                             \
  catch (std::exception & ex) {                                                                                                 \
    throw cms::CMSException(ex.what(), nullptr);                                                                                \
  }                                                                                                                             \
  catch (...) {                                                                                                                 \
    throw cms::CMSException("Caught Unknown Exception", nullptr);                                                               \
  }

#endif /* _CMSEXCEPTIONSUPPORT_H_ */
