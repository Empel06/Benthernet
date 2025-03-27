# Client Documentation

## Overview
This documentation describes the functionality of the client application that uses ZeroMQ to request and receive basketball match data via the Benternet network. Additionally, it explains how to write your own client that works with the existing server code.

## How-to Build

### Requirements
- C++ compiler (e.g., g++)
- ZeroMQ library
- CMake (optional for project management)

## Network Overview

The client communicates with Benternet via ZeroMQ using a PUSH socket to send requests and a SUB socket to receive match data.

```
+--------------+     PUSH      +-------------------------+
|   Client    | ------------> | Benternet Server (PULL) |
|             |               |                         |
|   (ZMQ)     |               |                         |
|             | <------------ | Benternet Server (PUB)  |
+--------------+     SUB       +-------------------------+
```

## Available Services

| Service                        | Description                                   |
|--------------------------------|----------------------------------------------|
| `service>basketbal?>`          | Requests a new basketball match             |
| `service>basketbal!>`          | Receives a message with match information   |

## Functionality

1. The client connects to the Benternet server via a PUSH socket on port `24041`.
2. The client connects to the Benternet server via a SUB socket on port `24042` and subscribes to the topic `service>basketbal!>`.
3. After pressing ENTER, a message `service>basketbal?>` is sent via the PUSH socket.
4. The client then receives match information via the SUB socket and displays it in the console.

## Writing Your Own Client

If you want to write your own client that works with the existing server, follow these steps:

1. **Create a ZeroMQ context:**
```cpp
zmq::context_t context{1};
```
2. **Create a PUSH socket to send requests:**
```cpp
zmq::socket_t push_socket{context, zmq::socket_type::push};
push_socket.connect("tcp://benternet.pxl-ea-ict.be:24041");
```
3. **Create a SUB socket to receive responses:**
```cpp
zmq::socket_t sub_socket{context, zmq::socket_type::sub};
sub_socket.connect("tcp://benternet.pxl-ea-ict.be:24042");
std::string sub_topic = "service>basketbal!>";
sub_socket.setsockopt(ZMQ_SUBSCRIBE, sub_topic.c_str(), sub_topic.length());
```
4. **Send a request:**
```cpp
std::string push_message = "service>basketbal?>";
push_socket.send(zmq::buffer(push_message), zmq::send_flags::none);
```
5. **Receive and process the response:**
```cpp
zmq::message_t reply;
sub_socket.recv(reply, zmq::recv_flags::none);
std::string match = reply.to_string();
std::cout << "Received match: " << match << std::endl;
```

## Example Output
```
Press ENTER to send a new request...
Request sent: service>basketbal?>
Received match: Real Madrid 87 - 92 FC Barcelona
```
