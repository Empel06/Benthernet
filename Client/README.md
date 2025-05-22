# Client Documentation

## Overview
This documentation describes the functionality of the client application for playing the BasketbalGame and SlotMachine over Benternet using ZeroMQ.

## How-to Build

### Requirements
- Python 3
- pyzmq library
- tkinter (for GUI)

## Network Overview
The client connects to the Benternet ZeroMQ endpoints:

- PUSH socket sends requests on port `24041`.
- SUB socket receives responses on port `24042`.

## Functionality

- The client subscribes to leaderboard and personal response topics.
- The client allows the user to enter their name and a guess number for the BasketbalGame.
- The client can initiate a slot machine spin if the player has enough points.
- Responses and leaderboard updates are shown in a GUI.

## Using the Client

1. Enter your player name.
2. Enter a guess (1-10) for the BasketbalGame and press "Speel BasketbalGame".
3. Press "Speel SlotMachine" to spin the basketball-themed slot machine (requires ≥ 2 points).
4. Press "Toon Leaderboard" to view the current player rankings.

## Communication Protocol

### Requests
- BasketbalGame guess: `Emiel>BasketbalGame?>PlayerName>Guess>`
- Slot machine spin: `Emiel>BasketbalMachine?>PlayerName>`

### Responses
- BasketbalGame results: `Emiel>BasketbalGame!>PlayerName>...`
- Slot machine results: `Emiel>BasketbalMachine!>PlayerName>...`
- Leaderboard updates: `Emiel>Leaderboard!>Player1:Score|Player2:Score|...`

## Example Output
```
[BasketbalGame] Gok verzonden...
[SlotMachine] Draai gestart...
Emiel>BasketbalGame!>Alice>Score! Je hebt een punt gescoord. Totaal: 5 punten.
Emiel>BasketbalMachine!>Alice>Spin: DRIBBLE - PASS - PASS => 🔥 Net play! Je wint 3 punten! (-2 inzet, +3 winst). Totaal: 6 punten.
Emiel>Leaderboard!>Alice:6|Bob:4|Charlie:3
```