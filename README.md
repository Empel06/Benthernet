# Benternet Project – Netwerk Programming

## Inleiding

Dit project is onderdeel van het vak *Netwerk Programming* en maakt gebruik van **Benternet**: een publish-subscribe architectuur gebaseerd op **ZMQ (ZeroMQ)** sockets in C++. De bedoeling is om een nuttige, werkende service te ontwikkelen die correct communiceert over het Benternet-netwerk.

**Push socket**: `tcp://benternet.pxl-ea-ict.be:24041`  
**Sub socket**: `tcp://benternet.pxl-ea-ict.be:24042`

## Doel van het project

Het bouwen van een dienst (service) die:
- Subscribed op specifieke berichten via Benternet
- Logisch en correct reageert op binnenkomende berichten
- Potentieel ook andere services aanspreekt
- Nuttig is voor andere gebruikers binnen het Benternet-ecosysteem

## Functionaliteit

De ontwikkelde service kan:
- Subscriben op specifieke topics met string filtering
- Berichten parsen op basis van parameters
- Gepaste reacties terugsturen afhankelijk van input
- Meerdere berichten tegelijkertijd verwerken (meerdere topics)
- Mogelijke fouten detecteren en gepaste feedback geven

## Technische details

- **Taal**: C++
- **Bibliotheek**: ZMQ (ZeroMQ)
- **Communicatie**:
  - `PUSH` via `tcp://benternet.pxl-ea-ict.be:24041`
  - `SUB` via `tcp://benternet.pxl-ea-ict.be:24042`
- **Berichtenstructuur**: Consistente en logische structuur met eenvoudige filtering en parsing van parameters
- **Topics**: De service handelt meerdere topics parallel af

## Voorbeeldberichten

**Inkomend (SUB)**:
```
service/request/name:temperature/location:lab1
```

**Uitgaand (PUSH)**:
```
service/response/temperature/location:lab1/value:21.5
```

## Gebruik

1. Clone deze repository
2. Compileer met `g++` en link de ZMQ-bibliotheek:
   ```bash
   g++ CodeName.cpp -o CodeName.exe -IC:\msys64\mingw64\include -LC:\msys64\mingw64\lib -lzmq
   ```
3. Start de service:
   ```bash
   ./benternet_service
   ```

   or

   ```bash
   CodeName.exe
