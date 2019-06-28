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
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import org.apache.log4j.Logger;

/**
 * Adds the incrementing sequence number to commands along with performing the correlation of responses to requests to
 * create a blocking request-response semantics.
 * 
 * 
 */
public class ResponseCorrelator extends TransportFilter {

	private static final Logger LOG = Logger.getLogger(ResponseCorrelator.class);
	private final Map<Integer, FutureResponse> requestMap = new HashMap<Integer, FutureResponse>();
	private IntSequenceGenerator sequenceGenerator;
	private final boolean debug = LOG.isDebugEnabled();
	private IOException error;

	public ResponseCorrelator(Transport next) {
		this(next, new IntSequenceGenerator());
	}

	public ResponseCorrelator(Transport next, IntSequenceGenerator sequenceGenerator) {
		super(next);
		this.sequenceGenerator = sequenceGenerator;
	}

	@Override
	public void oneway(Object o) throws IOException {
		Command command = (Command) o;
		command.setCommandId(sequenceGenerator.getNextSequenceId());
		command.setResponseRequired(false);
		next.oneway(command);
	}

	@Override
	public FutureResponse asyncRequest(Object o, ResponseCallback responseCallback) throws IOException {
		Command command = (Command) o;
		command.setCommandId(sequenceGenerator.getNextSequenceId());
		command.setResponseRequired(true);
		FutureResponse future = new FutureResponse(responseCallback);
		IOException priorError = null;
		synchronized (requestMap) {
			priorError = this.error;
			if (priorError == null) {
				requestMap.put(new Integer(command.getCommandId()), future);
			}
		}

		if (priorError != null) {
			future.set(new ExceptionResponse(priorError));
			throw priorError;
		}

		next.oneway(command);
		return future;
	}

	@Override
	public Object request(Object command) throws IOException {
		FutureResponse response = asyncRequest(command, null);
		return response.getResult();
	}

	@Override
	public Object request(Object command, int timeout) throws IOException {
		FutureResponse response = asyncRequest(command, null);
		return response.getResult(timeout);
	}

	@Override
	public void onCommand(Object o) {
		Command command = null;
		if (o instanceof Command) {
			command = (Command) o;
		} else {
			throw new ClassCastException("Object cannot be converted to a Command,  Object: " + o);
		}
		if (command.isResponse()) {
			Response response = (Response) command;
			FutureResponse future = null;
			synchronized (requestMap) {
				future = requestMap.remove(Integer.valueOf(response.getCorrelationId()));
			}
			if (future != null) {
				future.set(response);
			} else {
				if (debug) {
					LOG.debug("Received unexpected response: {" + command + "}for command id: " + response.getCorrelationId());
				}
			}
		} else {
			getTransportListener().onCommand(command);
		}
	}

	/**
	 * If an async exception occurs, then assume no responses will arrive for any of current requests. Lets let them
	 * know of the problem.
	 */
	@Override
	public void onException(IOException error) {
		dispose(error);
		super.onException(error);
	}

	@Override
	public void stop() throws Exception {
		dispose(new IOException("Stopped."));
		super.stop();
	}

	private void dispose(IOException error) {
		ArrayList<FutureResponse> requests = null;
		synchronized (requestMap) {
			if (this.error == null) {
				this.error = error;
				requests = new ArrayList<FutureResponse>(requestMap.values());
				requestMap.clear();
			}
		}
		if (requests != null) {
			for (Iterator<FutureResponse> iter = requests.iterator(); iter.hasNext();) {
				FutureResponse fr = iter.next();
				fr.set(new ExceptionResponse(error));
			}
		}
	}

	public IntSequenceGenerator getSequenceGenerator() {
		return sequenceGenerator;
	}

	@Override
	public String toString() {
		return next.toString();
	}
}
