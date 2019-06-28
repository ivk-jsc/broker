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
 
package com.broker.libupmq.foreign;

import javax.jms.BytesMessage;
import javax.jms.MapMessage;
import javax.jms.Message;
import javax.jms.ObjectMessage;
import javax.jms.StreamMessage;
import javax.jms.TextMessage;

public class ForeignConverterFactory {

	public static ForeignConverter create(Message message) {
		ForeignConverter result;

		if (message instanceof BytesMessage) {
			result = new ForeignBytesMessageConverter();
		} else if (message instanceof MapMessage) {
			result = new ForeignMapMessageConverter();
		} else if (message instanceof ObjectMessage) {
			result = new ForeignObjectMessageConverter();
		} else if (message instanceof StreamMessage) {
			result = new ForeignStreamMessageConverter();
		} else if (message instanceof TextMessage) {
			result = new ForeignTextMessageConverter();
		} else {
			result = new ForeignMessageConverter();
		}
		return result;
	}
}
