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

import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.atomic.AtomicBoolean;

import org.apache.log4j.Logger;

/**
 * A helper class for working with services together with a useful base class for service implementations.
 * 
 * 
 */
public abstract class ServiceSupport implements Service {
	private static final Logger LOG = Logger.getLogger(ServiceSupport.class);

	private AtomicBoolean started = new AtomicBoolean(false);
	private AtomicBoolean stopping = new AtomicBoolean(false);
	private AtomicBoolean stopped = new AtomicBoolean(false);
	private List<ServiceListener> serviceListeners = new CopyOnWriteArrayList<ServiceListener>();

	public static void dispose(Service service) {
		try {
			service.stop();
		} catch (Exception e) {
			LOG.debug("Could not stop service: " + service + ". Reason: " + e, e);
		}
	}

	@Override
	public void start() throws Exception {
		if (started.compareAndSet(false, true)) {
			boolean success = false;
			stopped.set(false);
			try {
				preStart();
				doStart();
				success = true;
			} finally {
				started.set(success);
			}
			for (ServiceListener l : this.serviceListeners) {
				l.started(this);
			}
		}
	}

	@Override
	public void stop() throws Exception {
		if (stopped.compareAndSet(false, true)) {
			stopping.set(true);
			ServiceStopper stopper = new ServiceStopper();
			try {
				doStop(stopper);
			} catch (Exception e) {
				stopper.onException(this, e);
			} finally {
				postStop(stopper);
			}
			stopped.set(true);
			started.set(false);
			stopping.set(false);
			for (ServiceListener l : this.serviceListeners) {
				l.stopped(this);
			}
			stopper.throwFirstException();
		}
	}

	/**
	 * @return true if this service has been started
	 */
	public boolean isStarted() {
		return started.get();
	}

	/**
	 * @return true if this service is in the process of closing
	 */
	public boolean isStopping() {
		return stopping.get();
	}

	/**
	 * @return true if this service is closed
	 */
	public boolean isStopped() {
		return stopped.get();
	}

	public void addServiceListener(ServiceListener l) {
		this.serviceListeners.add(l);
	}

	public void removeServiceListener(ServiceListener l) {
		this.serviceListeners.remove(l);
	}

	/**
	 *
	 * handle for various operations after stopping the service (like locking)
	 *
	 * @throws Exception
	 */
	protected void postStop(ServiceStopper stopper) throws Exception {
	}

	protected abstract void doStop(ServiceStopper stopper) throws Exception;

	/**
	 *
	 * handle for various operations before starting the service (like locking)
	 *
	 * @throws Exception
	 */
	protected void preStart() throws Exception {
	}

	protected abstract void doStart() throws Exception;
}
