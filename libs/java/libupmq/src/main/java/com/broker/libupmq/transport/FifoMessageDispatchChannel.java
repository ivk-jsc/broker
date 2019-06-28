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

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

public class FifoMessageDispatchChannel implements MessageDispatchChannel {

	private final Object mutex = new Object();
	private final LinkedList<Command> list;
	private boolean closed;
	private boolean running;

	public FifoMessageDispatchChannel() {
		this.list = new LinkedList<Command>();
	}

	@Override
	public void enqueue(Command message) {
		synchronized (mutex) {
			list.addLast(message);
			mutex.notify();
		}
	}

	@Override
	public void enqueueFirst(Command message) {
		synchronized (mutex) {
			list.addFirst(message);
			mutex.notify();
		}
	}

	@Override
	public boolean isEmpty() {
		synchronized (mutex) {
			return list.isEmpty();
		}
	}

	@Override
	public Command dequeue(long timeout) throws InterruptedException {
		synchronized (mutex) {
			// Wait until the consumer is ready to deliver messages.
			while (timeout != 0 && !closed && (list.isEmpty() || !running)) {
				if (timeout == -1) {
					mutex.wait();
				} else {
					mutex.wait(timeout);
					break;
				}
			}
			if (closed || !running || list.isEmpty()) {
				return null;
			}
			Command command = list.removeFirst();

			//4bas
			//						if (((UPMQCommand) command)._headerMessage.getProtoMessageTypeCase() == Protocol.ProtoMessage.ProtoMessageTypeCase.MESSAGE) {
			//							System.out.println("from local queue: " + ((UPMQCommand) command)._headerMessage.getMessage().getMessageId());
			//						}

			return command;
		}
	}

	@Override
	public Command dequeueNoWait() {
		synchronized (mutex) {
			if (closed || !running || list.isEmpty()) {
				return null;
			}
			return list.removeFirst();
		}
	}

	@Override
	public Command peek() {
		synchronized (mutex) {
			if (closed || !running || list.isEmpty()) {
				return null;
			}
			return list.getFirst();
		}
	}

	@Override
	public void start() {
		synchronized (mutex) {
			running = true;
			closed = false;
			mutex.notifyAll();
		}
	}

	@Override
	public void stop() {
		synchronized (mutex) {
			running = false;
			mutex.notifyAll();
		}
	}

	@Override
	public void close() {
		synchronized (mutex) {
			if (!closed) {
				running = false;
				closed = true;
			}
			mutex.notifyAll();
		}
	}

	@Override
	public void clear() {
		synchronized (mutex) {
			list.clear();
		}
	}

	@Override
	public boolean isClosed() {
		return closed;
	}

	@Override
	public int size() {
		synchronized (mutex) {
			return list.size();
		}
	}

	@Override
	public Object getMutex() {
		return mutex;
	}

	@Override
	public boolean isRunning() {
		return running;
	}

	@Override
	public List<Command> removeAll() {
		synchronized (mutex) {
			ArrayList<Command> rc = new ArrayList<Command>(list);
			list.clear();
			return rc;
		}
	}

	@Override
	public String toString() {
		synchronized (mutex) {
			return list.toString();
		}
	}
}
