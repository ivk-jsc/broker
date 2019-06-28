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
 
package com.broker.libupmq.destination;

import javax.jms.Destination;

public class UPMQDestination implements Destination {

	public static final String QUEUE_PREFIX = "queue://";
	public static final String TOPIC_PREFIX = "topic://";
	public static final String TEMP_QUEUE_PREFIX = "temp-queue://";
	public static final String TEMP_TOPIC_PREFIX = "temp-topic://";

	public static final String DEFAULT_DESTINATION = "defaultDestination";

	private String _uri = null;

	public UPMQDestination(String uri) {
		_uri = uri;
	}

	public String getUri() {
		return _uri;
	}
}
