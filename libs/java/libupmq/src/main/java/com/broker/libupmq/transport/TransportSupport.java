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

import java.io.IOException;
import java.net.URI;

import org.apache.log4j.Logger;

/**
 * A useful base class for transport implementations.
 * 
 * 
 */
public abstract class TransportSupport extends ServiceSupport implements Transport {
	private static final Logger LOG = Logger.getLogger(TransportSupport.class);

	TransportListener transportListener;

	/**
	 * Returns the current transport listener
	 */
	@Override
	public TransportListener getTransportListener() {
		return transportListener;
	}

	/**
	 * Registers an inbound command listener
	 * 
	 * @param commandListener
	 */
	@Override
	public void setTransportListener(TransportListener commandListener) {
		this.transportListener = commandListener;
	}

	/**
	 * narrow acceptance
	 * 
	 * @param target
	 * @return 'this' if assignable
	 */
	@Override
	public <T> T narrow(Class<T> target) {
		boolean assignableFrom = target.isAssignableFrom(getClass());
		if (assignableFrom) {
			return target.cast(this);
		}
		return null;
	}

	@Override
	public FutureResponse asyncRequest(Object command, ResponseCallback responseCallback) throws IOException {
		throw new AssertionError("Unsupported Method");
	}

	@Override
	public Object request(Object command) throws IOException {
		throw new AssertionError("Unsupported Method");
	}

	@Override
	public Object request(Object command, int timeout) throws IOException {
		throw new AssertionError("Unsupported Method");
	}

	/**
	 * Process the inbound command
	 */
	public void doConsume(Object command) {
		if (command != null) {
			if (transportListener != null) {
				transportListener.onCommand(command);
			} else {
				LOG.error("No transportListener available to process inbound command: " + command);
			}
		}
	}

	/**
	 * Passes any IO exceptions into the transport listener
	 */
	public void onException(IOException e) {
		if (transportListener != null) {
			try {
				transportListener.onException(e);
			} catch (RuntimeException e2) {
				// Handle any unexpected runtime exceptions by debug logging
				// them.
				LOG.debug("Unexpected runtime exception: " + e2, e2);
			}
		}
	}

	protected void checkStarted() throws IOException {
		if (!isStarted()) {
			throw new IOException("The transport is not running.");
		}
	}

	@Override
	public boolean isFaultTolerant() {
		return false;
	}

	@Override
	public void reconnect(URI uri) throws IOException {
		throw new IOException("Not supported");
	}

	@Override
	public boolean isReconnectSupported() {
		return false;
	}

	@Override
	public boolean isUpdateURIsSupported() {
		return false;
	}

	@Override
	public void updateURIs(boolean reblance, URI[] uris) throws IOException {
		throw new IOException("Not supported");
	}

	@Override
	public boolean isDisposed() {
		return isStopped();
	}

	@Override
	public boolean isConnected() {
		return isStarted();
	}

}
