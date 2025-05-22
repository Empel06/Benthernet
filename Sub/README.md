# Server Documentation

## Overview

This document explains the inner workings of the basketball match server. The server listens for client requests, processes basketball game strength guesses and match winner predictions, manages player scores, and sends responses using ZeroMQ over the Benternet network.

## How the Server Works

### 1. Initialization

The server starts by creating a **ZeroMQ context**, which manages sockets and network communication:

```cpp
zmq::context_t context{1};
```

### 2. Setting Up Sockets

The server uses two ZeroMQ sockets:

* **PUSH socket** (connects to Benternet on port `24041`): Sends responses back to clients.
* **SUB socket** (connects to Benternet on port `24042`): Receives requests from clients.

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

* **BasketbalGame:** The client sends a strength guess between 1-10. If the guess is within 1 of a randomly generated target, the player earns 1 point.
* **BasketbalMachine:** The client guesses the winner of a randomly generated match between two basketball teams. Correct guesses reward 5 points but require spending 2 points to play.

### 4. Processing Logic

* **BasketbalGame:**

  * A random target strength (1-10) is generated.
  * If the player's guess is within ±1 of the target, 1 point is awarded.
  * A message with success/failure is sent back to the client.

* **BasketbalMachine:**

  * The player must have at least 2 points to participate.
  * A match is generated between two random teams with random scores.
  * The player guesses the winning team.
  * If correct, 5 points are awarded (net +3 due to 2-point cost).
  * If incorrect, no points are returned (net -2).

* **Leaderboard:**

  * The server keeps a map of player names to scores.
  * Any score update triggers a new leaderboard broadcast.

### 5. Sending Responses

The server uses the PUSH socket to send responses:

* `Emiel>BasketbalGame!>`: Response to strength guessing.
* `Emiel>BasketbalMachine!>`: Response to match prediction.
* `Emiel>Leaderboard!>`: Broadcast of updated leaderboard.

### Message Formats

* Request (Game): `Emiel>BasketbalGame?>PlayerName>7>`

* Response (Game): `Emiel>BasketbalGame!>PlayerName>Score! Je hebt een punt gescoord. Totaal: 5 punten.`

* Request (Machine): `Emiel>BasketbalMachine?>PlayerName>TeamName>`

* Response (Correct): `Emiel>BasketbalMachine!>PlayerName>Wedstrijd: Lakers 112 - Celtics 108. Je gokte correct! +5 punten. Totaal: 9 punten.`

* Response (Incorrect): `Emiel>BasketbalMachine!>PlayerName>Wedstrijd: Lakers 112 - Celtics 108. Helaas, je gok was fout. Je verloor 2 punten inzet. Totaal: 3 punten.`

* Leaderboard: `Emiel>Leaderboard!>Alice:12|Bob:10|Charlie:7`

### Network Overview

The server communicates with the Benternet using this pattern:

```
Client (SUB) <---> Benternet Server (PUB on 24042)
Client (PUSH) ---> Benternet Server (PULL on 24041)
```

### Example Console Output

```
BasketbalService gestart...
[Ontvangen] Emiel>BasketbalGame?>Alice>7>
[BasketbalGame] Alice: kracht=7, doel=6
[Ontvangen] Emiel>BasketbalMachine?>Bob>Celtics>
[BasketbalMachine] Bob: gokte op celtics, winnaar: lakers
[Leaderboard] Verzonden leaderboard update
```

---

This concludes the documentation for the basketball server service using ZeroMQ and Benternet.
