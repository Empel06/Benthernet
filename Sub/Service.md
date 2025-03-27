# Server Documentation

## Overview
This document explains the inner workings of the basketball match server. The server listens for client requests, generates random basketball match data, and sends responses using ZeroMQ over the Benternet network.

## How the Server Works

### 1. Initialization
The server starts by creating a **ZeroMQ context**, which is required for managing sockets and network communication:
```cpp
zmq::context_t context{1};
```

### 2. Setting Up Sockets
The server sets up two ZeroMQ sockets:
- **PULL socket** (port `24041`): Receives client requests.
- **PUB socket** (port `24042`): Sends match data to clients.

```cpp
zmq::socket_t pull_socket{context, zmq::socket_type::pull};
pull_socket.bind("tcp://*:24041");

zmq::socket_t pub_socket{context, zmq::socket_type::pub};
pub_socket.bind("tcp://*:24042");
```

### 3. Listening for Requests
The server continuously waits for requests from clients. A request is expected to be in the format `service>basketbal?>`.

```cpp
zmq::message_t request;
pull_socket.recv(request, zmq::recv_flags::none);
std::string msg = request.to_string();
std::cout << "Received request: " << msg << std::endl;
```

### 4. Generating a Random Basketball Match
When a request is received, the server randomly selects two teams and generates a match result.
```cpp
BasketbalWedstrijd wedstrijd = genereerWedstrijd();
std::string response = "service>basketbal!>" + wedstrijd.team1 + " " + std::to_string(wedstrijd.score1) + " - " + std::to_string(wedstrijd.score2) + " " + wedstrijd.team2 + ">";
```

### 5. Sending the Response
The generated match result is then sent via the PUB socket to all subscribed clients.
```cpp
pub_socket.send(zmq::buffer(response), zmq::send_flags::none);
std::cout << "Match sent: " << response << std::endl;
```

## Network Overview
The server interacts with multiple clients through a request-response pattern over ZeroMQ:

```
+--------------+     PUSH      +-------------------------+
|   Client    | ------------> | Benternet Server (PULL) |
|             |               |                         |
|   (ZMQ)     |               |                         |
|             | <------------ | Benternet Server (PUB)  |
+--------------+     SUB       +-------------------------+
```

## Example Workflow

1. A client sends `service>basketbal?>` to request a match.
2. The server receives the request and generates a random match.
3. The server sends the result using the topic `service>basketbal!>`.
4. The client receives the match data.

### Example Console Output
```
Basketball match service running...
Received request: service>basketbal?>
Generated match: Real Madrid 87 - 92 FC Barcelona
Match sent: service>basketbal!>Real Madrid 87 - 92 FC Barcelona>
```
