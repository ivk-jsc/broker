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
 
package com.broker.libupmq.connection;

import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicBoolean;

import javax.jms.Connection;
import javax.jms.ConnectionConsumer;
import javax.jms.ConnectionMetaData;
import javax.jms.Destination;
import javax.jms.ExceptionListener;
import javax.jms.IllegalStateException;
import javax.jms.InvalidDestinationException;
import javax.jms.JMSException;
import javax.jms.Queue;
import javax.jms.QueueConnection;
import javax.jms.ServerSessionPool;
import javax.jms.Session;
import javax.jms.Topic;
import javax.jms.TopicConnection;
import javax.jms.XAQueueConnection;
import javax.jms.XATopicConnection;
import javax.net.SocketFactory;

import org.apache.log4j.Logger;

import com.broker.libupmq.client.UPMQMessageConsumer;
import com.broker.libupmq.core.UPMQConnectionMetaData;
import com.broker.libupmq.session.UPMQSession;
import com.broker.libupmq.transport.AsyncCallback;
import com.broker.libupmq.transport.Command;
import com.broker.libupmq.transport.ConnectionClosedException;
import com.broker.libupmq.transport.ExceptionResponse;
import com.broker.libupmq.transport.FutureResponse;
import com.broker.libupmq.transport.JMSExceptionSupport;
import com.broker.libupmq.transport.MutexTransport;
import com.broker.libupmq.transport.Response;
import com.broker.libupmq.transport.ResponseCallback;
import com.broker.libupmq.transport.ResponseCorrelator;
import com.broker.libupmq.transport.ServiceSupport;
import com.broker.libupmq.transport.TcpTransport;
import com.broker.libupmq.transport.Transport;
import com.broker.libupmq.transport.TransportListener;
import com.broker.libupmq.transport.UPMQCommand;
import com.broker.libupmq.transport.UPMQDispatcher;
import com.broker.libupmq.transport.UPMQWireFormat;
import com.broker.libupmq.transport.WireFormat;
import com.broker.protocol.Protocol;

public class UPMQConnection implements Connection, TransportListener {
	private static final Logger log = Logger.getLogger(UPMQConnection.class);

	//new
	public Transport transport = null;
	public WireFormat wireformat = null;
	private final ConcurrentHashMap<String, UPMQDispatcher> dispatchers = new ConcurrentHashMap<String, UPMQDispatcher>();
	//new (end)

	public final Object lock = new Object();

	private final AtomicBoolean _isStarted = new AtomicBoolean(false);
	private final AtomicBoolean _isStoped = new AtomicBoolean(true);
	private final AtomicBoolean _isClosed = new AtomicBoolean(false);

	private URI _providerURI = null;
	private String _userName = null;
	private String _password = null;

	public String _objectId = UUID.randomUUID().toString();
	private boolean _objectIdSet = false;
	private boolean _objectIdLocked = false;

	public ExceptionListener _exceptionListener = null;
	public Thread _exceptionThread = null;

	private ConnectionMetaData _connectionMetaData = null;

	private volatile Set<UPMQSession> _sessions = new HashSet<UPMQSession>();

	public UPMQConnection(String providerURI, String userName, String password) throws JMSException {
		_userName = userName;
		_password = password;

		try {
			_providerURI = new URI(providerURI);
		} catch (URISyntaxException e) {
			JMSException ex = new JMSException("provider uri error");
			ex.setLinkedException(e);
			throw ex;
		}

		try {
			wireformat = new UPMQWireFormat();

			transport = new TcpTransport(wireformat, SocketFactory.getDefault(), _providerURI, null);
			transport = new MutexTransport(transport);
			transport = new ResponseCorrelator(transport);

			transport.setTransportListener(this);
			transport.start();
		} catch (Exception e) {
			JMSException ex = new JMSException("transport start error");
			ex.setLinkedException(e);
			throw ex;
		}

		Protocol.ProtoMessage.Builder messageb = Protocol.ProtoMessage.newBuilder();
		messageb.setObjectId(_objectId);

		//TODO set revision number
		messageb.getConnectBuilder().getClientVersionBuilder().setClientRevisionVersion(0);
		messageb.getConnectBuilder().getClientVersionBuilder().setClientMajorVersion(0);
		messageb.getConnectBuilder().getClientVersionBuilder().setClientMinorVersion(0);

		if (_password != null)
			messageb.getConnectBuilder().setPassword(_password);
		if (_userName != null)
			messageb.getConnectBuilder().setUsername(_userName);
		if (_objectId != null) {
			messageb.getConnectBuilder().setClientId(_objectId);
			_objectIdSet = true;
		}

		UPMQCommand response = (UPMQCommand) syncSendPacket(new UPMQCommand(messageb, null, null));
		Protocol.Connected connected = response.processConnected();

		_connectionMetaData = new UPMQConnectionMetaData(messageb.getConnect().getClientVersion(), connected.getServerVersion());

		log.debug("+conn: " + _objectId + " (client version: " + _connectionMetaData.getJMSVersion() + ", server version: " + _connectionMetaData.getProviderVersion() + ")");
	}

