# Client Documentation

## Overview

Deze documentatie beschrijft de functionaliteit van de clientapplicatie voor het spelen van de **BasketbalGame** en de **BasketbalMachine** via **Benternet** met gebruik van **ZeroMQ** en een **Tkinter GUI**.

## Vereisten

* Python 3
* `pyzmq` bibliotheek (`pip install pyzmq`)
* `tkinter` (standaard in Python op de meeste systemen)

## Network Overview

De client maakt verbinding met de Benternet ZeroMQ-endpoints:

* **PUSH socket** stuurt berichten naar de server via poort `24041`.
* **SUB socket** ontvangt berichten van de server via poort `24042`.

## Functionaliteit

* Gebruikers kunnen:

  * Een naam invoeren.
  * Een krachtgetal tussen 1 en 10 invoeren en meespelen in de **BasketbalGame**.
  * Een team kiezen ("Lakers" of "Celtics") en gokken op de winnaar in de **BasketbalMachine** (vereist 2 punten).
  * Het actuele leaderboard opvragen en bekijken.

* De client abonneert zich op de volgende topics:

  * `Emiel>BasketbalGame!>Naam>`
  * `Emiel>BasketbalMachine!>Naam>`
  * `Emiel>Leaderboard!>`

* Alle serverresponses worden weergegeven in de GUI. De leaderboard-popup toont een gesorteerde scorelijst.

## GUI Overzicht

* **Spelerinvoer:** naamveld
* **BasketbalGame:** getalinvoer (1–10) + knop "Speel BasketbalGame"
* **BasketbalMachine:** teaminvoer + knop "Speel BasketbalMachine"
* **Leaderboard:** knop "Toon Leaderboard"
* **Output Box:** toont serverreacties en status

## Berichtenstructuur

### Verzoeken (van client naar server)

* BasketbalGame:

  ```
  Emiel>BasketbalGame?>Naam>Kracht>
  ```
* BasketbalMachine:

  ```
  Emiel>BasketbalMachine?>Naam>Team>
  ```

### Antwoorden (van server naar client)

* BasketbalGame:

  ```
  Emiel>BasketbalGame!>Naam>Resultaattekst
  ```
* BasketbalMachine:

  ```
  Emiel>BasketbalMachine!>Naam>Wedstrijd: Team1 Score - Team2 Score. ...
  ```
* Leaderboard:

  ```
  Emiel>Leaderboard!>Naam1:Score1|Naam2:Score2|...
  ```

## Voorbeeldoutput

```
[BasketbalGame] Gok verzonden voor Alice met getal 7...
[BasketbalMachine] Wedstrijd gok verzonden voor Bob op team Celtics...

Emiel>BasketbalGame!>Alice>Score! Je hebt een punt gescoord. Totaal: 5 punten.
Emiel>BasketbalMachine!>Bob>Wedstrijd: Lakers 102 - Celtics 97. Je gokte correct! +5 punten. Totaal: 8 punten.
Emiel>Leaderboard!>Alice:5|Bob:8|Charlie:3
```

## Opstarten

Voer de client uit met:

```bash
python client.py
```

Een grafische interface zal openen waarin gebruikers kunnen spelen en scores volgen.
