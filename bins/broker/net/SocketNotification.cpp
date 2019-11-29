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


#include "SocketNotification.h"


namespace upmq {
namespace Net {


SocketNotification::SocketNotification(SocketReactor* pReactor):
	_pReactor(pReactor)
{
}


SocketNotification::~SocketNotification()
{
}

	
void SocketNotification::setSocket(const Poco::Net::Socket& socket)
{
	_socket = socket;
}


ReadableNotification::ReadableNotification(SocketReactor* pReactor): 
	SocketNotification(pReactor)
{
}


ReadableNotification::~ReadableNotification()
{
}


WritableNotification::WritableNotification(SocketReactor* pReactor): 
	SocketNotification(pReactor)
{
}


WritableNotification::~WritableNotification()
{
}


ErrorNotification::ErrorNotification(SocketReactor* pReactor): 
	SocketNotification(pReactor)
{
}


ErrorNotification::~ErrorNotification()
{
}


TimeoutNotification::TimeoutNotification(SocketReactor* pReactor): 
	SocketNotification(pReactor)
{
}


TimeoutNotification::~TimeoutNotification()
{
}


IdleNotification::IdleNotification(SocketReactor* pReactor): 
	SocketNotification(pReactor)
{
}


IdleNotification::~IdleNotification()
{
}


ShutdownNotification::ShutdownNotification(SocketReactor* pReactor): 
	SocketNotification(pReactor)
{
}


ShutdownNotification::~ShutdownNotification()
{
}


} } // namespace Poco::Net
