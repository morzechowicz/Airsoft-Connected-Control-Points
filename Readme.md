## ESP32-Based Control Point Game

## What is this?

It's an ESP32-based project that utilizes LoRa for wireless communication.

---

## What does it do?

This project implements a simple "control point" game mode, similar to *Conquest* in Battlefield games. Two teams compete to control points on a map, earning score points for each control point they hold.

---

## How does it work?

- Each control point tracks which team currently owns it.
- Teams earn points over time for each control point they control.
- To capture a point, a team must hold down their team button (the large, colorful one) for a configurable amount of time (1–30 seconds).
- The game ends when either:
    - The timer runs out, **or**
    - One team reaches the target score (both are configurable).
- Final scores are displayed on all devices at the end of the game.

---

## How to configure

### old:
1. **Power on** all control points you want to use and place them in your chosen locations (not necessarily in that order).
2. **Configure settings** (TO DO: Add configuration instructions).
3. After confirming the configuration, the game timer will start.
4. When the timer reaches zero or a team wins, the final scores will be shown on all devices.

### new:
There is a new configuration app that works over BLE.
---

## Planned:

1. Memory for the main node. No game will be lost due to sudden power loss or unfair players.

*More detailed configuration instructions coming soon!*




# Instrukcja dla graczy - Skrócona:
1. Aby przejąć punkt przytrzymaj przycisk w kolorze swojej drużyny.
2. Punkt jest przejęty kiedy przycisk świeci. Jeżeli oba nie świecą punkt jest neutralny
3. Resztę informacji można przeczytać z ekranu.


# Instrukcja dla graczy - Rozszerzona:
0. Start gry oznaczany jest sygnałem audio. Przed startem wyświetlane jest odliczanie. W trakcie gry na ekranie widać czas do końca gry oraz wynik każdej ze stron.
1. W grze znajdują/e się jeden/dwa punkty.
2. Każdy punkt ma 3 stany : przejęty przez niebieskich, przejęty przez żółtych, neutralny.
3. Kiedy punkt jest przejęty przez drużynę tej drużynie doliczane są co minutę punkty równe ilości przejętych punktów.
4. PRZEJĘCIE PUNKTU: przytrzymanie przycisku w kolorze drużyny przez ustalony czas. Na ekranie wyświetla się pasek postępu przejmowania. Punkt wydaję 3 sygnały dźwiękowe jeśli zostanie przejęty.
5. Aktualny stan punktu wyświetlany jest na ekranie oraz przycisk świeci w kolorze drużyny jeżeli został przejęty. Punkt jest neutralny jeżeli żaden z przycisków nie świeci.
6. Koniec gry sygnalizowany jest przez sygnał dźwiękowy. na ekranie wyświetlany jest zwycięzca.

# Format informacyjny ekranu.
## W trakcie gry:

T: 32 min BLUE - Czas do końca 32 minuty. Nazwa drużyny do której należy punkt. <br>
B: 3 | Y: 5 - wynik dla niebieskich(B) 3 punkty, dla żółtych(Y) 5 punktów

## W trakcie przejmowania:

TEAM BLUE - Nazwa przejmującej drużyny <br>
[#####          ] - Pasek postępu przejmowania. Po wypełnieniu całej dolnej linii przez # = punkt jest przejęty.

## Koniec gry:

BLUE WON - Wygrała drużyna niebieskich. <br>
B: 15 | Y: 5 - Wynik dla niebieskich(B) 15, dla żółtych(Y) 5.


# Notatki:
1. instrukcje skróconą można wydrukować i przykleić.
2. Dla bardzo opornych można zrobić instrukcję obrazkową XD

## Scenariusz:

# Wersja krótka:

Cel gry zdobyć więcej punktów niż przeciwnik.

Jak zdobywać punkty?

Na terenie gry znajdują sie obszary kontrolne które trzeba przejąć.
Przejmowanie odbywa się na zasadzie przytrzymania przycisków w kolorze twojej drużyny do usłyszanie sygnału.
(przejęty punkt oznaczony jest zaświeconym przyciskiem i napisem na ekranie.)

Co minutę naliczane są punkty na podstawie ilości kontrolowanych obszarów.

koniec następuje kiedy jedna z drużyn uzbiera max punktów lub skończy się czas.

# Respy,trafienia,medycy:

Resp w na bazie twojej drużyny. Wchodziw wychodzisz / resp w parach zawsze wychodzą tylko 2 osoby / resp po x minut

Medycy. NIE / tak 

Każde trafienie w ciebie,ubranie i oporządzenie się liczy.
Dostałeś => krzyczysz "dostałem/hit" => zakładasz na siebie szmatę trupa (kamizelka odblaskowa) => schodzisz zna resp.
Trafienie w broń eliminuje broń do czasu twojego respa.

założona szmata trupa = nie jesteś w grze
brak szmaty = jesteś w grze

Medyk. Każdy może leczyć. leczenie polega na trzymaniu oburącz leczonej osoby przez 30 sekund. uleczony może się poruszać dopiero jak schowa szmatę trupa.

ranny nie może się przemieszczać. Jeżeli się przemieściłes jako ranny z automatu umierasz i musisz iść na resp

Dostałeś => krzyczysz "dostałem/hit" => zakładasz na siebie szmatę trupa (kamizelka odblaskowa) => czekasz na medyka do 10minut => jeżeli nikt cię nie uleczy wracasz na resp

# Limity, zgody piro:

Piro: ZAKAZ NULL ZERO jak kogoś złapię na rzucaniu piro to nogi z dupy powyrywam

1.2j każda odległość full auto i pojedyńczy
Limity: 1.9j full auto i pojedynczy do minimum 10 metrów od przeciwnika.

Dla osób poniżej 18 wymagany opiekun lub pisemna zgoda rodzica