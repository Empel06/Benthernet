# Benternet Project – Network Programming

## Introduction

This project is part of the course *Network Programming* and uses **Benternet**: a publish-subscribe architecture based on **ZMQ (ZeroMQ)** sockets in C++. The goal is to develop a useful, working service that communicates correctly over the Benternet network.

**Push socket**: `tcp://benternet.pxl-ea-ict.be:24041`
**Sub socket**: `tcp://benternet.pxl-ea-ict.be:24042`

## Project goal

Building a service that:
- Subscribes to specific messages via Benternet
- Logically and correctly responds to incoming messages
- Potentially also addresses other services
- Is useful for other users within the Benternet ecosystem

## Functionality

The developed service can:
- Subscribe to specific topics with string filtering
- Parse messages based on parameters
- Return appropriate responses depending on input
- Process multiple messages simultaneously (multiple topics)
- Detect possible errors and provide appropriate feedback

## Technical details

- **Language**: C++
- **Library**: ZMQ (ZeroMQ)
- **Communication**: 
- `PUSH` via `tcp://benternet.pxl-ea-ict.be:24041` 
- `SUB` via `tcp://benternet.pxl-ea-ict.be:24042`
- **Message structure**: Consistent and logical structure with easy filtering and parsing of parameters
- **Topics**: The service handles multiple topics in parallel

## Additional Documentation

- [Client Documentation](./client.md) - Explains how the client connects to the server, sends messages via the **PUSH** socket, and receives responses through the **SUB** socket.
- [Service Documentation](./service.md) - Describes how the server operates, processes client requests, and generates and sends random basketball match data.


## Usage

1. Clone this repository
2. Compile with `g++` and link the ZMQ library:
```bash
g++ CodeName.cpp -o CodeName.exe -IC:\msys64\mingw64\include -LC:\msys64\mingw64\lib -lzmq
```
3. Start the service:
```bash
./CodeName
```

or

```bash
CodeName.exe