	@Override
	public String getClientID() throws JMSException {
		synchronized (lock) {
			checkClosed();

			return _objectId;
		}
	}

	@Override
	public void setClientID(String clientID) throws JMSException {
		if (clientID == null || "".equals(clientID))
			return;

		synchronized (lock) {
			checkClosed();

			if (_objectIdSet) {
				throw new javax.jms.IllegalStateException("The client ID has already been set");
			}
			if (!_sessions.isEmpty()) {
				throw new javax.jms.IllegalStateException("The client ID cannot be set after a session has been created");
			}
			if (_exceptionListener != null) {
				throw new javax.jms.IllegalStateException("The client ID cannot be set after an exception listener is set");
			}
			if (_objectIdLocked) {
				throw new javax.jms.IllegalStateException("The client ID cannot be set after the connection is started");
			}

			Protocol.ProtoMessage.Builder messageb = Protocol.ProtoMessage.newBuilder();
			messageb.setObjectId(_objectId);

			messageb.getClientInfoBuilder().setReceiptId(_objectId);
			messageb.getClientInfoBuilder().setNewClientId(clientID);
			messageb.getClientInfoBuilder().setOldClientId(_objectId);

			UPMQCommand response = (UPMQCommand) syncSendPacket(new UPMQCommand(messageb, null, null));
			response.processReceipt();

			_objectId = clientID;
			_objectIdSet = true;
		}
	}

	@Override
	public ConnectionMetaData getMetaData() throws JMSException {
		synchronized (lock) {
			checkClosed();
			return _connectionMetaData;
		}
	}

	@Override
	public void setExceptionListener(ExceptionListener listener) throws JMSException {
		synchronized (lock) {
			checkClosed();
			_exceptionListener = listener;
		}
	}

	@Override
	public ExceptionListener getExceptionListener() throws JMSException {
		synchronized (lock) {
			checkClosed();
			return _exceptionListener;
		}
	}

	@Override
	public void start() throws JMSException {
		synchronized (lock) {
			checkClosed();

			_isStarted.set(true);
			_isStoped.set(false);
			_isClosed.set(false);

			_objectIdLocked = true;

			for (Iterator<UPMQSession> iterator = _sessions.iterator(); iterator.hasNext();) {
				final UPMQSession session = iterator.next();
				session.start();
			}
		}
	}

	@Override
	public void stop() throws JMSException {
		synchronized (lock) {
			checkClosed();

			_objectIdLocked = true;

			for (Iterator<UPMQSession> iterator = _sessions.iterator(); iterator.hasNext();) {
				final UPMQSession session = iterator.next();
				session.stop();
			}

			_isStoped.set(true);
			_isStarted.set(false);
		}
	}

	@Override
	public void close() throws JMSException {

		synchronized (lock) {
			if (_isClosed.get() == true)
				return;

			/*
			if (_objectIDSet) {
				_objectId = null;
			}
			*/

			for (Iterator<UPMQSession> iterator = _sessions.iterator(); iterator.hasNext();) {
				final UPMQSession session = iterator.next();
				iterator.remove();
				session.close();
			}

			Protocol.ProtoMessage.Builder messageb = Protocol.ProtoMessage.newBuilder();
			messageb.setObjectId(_objectId);

			messageb.getDisconnectBuilder().setReceiptId(_objectId);
			messageb.getDisconnectBuilder().setClientId(_objectId);

			UPMQCommand response = (UPMQCommand) syncSendPacket(new UPMQCommand(messageb, null, null));
			response.processReceipt();

			_isStarted.set(false);
			_isStoped.set(true);
			_isClosed.set(true);

			ServiceSupport.dispose(this.transport);

			log.debug("-conn: " + _objectId);
		}
	}

	@Override
	public Session createSession(boolean transacted, int acknowledgeMode) throws JMSException {
		synchronized (lock) {
			checkClosed();
			final UPMQSession session = new UPMQSession(this, transacted, acknowledgeMode);
			addSession(session);
			return session;
		}
	}

