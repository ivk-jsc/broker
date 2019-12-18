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

#ifndef __ExceptionImpl_H__
#define __ExceptionImpl_H__

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
#include <cms/NullFormatException.h>
#include <cms/NumberFormatException.h>
#include <cms/ResourceAllocationException.h>
#include <cms/TransactionInProgressException.h>
#include <cms/TransactionRolledBackException.h>
#include <cms/UnsupportedOperationException.h>

#define CATCH_ALL_NOTHROW \
  catch (...) {           \
  }

#define CATCH_ALL_THROW_CMSEXCEPTION                                                                                                          \
  catch (cms::CMSSecurityException & secex) {                                                                                                 \
    secex.setMark(__FUNCTION__, __FILE__, __LINE__);                                                                                          \
    throw;                                                                                                                                    \
  }                                                                                                                                           \
  catch (cms::IllegalStateException & illstex) {                                                                                              \
    illstex.setMark(__FUNCTION__, __FILE__, __LINE__);                                                                                        \
    throw;                                                                                                                                    \
  }                                                                                                                                           \
  catch (cms::InvalidClientIdException & invidex) {                                                                                           \
    invidex.setMark(__FUNCTION__, __FILE__, __LINE__);                                                                                        \
    throw;                                                                                                                                    \
  }                                                                                                                                           \
  catch (cms::InvalidDestinationException & invdestex) {                                                                                      \
    invdestex.setMark(__FUNCTION__, __FILE__, __LINE__);                                                                                      \
    throw;                                                                                                                                    \
  }                                                                                                                                           \
  catch (cms::InvalidSelectorException & invselex) {                                                                                          \
    invselex.setMark(__FUNCTION__, __FILE__, __LINE__);                                                                                       \
    throw;                                                                                                                                    \
  }                                                                                                                                           \
  catch (cms::MessageEOFException & eofex) {                                                                                                  \
    eofex.setMark(__FUNCTION__, __FILE__, __LINE__);                                                                                          \
    throw;                                                                                                                                    \
  }                                                                                                                                           \
  catch (cms::MessageFormatException & formex) {                                                                                              \
    formex.setMark(__FUNCTION__, __FILE__, __LINE__);                                                                                         \
    throw;                                                                                                                                    \
  }                                                                                                                                           \
  catch (cms::MessageNotReadableException & nreadex) {                                                                                        \
    nreadex.setMark(__FUNCTION__, __FILE__, __LINE__);                                                                                        \
    throw;                                                                                                                                    \
  }                                                                                                                                           \
  catch (cms::MessageNotWriteableException & nwritex) {                                                                                       \
    nwritex.setMark(__FUNCTION__, __FILE__, __LINE__);                                                                                        \
    throw;                                                                                                                                    \
  }                                                                                                                                           \
  catch (cms::NullFormatException & nullformex) {                                                                                             \
    nullformex.setMark(__FUNCTION__, __FILE__, __LINE__);                                                                                     \
    throw;                                                                                                                                    \
  }                                                                                                                                           \
  catch (cms::NumberFormatException & nbformex) {                                                                                             \
    nbformex.setMark(__FUNCTION__, __FILE__, __LINE__);                                                                                       \
    throw;                                                                                                                                    \
  }                                                                                                                                           \
  catch (cms::ResourceAllocationException & resalcex) {                                                                                       \
    resalcex.setMark(__FUNCTION__, __FILE__, __LINE__);                                                                                       \
    throw;                                                                                                                                    \
  }                                                                                                                                           \
  catch (cms::TransactionInProgressException & txprogex) {                                                                                    \
    txprogex.setMark(__FUNCTION__, __FILE__, __LINE__);                                                                                       \
    throw;                                                                                                                                    \
  }                                                                                                                                           \
  catch (cms::TransactionRolledBackException & txrbex) {                                                                                      \
    txrbex.setMark(__FUNCTION__, __FILE__, __LINE__);                                                                                         \
    throw;                                                                                                                                    \
  }                                                                                                                                           \
  catch (cms::UnsupportedOperationException & unopex) {                                                                                       \
    unopex.setMark(__FUNCTION__, __FILE__, __LINE__);                                                                                         \
    throw;                                                                                                                                    \
  }                                                                                                                                           \
  catch (cms::CMSException & cmsex) {                                                                                                         \
    cmsex.setMark(__FUNCTION__, __FILE__, __LINE__);                                                                                          \
    throw;                                                                                                                                    \
  }                                                                                                                                           \
  catch (std::exception & stdex) {                                                                                                            \
    throw cms::CMSException(stdex.what(),                                                                                                     \
                            nullptr,                                                                                                          \
                            std::vector<triplet<std::string, std::string, int> >(                                                             \
                                1, make_triplet<std::string, std::string, int>(std::string(__FUNCTION__), std::string(__FILE__), __LINE__))); \
  }                                                                                                                                           \
  catch (...) {                                                                                                                               \
    throw cms::CMSException("Unknown Exception",                                                                                              \
                            nullptr,                                                                                                          \
                            std::vector<triplet<std::string, std::string, int> >(                                                             \
                                1, make_triplet<std::string, std::string, int>(std::string(__FUNCTION__), std::string(__FILE__), __LINE__))); \
  }

//  if (_ci._parentConnection->_ci._exceptionListener != NULL)
//  _ci._parentConnection->_ci._exceptionListener->onException(ex);

#endif  //__ExceptionImp_H__
