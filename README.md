# UPMQ

UPMQ - JMS-like message broker written with C++.

## Proto
* [UPMQ protocol (russian version)](https://github.com/ivk-jsc/broker/wiki/UPMQ.proto-%5BRU%5D) 

## Binaries
* UPMQ broker [bins/broker] - c++ message broker

## Libraries
* libupmqprotocol [libs/libupmqprotocol] - wire protocol using protobuf-3 notation
* libupmq [libs/libupmq] - c++ client library, JMS 1.1-like implementation
* libupmq [libs/java/libupmq] - java client library, JMS 1.1 implementation

## Build Requirements

UPMQ dependencies :
* Main 
  * [poco-libraries](https://github.com/pocoproject/poco)
  * [protobuf-library](https://github.com/protocolbuffers/protobuf)
* Tests
  * [google-test](https://github.com/google/googletest) 

**!** Please check odbc installation on your system if you want to use UPMQ broker with database over ODBC  
**!** Please check PostgreSQL installation on your system if you want to use UPMQ broker with POCO-1.10.0 or higher and PostgreSQL database   

## Build

Build UPMQ with [cmake](https://cmake.org)

```shell
git clone https://github.com/ivk-jsc/broker
cd broker
mkdir cmake-build-release
cd cmake-build-release
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
cmake --build . --target install
```

## Configuration

UPMQ broker [bins/broker] is configured by the broker.xml file (can be found in ```configs``` directory or in ```<install-dir>/etc``` directory, if you already installed UPMQ.

```xml
<config>
    <broker>
        <!-- TCP port -->
        <port>12345</port>
        <!-- Server name -->
        <name>broker</name>
        <http>
            <!-- Http port for Web-admin GUI-->
            <port>9090</port>
            <!-- Path to html templates -->
            <site>../share/upmq/www</site> 
        </http>
        <heartbeat>
            <send>0</send>
            <recv>0</recv>
        </heartbeat>
        <net>
            <!-- Max clients count -->
            <max-connections>1024</max-connections> 
        </net>
        <threads>
            <!-- A number of acceptor threads-->
            <accepter>8</accepter>
            <!-- A number of tcp-readers clients -->
            <reader>8</reader>
            <!-- A number of tcp-writers clients -->
            <writer>8</writer>
            <!-- A number of threads which serve subscribers -->
            <subscriber>8</subscriber>
        </threads>
        <log>
            <!-- Log level -->
            <!--log=1 - A fatal error. The application will most likely terminate. This is the highest priority.-->
            <!--log=2 - A critical error. The application might not be able to continue running successfully.-->
            <!--log=3 - An error. An operation did not complete successfully, but the application as a whole is not affected.-->
            <!--log=4 - A warning. An operation completed with an unexpected result.-->
            <!--log=5 - A notice, which is an information with just a higher priority.-->
            <!--log=6 - An informational message, usually denoting the successful completion of an operation.-->
            <!--log=7 - A debugging message.-->
            <!--log=8 - A tracing message. This is the lowest priority.-->
            <level>8</level>
            <!-- Log path-->
            <path windows="C:/ProgramData" _nix="/var/log">upmq/log</path>
            <!-- Interactive mode - use 0 for disable console output -->
            <interactive>true</interactive>
        </log>
	<sessions>
	    <!-- Maximum sessions count per connection -->
            <max-count>1024</max-count>
        </sessions>
        <subscriptions>
		    <!-- Maximum subscriptions count per destination -->
            <max-count>1024</max-count>
        </subscriptions>
        <destinations>
            <!-- Auto create destination -->
            <autocreate>true</autocreate>
            <!-- Forward message to another server by message properties -->
            <forward by-property="false"/>
            <!-- Maximum destinations count -->
            <max-count>1024</max-count>
        </destinations>
        <storage>
            <!-- DBMS properties -->
            <connection dbms="sqlite-native" pool="64" sync="false" journal-mode="WAL">
                <!-- Connection string or database name -->
                <value use-path="true">upmq.db</value>
                <!-- Database path-->
                <path windows="C:/ProgramData" _nix="../share">upmq/db</path>
            </connection>
            <!-- Persistent storage path -->
            <data windows="C:/ProgramData" _nix="../share">upmq/data</data>
        </storage>
    </broker>
</config>
```

## Examples

```examples``` directory contains ```consumer``` and ```producer``` imlpementations.

In general, you should follow these rules:

```cpp
//Create connection factory
ConnectionFactory*    factory = ConnectionFactory::create("tcp://127.0.0.1:12345");

//Create connection
cms::Connection*      connection = factory->createConnection();
//Create Session
cms::Session*         session    = connection->createSession(AUTO_ACKNOWLEDGE);
//Create destination, it can be queue or topic
cms::Destination*     queue      = session->createQueue("messages");
// Create sender or receiver
cms::MessageProducer* sender     = session->createProducer(queue);
cms::MessageConsumer* sender     = session->createConsumer(queue);

//Start connection
connection->start();

//Create message
cms::TextMessage* message = session->createTextMessage("Hello");
// Send Message
sender->send(message);
delete message;

// Receive message
message = consumer->receive();
delete message;

//Close and free all objects
consumer->close();
producer->close();
session->close();
connection->close();

delete destination;
delete consumer;
delete producer;
delete session;
delete connection;
delete connectionFactory;
```

## Videos
[Alexander Bychuk — C++ Enterprise Edition-message brokers](https://youtu.be/oZwPQte3za8) ***[RU]*
