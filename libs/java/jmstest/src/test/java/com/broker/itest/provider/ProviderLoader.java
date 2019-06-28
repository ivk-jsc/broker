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
 
package com.broker.itest.provider;

import org.apache.log4j.Appender;
import org.apache.log4j.ConsoleAppender;
import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.apache.log4j.SimpleLayout;

public abstract class ProviderLoader {

	//QueueFactory Name
	protected static final String qcfName = "testQCF";
	//TopicFactory Name
	protected static final String tcfName = "testTCF";
	//Timeout
	public static final long TIMEOUT = 5 * 1000;

	public static Provider getProvider() {
		return new UPMQProvider();
	}

	/**
	 * Ensures that if Log4j is not configured, at least all warnings and errors are sent to the console.
	 */
	private static void setupLog4j() {
		Logger logger = Logger.getRootLogger();
		if (!logger.getAllAppenders().hasMoreElements()) {
			Appender console = new ConsoleAppender(new SimpleLayout());
			logger.addAppender(console);
			logger.setLevel(Level.INFO);
		}
	}
}
