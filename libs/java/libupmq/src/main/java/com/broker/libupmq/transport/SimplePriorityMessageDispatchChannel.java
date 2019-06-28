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

public class SimplePriorityMessageDispatchChannel implements MessageDispatchChannel {
	private static final Integer MAX_PRIORITY = 10;
	private final Object mutex = new Object();
	private final LinkedList<Command>[] lists;
	private boolean closed;
	private boolean running;
	private int size = 0;

	@SuppressWarnings("unchecked")
	public SimplePriorityMessageDispatchChannel() {
		this.lists = new LinkedList[MAX_PRIORITY];
		for (int i = 0; i < MAX_PRIORITY; i++) {
			lists[i] = new LinkedList<Command>();
		}
	}

	@Override
	public void enqueue(Command message) {
		synchronized (mutex) {
			getList(message).addLast(message);
			this.size++;
			mutex.notify();
		}
	}

	@Override
	public void enqueueFirst(Command message) {
		synchronized (mutex) {
			getList(message).addFirst(message);
			this.size++;
			mutex.notify();
		}
	}

	@Override
	public boolean isEmpty() {
		return this.size == 0;
	}

	@Override
	public Command dequeue(long timeout) throws InterruptedException {
		synchronized (mutex) {
			// Wait until the consumer is ready to deliver messages.
			while (timeout != 0 && !closed && (isEmpty() || !running)) {
				if (timeout == -1) {
					mutex.wait();
				} else {
					mutex.wait(timeout);
					break;
				}
			}
			if (closed || !running || isEmpty()) {
				return null;
			}
			return removeFirst();
		}
	}

	@Override
	public Command dequeueNoWait() {
		synchronized (mutex) {
			if (closed || !running || isEmpty()) {
				return null;
			}
			return removeFirst();
		}
	}

	@Override
	public Command peek() {
		synchronized (mutex) {
			if (closed || !running || isEmpty()) {
				return null;
			}
			return getFirst();
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
			for (int i = 0; i < MAX_PRIORITY; i++) {
				lists[i].clear();
			}
			this.size = 0;
		}
	}

	@Override
	public boolean isClosed() {
		return closed;
	}

	@Override
	public int size() {
		synchronized (mutex) {
			return this.size;
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
			ArrayList<Command> result = new ArrayList<Command>(size());
			for (int i = MAX_PRIORITY - 1; i >= 0; i--) {
				List<Command> list = lists[i];
				result.addAll(list);
				size -= list.size();
				list.clear();
			}
			return result;
		}
	}

	@Override
	public String toString() {
		String result = "";
		for (int i = MAX_PRIORITY - 1; i >= 0; i--) {
			result += i + ":{" + lists[i].toString() + "}";
		}
		return result;
	}

	protected int getPriority(Command message) {
		int priority = javax.jms.Message.DEFAULT_PRIORITY;
		if (message != null) {
			priority = Math.max(((UPMQCommand) message)._headerMessage.getMessage().getPriority(), 0);
			priority = Math.min(priority, 9);
		}
		return priority;
	}

	protected LinkedList<Command> getList(Command md) {
		return lists[getPriority(md)];
	}

	private final Command removeFirst() {
		if (this.size > 0) {
			for (int i = MAX_PRIORITY - 1; i >= 0; i--) {
				LinkedList<Command> list = lists[i];
				if (!list.isEmpty()) {
					this.size--;
					return list.removeFirst();
				}
			}
		}
		return null;
	}

	private final Command getFirst() {
		if (this.size > 0) {
			for (int i = MAX_PRIORITY - 1; i >= 0; i--) {
				LinkedList<Command> list = lists[i];
				if (!list.isEmpty()) {
					return list.getFirst();
				}
			}
		}
		return null;
	}
}
