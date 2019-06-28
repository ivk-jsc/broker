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

/**
 * A useful base class for a transport implementation which has a background reading thread.
 * 
 * 
 */
public abstract class TransportThreadSupport extends TransportSupport implements Runnable {

	private boolean daemon;
	private Thread runner;
	// should be a multiple of 128k
	private long stackSize;

	public boolean isDaemon() {
		return daemon;
	}

	public void setDaemon(boolean daemon) {
		this.daemon = daemon;
	}

	@Override
	protected void doStart() throws Exception {
		runner = new Thread(null, this, "UPMQ Transport: " + toString(), stackSize);
		runner.setDaemon(daemon);
		runner.start();
	}

	/**
	 * @return the stackSize
	 */
	public long getStackSize() {
		return this.stackSize;
	}

	/**
	 * @param stackSize
	 *            the stackSize to set
	 */
	public void setStackSize(long stackSize) {
		this.stackSize = stackSize;
	}
}
