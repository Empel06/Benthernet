# Server Documentation

## Overview
This document explains the inner workings of the basketball match server with extended features. The server listens for client requests, processes basketball game guesses and slot machine spins, manages player scores, and sends responses using ZeroMQ over the Benternet network.

## How the Server Works

### 1. Initialization
The server starts by creating a **ZeroMQ context**, which manages sockets and network communication:
```cpp
zmq::context_t context{1};
```

### 2. Setting Up Sockets
The server uses two ZeroMQ sockets:
- **PUSH socket** (connects to Benternet on port `24041`): Sends responses back to clients.
- **SUB socket** (connects to Benternet on port `24042`): Receives requests from clients.

```cpp
zmq::socket_t push_socket{context, zmq::socket_type::push};
push_socket.connect("tcp://benternet.pxl-ea-ict.be:24041");

zmq::socket_t sub_socket{context, zmq::socket_type::sub};
sub_socket.connect("tcp://benternet.pxl-ea-ict.be:24042");

sub_socket.set(zmq::sockopt::subscribe, "Emiel>BasketbalGame?>");
sub_socket.set(zmq::sockopt::subscribe, "Emiel>BasketbalMachine?>");
```

### 3. Handling Requests
The server listens for two types of requests:
- **BasketbalGame:** The client sends a guess (1-10) to compete against a random target. If close enough, they score a point.
- **BasketbalMachine:** The client spins a slot machine with basketball-themed actions. Matching spins yield points.

### 4. Processing Logic

- For **BasketbalGame**, the server compares the guess with a random target number. If the difference is ≤ 1, the player earns a point.
- For **BasketbalMachine**, players need at least 2 points to spin. Three identical actions yield 10 points, two identical yield 3 points, else zero.
- The server keeps track of scores in a leaderboard and updates clients when scores change.

### 5. Sending Responses
The server sends messages to clients using the PUSH socket with topics like:
- `Emiel>BasketbalGame!>` for game results
- `Emiel>BasketbalMachine!>` for slot machine results
- `Emiel>Leaderboard!>` for leaderboard updates

### Example Message Formats
- Request: `Emiel>BasketbalGame?>PlayerName>7>`
- Response: `Emiel>BasketbalGame!>PlayerName>Score! You earned a point. Total: 5 points.`
- Slot spin: `Emiel>BasketbalMachine!>PlayerName>Spin: DRIBBLE - PASS - SHOOT => Perfect play! You win 10 points! (-2 bet, +10 win). Total: 13 points.`
- Leaderboard: `Emiel>Leaderboard!>Alice:12|Bob:10|Charlie:7`

## Network Overview
The server communicates with the Benternet using the following pattern:

```
Client (SUB) <---> Benternet Server (PUB on 24042)
Client (PUSH) ---> Benternet Server (PULL on 24041)
```

## Example Console Output
```
🎮 BasketbalService gestart...
[Ontvangen] Emiel>BasketbalGame?>Alice>7>
[BasketbalGame] Alice: kracht=7, doel=6
[Ontvangen] Emiel>BasketbalMachine?>Bob>
[BasketbalMachine] Bob: DRIBBLE - DRIBBLE - DRIBBLE => 🏆 Perfecte play! Je wint 10 punten!
[Leaderboard] Verzonden leaderboard update
```