	public UPMQSession _createSession(boolean transacted, int acknowledgeMode) throws JMSException {
		synchronized (lock) {
			checkClosed();
			final UPMQSession session = new UPMQSession(this, transacted, acknowledgeMode);
			addSession(session);
			return session;
		}
	}

	public UPMQMessageConsumer _createConnectionConsumer(ServerSessionPool sessionPool, Destination destination, String messageSelector, String subscription, boolean NoLocal,
			UPMQMessageConsumer.UPMQConsumer type) throws JMSException {
		checkClosed();

		if (destination == null)
			throw new InvalidDestinationException("Don't understand null destinations");

		Session session = null;
		UPMQMessageConsumer consumer = null;

		switch (type) {
		case CONSUMER:
			session = createSession(false, Session.AUTO_ACKNOWLEDGE);
			consumer = (UPMQMessageConsumer) session.createConsumer(destination, messageSelector, NoLocal);
			consumer.setServerSessionPool(sessionPool);
			consumer.setMessageListener(consumer);
			return consumer;
		case DURABLE_CONSUMER:
			session = createSession(false, Session.AUTO_ACKNOWLEDGE);
			consumer = (UPMQMessageConsumer) session.createDurableSubscriber((Topic) destination, subscription, messageSelector, NoLocal);
			consumer.setServerSessionPool(sessionPool);
			consumer.setMessageListener(consumer);
			return consumer;
		case BROWSER:
		default:
			//FIXME
			throw new JMSException("invalid state");
		}
	}

	public ConnectionConsumer createConnectionConsumer(Topic topic, String messageSelector, ServerSessionPool sessionPool, int maxMessages) throws JMSException {
		checkTopicConnection();
		return _createConnectionConsumer(sessionPool, topic, messageSelector, null, false, UPMQMessageConsumer.UPMQConsumer.CONSUMER);
	}

	public ConnectionConsumer createConnectionConsumer(Queue queue, String messageSelector, ServerSessionPool sessionPool, int maxMessages) throws JMSException {
		checkQueueConnection();
		return _createConnectionConsumer(sessionPool, queue, messageSelector, null, false, UPMQMessageConsumer.UPMQConsumer.CONSUMER);
	}

	@Override
	public ConnectionConsumer createConnectionConsumer(Destination destination, String messageSelector, ServerSessionPool sessionPool, int maxMessages) throws JMSException {
		return _createConnectionConsumer(sessionPool, destination, messageSelector, null, false, UPMQMessageConsumer.UPMQConsumer.CONSUMER);
	}

	@Override
	public ConnectionConsumer createDurableConnectionConsumer(Topic topic, String subscriptionName, String messageSelector, ServerSessionPool sessionPool, int maxMessages) throws JMSException {
		checkTopicConnection();
		return _createConnectionConsumer(sessionPool, topic, null, subscriptionName, false, UPMQMessageConsumer.UPMQConsumer.DURABLE_CONSUMER);
	}

	protected void checkClosed() throws JMSException {
		if (_isClosed.get()) {
			throw new javax.jms.IllegalStateException("Cannot perform operation - connection has been closed");
		}
	}

	private void checkTopicConnection() throws IllegalStateException {
		if (this instanceof QueueConnection || this instanceof XAQueueConnection)
			throw new javax.jms.IllegalStateException("Cannot perform operation");
	}

	private void checkQueueConnection() throws IllegalStateException {
		if (this instanceof TopicConnection || this instanceof XATopicConnection)
			throw new javax.jms.IllegalStateException("Cannot perform operation");
	}

	public boolean isStarted() {
		return _isStarted.get();
	}

	protected boolean isStoped() {
		return _isStoped.get();
	}

	protected boolean isClosed() {
		return _isClosed.get();
	}

	public boolean isAlive() {
		return (_isStarted.get() && !_isStoped.get() && !_isClosed.get());
	}

	protected void addSession(final UPMQSession session) throws JMSException {
		synchronized (lock) {
			_sessions.add(session);
		}
	}

	public void removeSession(final UPMQSession session) {
		synchronized (lock) {
			_sessions.remove(session);
		}
	}

	public void addDispatcher(String objectId, UPMQDispatcher dispatcher) {
		dispatchers.put(objectId, dispatcher);
	}

	public void removeDispatcher(String objectId) {
		dispatchers.remove(objectId);
	}

	//-----	

	@Override
	public void onCommand(Object object) {
		final UPMQCommand command = (UPMQCommand) object;

		UPMQDispatcher dispatcher = dispatchers.get(command.getObjectId());
		if (dispatcher != null) {
			dispatcher.dispatch(command);
//						log.debug("#incoming message");
			//4bas
//						if (command._headerMessage.getProtoMessageTypeCase() == Protocol.ProtoMessage.ProtoMessageTypeCase.MESSAGE) {
//							System.out.println("  to local queue: " + command._headerMessage.getMessage().getMessageId());
//						}
		}
	}

