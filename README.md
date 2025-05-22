
# BasketbalArena Project - Eén gecombineerde service met GUI-client

Welkom bij **BasketbalArena**! Dit project bestaat uit één gecombineerde service met twee ZeroMQ-gebaseerde minigames en een leaderboard:

- **BasketbalGame** – Raad een getal tussen 1 en 10 en verdien punten (mits correcte gok).
- **BasketbalMachine** – Betaal 2 punten per draai, win tot 10 punten op basis van actiescombinatie.

Beide spellen delen een **gezamenlijke puntenbalans per spelernaam**, waardoor er één consistente spelervaring is.

---

## Overzicht

Spelers communiceren via ZeroMQ (PUSH/SUB) met de BasketbalArena-service die op Benternet draait. De speler:

1. Voert zijn naam in via de GUI-client.
2. Stuurt een verzoek naar BasketbalGame of BasketbalMachine.
3. Krijgt een gepersonaliseerd antwoord via de SUB-socket met puntenupdate.

Alle punten worden per spelernaam bijgehouden in een `unordered_map<std::string, int>` aan de servicezijde.

---

## BasketbalGame Flow

```mermaid
flowchart TD
    Start --> NaamInvoer[Speler voert naam in]
    NaamInvoer --> Raad[Raad getal tussen 1 en 10]
    Raad --> SendGuess[Client stuurt Emiel>BasketbalGame?>Naam>Gok>]
    SendGuess --> Service[BasketbalArena-service ontvangt gok]
    Service --> CheckRange[Controleer of getal geldig is]
    CheckRange --> Compare[Genereer random getal en vergelijk]
    Compare --> Resultaat[Correct? Ja/nee + update punten]
    Resultaat --> SendBack[Stuur antwoord via Emiel>BasketbalGame!>Naam>...>]
    SendBack --> ClientToon[Client toont resultaat aan speler]
```

---

## BasketbalMachine Flow

```mermaid
flowchart TD
    Start --> NaamInvoer[Speler voert naam in]
    NaamInvoer --> ClickPlay[Klik op 'Speel!']
    ClickPlay --> CheckPoints[Service controleert of speler minstens 2 punten heeft]
    CheckPoints --> SendSpin[Client stuurt Emiel>BasketbalMachine?>Naam>]
    SendSpin --> MachineService[BasketbalArena-service verwerkt verzoek]
    MachineService --> Rollen[Genereer 3 acties]
    Rollen --> Evaluatie[Bepaal winst: 0, 3 of 10 punten]
    Evaluatie --> Update[Pas puntenbalans aan: -2 inzet, +winst]
    Update --> SendBack[Stuur resultaat terug via Emiel>BasketbalMachine!>Naam>]
    SendBack --> ClientToon[Toon resultaat en huidige balans in GUI]
```

---

## Naam & Puntenbeheer

De gecombineerde service gebruikt:

- Eén `unordered_map<std::string, int>` om de punten per speler bij te houden.
- Punten zijn persistent zolang de service draait.
- Spelers moeten dus **dezelfde naam hergebruiken** voor continuïteit.

---

## Communicatieschema

```mermaid
sequenceDiagram
    participant Client
    participant BasketbalArenaService

    Client->>BasketbalArenaService: Emiel>BasketbalGame?>Emiel>3>
    BasketbalArenaService-->>Client: Emiel>BasketbalGame!>Emiel>Score! +1 punt>

    Client->>BasketbalArenaService: Emiel>BasketbalMachine?>Emiel>
    BasketbalArenaService-->>Client: Emiel>BasketbalMachine!>Emiel>DRIBBLE PASS SHOOT => 3 punten (netto +1)
```

---

## Bestandsoverzicht

- `service.cpp` – C++ gecombineerde BasketbalGame + BasketbalMachine service
- `client.py` – Python GUI-client voor beide spellen
- Optioneel: testscript, documentatie, oudere versies

---

## Uitvoeren

```bash
# BasketbalArena service starten (na compilatie)
service.exe

# GUI-client starten
python client.py
```

---
