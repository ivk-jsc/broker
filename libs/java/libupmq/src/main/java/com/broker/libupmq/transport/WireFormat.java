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
 
package com.broker.libupmq.transport;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

/**
 * Provides a mechanism to marshal commands into and out of packets or into and out of streams, Channels and Datagrams.
 *
 * 
 */
public interface WireFormat {

	/**
	 * Stream based marshaling
	 */
	void marshal(Object command, DataOutput out) throws IOException;

	/**
	 * Packet based un-marshaling
	 */
	Object unmarshal(DataInput in) throws IOException;

	/**
	 * @param the
	 *            version of the wire format
	 */
	void setVersion(int version);

	/**
	 * @return the version of the wire format
	 */
	int getVersion();
}