	@Override
	public void onException(IOException error) {
		// TODO Auto-generated method stub
		log.debug("onException: " + error.getMessage());
	}

	@Override
	public void transportInterupted() {
		// TODO Auto-generated method stub
		log.debug("transportInterupted");
	}

	@Override
	public void transportResumed() {
		// TODO Auto-generated method stub
		log.debug("transportResumed");
	}

	//-----

	public void asyncSendPacket(Command command) throws JMSException {
		if (isClosed()) {
			throw new ConnectionClosedException();
		} else {
			doAsyncSendPacket(command);
		}
	}

	private void doAsyncSendPacket(Command command) throws JMSException {
		try {
			this.transport.oneway(command);
		} catch (IOException e) {
			throw JMSExceptionSupport.create(e);
		}
	}

	public Response syncSendPacket(Command command) throws JMSException {
		if (isClosed()) {
			throw new ConnectionClosedException();
		} else {

			try {
				Response response = (Response) this.transport.request(command);
				if (response.isException()) {
					ExceptionResponse er = (ExceptionResponse) response;
					if (er.getException() instanceof JMSException) {
						throw (JMSException) er.getException();
					} else {
						if (isClosed()) {// || closing.get()
							log.debug("Received an exception but connection is closing");
						}
						JMSException jmsEx = null;
						try {
							jmsEx = JMSExceptionSupport.create(er.getException());
						} catch (Throwable e) {
							log.error("Caught an exception trying to create a JMSException for " + er.getException(), e);
						}
						//dispose of transport for security exceptions
						//TODO:
						/*
						if (er.getException() instanceof SecurityException && command instanceof ConnectionInfo){
						    Transport t = this.transport;
						    if (null != t){
						        ServiceSupport.dispose(t);
						    }
						}
						*/
						if (jmsEx != null) {
							throw jmsEx;
						}
					}
				}
				return response;
			} catch (IOException e) {
				throw JMSExceptionSupport.create(e);
			}
		}
	}

	public void syncSendPacket(final Command command, final AsyncCallback onComplete) throws JMSException {
		if (onComplete == null) {
			syncSendPacket(command);
		} else {
			if (isClosed()) {
				throw new ConnectionClosedException();
			}
			try {
				this.transport.asyncRequest(command, new ResponseCallback() {
					@Override
					public void onCompletion(FutureResponse resp) {
						Response response;
						Throwable exception = null;
						try {
							response = resp.getResult();
							if (response.isException()) {
								ExceptionResponse er = (ExceptionResponse) response;
								exception = er.getException();
							}
						} catch (Exception e) {
							exception = e;
						}
						if (exception != null) {
							if (exception instanceof JMSException) {
								onComplete.onException((JMSException) exception);
							} else {
								if (isClosed()) {// || closing.get()
									log.debug("Received an exception but connection is closing");
								}
								JMSException jmsEx = null;
								try {
									jmsEx = JMSExceptionSupport.create(exception);
								} catch (Throwable e) {
									log.error("Caught an exception trying to create a JMSException for " + exception, e);
								}
								// dispose of transport for security exceptions on connection initiation
								//TODO:
								/*
								if (exception instanceof SecurityException && command instanceof ConnectionInfo){
								    Transport t = transport;
								    if (null != t){
								        ServiceSupport.dispose(t);
								    }
								}
								*/
								if (jmsEx != null) {
									onComplete.onException(jmsEx);
								}
							}
						} else {
							onComplete.onSuccess();
						}
					}
				});
			} catch (IOException e) {
				throw JMSExceptionSupport.create(e);
			}
		}
	}

	public Response syncSendPacket(Command command, int timeout) throws JMSException {
		if (isClosed()) {// || closing.get()
			throw new ConnectionClosedException();
		} else {
			return doSyncSendPacket(command, timeout);
		}
	}

	private Response doSyncSendPacket(Command command, int timeout) throws JMSException {
		try {
			Response response = (Response) (timeout > 0 ? this.transport.request(command, timeout) : this.transport.request(command));
			if (response != null && response.isException()) {
				ExceptionResponse er = (ExceptionResponse) response;
				if (er.getException() instanceof JMSException) {
					throw (JMSException) er.getException();
				} else {
					throw JMSExceptionSupport.create(er.getException());
				}
			}
			return response;
		} catch (IOException e) {
			throw JMSExceptionSupport.create(e);
		}
	}

}
