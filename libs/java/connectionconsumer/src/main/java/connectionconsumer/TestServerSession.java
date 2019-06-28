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
 
package connectionconsumer;
                                                                           
import javax.jms.Session;
import javax.jms.Message;
import javax.jms.TopicSession;
import javax.jms.TopicConnection;
import javax.jms.ServerSession;
import javax.jms.MessageListener;
import javax.jms.JMSException;
                                                                           
/**
   This is a very simple implementation of a server session,
   which creates a new thread for performing asynchronous message
   processing each time it is called.
 */
                                                                           
public class TestServerSession implements ServerSession
{
    private final TopicConnection _conn;
    private       TopicSession    _topicSession;
                                                                           
    TestServerSession(TopicConnection conn)
    {
       _conn = conn;
    }
                                                                           
    // get or create the session for this server session
    // when creating a session a message listener is set
    public synchronized Session getSession() throws JMSException
    {
       if (_topicSession == null) {
          _topicSession = _conn.createTopicSession(false, Session.AUTO_ACKNOWLEDGE);
          MessageListener listener;
          listener = new MyMessageListener(_topicSession);
          _topicSession.setMessageListener(listener);
       }
                                                                          
       return _topicSession;
    }
                                                                           
    public void start() throws JMSException
    {
       Thread t = new Thread(_topicSession);
       t.start();
    }
                                                                           
    // a simple message listener that counts 100 messages
    static class MyMessageListener implements MessageListener
    {
       private final TopicSession _topicSession;
                                                                          
       MyMessageListener(TopicSession topicSession)
       {
          _topicSession = topicSession;
       }
                                                                          
       // must be thread-safe
       public void onMessage(Message msg)
       {

          System.out.print(".");

                                                                         
          if (++_msgCount == 100)
          {
             System.out.println("done");
             System.exit(0);
          }
       }
    }
                                                                           
    static int _msgCount = 0;
}