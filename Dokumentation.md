# VOR Sender und Empfänger - Dokumentation

## Glossar / Begriffsdefinitionen

- VOR Quartal (quarter): 90° Segment des VOR
- VOR-Master: VOR-Sender, der den Nordimpuls vorgibt
- VOR-Slave(s): VOR-Sender, die dem Master folgen
- Nordimpuls: Funksignal, das den Beginn einer VOR-Umdrehung anzeigt
- Norden: bezeichnet nicht magnetisch oder geographisch Nord, sonden ist im Spielfeld definiert!
- IR-Strahl (IR beam): Umlaufender Lichtstrahl
- VOR Segmente (segments): Anzahl der umlaufenden Segmente (Anzahl IR-LEDs)


## Die VOR-Technologie
Die hier verwendete Technologie zur Positionsbestimmung ist dem sog. VOR aus der Luftfahrt angelehnt. Hierbei werden bekannt Punkte angepeilt, um seine eigene Position berechnen zu können.  
Damit nun aber keine Kompasspeilungen vorgenommen werden müssen, gibt es einen Trick: Die Funkfeuer/ Sendetürme senden ein Umlaufendes Signal mit einem relativ kleinen Abstrahlwinkel. In dieser Implementation wird das umlaufende Signal mit IR-LEDs realisiert. Um nun aus diesem umlaufenden Strahl eine Winkelinformation zu erhalten, muss es noch eine Referenz geben. Diese wird durch einen allseitig abgegebenen Nordimpuls realisiert. Nun kann die Zeit zwischen dem Nordimpuls und der Messung den Strahl gemessen werden, die proportional zum Abstrahlwinkel ist. Die Peilung ist also immer zum Empfänger hin.  
Da die Positionen der Sendetürme bekannt sind, kann aus mindestens zwei Peilungen eine Positions ermittelt werden.  
Damit auch der Fall abgedeckt ist, dass ein Hindernis die Sicht auf einen Sendeturm verdeckt, gibt es drei Sendetürme.


## Positionsberechnung

### Spielfeld und Koordinatensystem
```
              ^ Norden
              |
[T1] ------------------ [T2]
     |                |
     |                |
     |                |
     |          (x)   |
     |                |
     |                |
     |                |
(3m) |                |
 y ^ |                |
   | ------------------
   ┗--> x   [T3]
      (2m)
```

### Zuordnung Winkel - Sendeturm
Je nach dem, aus welchem Winkel (bezogen auf Norden) der Strahl eintrifft, kann dieser eindeutig zu einem Sendeturm zugeordnet werden.

a)   0°- 90°: Sender 3  
b)  90°-180°: Sender 1  
c) 180°-270°: Sender 2  
d) 270°-360°: Sender 3  

Also muss in der Software die aus der Zeit berechnete Winkelangabe dem richtigen Sendetum zugeordnet werden.

### Berechnung
Prinzipiell ergibt sich aus der Geometrie erst einmal ein LGS mit drei Gleichungen.

```
Sender 1: g_1(x) = m_1*(x)   + 3   mit: m_1 = -tan(α_1-90°)
Sender 2: g_2(x) = m_2*(x-2) + 3   mit: m_2 = -tan(α_2-90°)
      ==> g_2(x) = m_2*x - (2*m_2 + 3)
Sender 3: g_3(x) = m_3*(x-1) + 0   mit: m_3 = -tan(α_3-90°)
      ==> g_3(x) = m_2*x - (1*m_2)
```

Andererseits kann nicht angenommen werden, dass sich alle drei Strahlen an einem Punkt schneiden, da schon die Anzahl der IR-LEDs nicht für eine hohe Genauigkeit ausreicht.

Daher müssen alle möglichen Schnittpunkte einzeln berechnet werden. Bei drei Sendern also drei Schnittpunkte, wenn ein Sender verdeckt ist, nur zwei Schnittpunkte.

### Algorithmus
Zur Berechnung der Position müssen folgende Schritte durchgeführt werden:

- Zeit zwischen Nordimpuls und Empfang des IR-Strahls messen
- aus der Zeit den entsprechenden Winkel ermitteln
- aus dem Winkel (oder der Zeit) auf den Sendeturm zurückschließen
- aus dem Winkel und dem Sendeturm ergibt sich eine Geradengleichung
- mit mind. zwei Geraden lässt sich eine Position errechnen

##### Ermittelung des Winkels:
TODO

##### Berechnung der Geradenparameter:
TODO

##### Schnittpunkt zweier Geraden:

```
für Geradengleichung der Form: y(x) = m*x + b

    b_1 - b_2
x = ---------
    m_2 - m_1
    
    b_1 / m_1  -  b_2 / m_2
y = -----------------------
     1 / m_1   -   1 / m_2
```

Implementierung als C Funktion:

```
void CalcIntersection (float var_m1, float var_m2, float var_b1, float var_b2, float* var_x, float* var_y) {
  *var_x = (var_b1 - var_b2) / (var_m2 - var_m1);
  *var_y = ((var_b1 / var_m1) - (var_b2 / var_m2)) / ((1 / var_m1) - (1 / var_m2));
```

### Mittelung der Werte
TODO


## Hardware
Dieses VOR-System besteht aus drei Sendern, und zwei Empfängern. Die Sender sind dabei miteinander gekoppelt und decken die 360° nur ein Mal ab. Es können beliebig weitere Empfänger hinzugefügt werden.

### Sender Hardware
Der Sender besteht aus:

- Arduino nano
- 433 MHz Sender
- (ein oder zwei) 8-Bit Schieberegister 74HC595 ??
- (8 oder 16) IR-LEDs mit 40 kHz Modulation
- (8 oder 16) Transistoren für die LEDs
- TODO: weitere Bauelemente auflisten, Schaltplan hinzufügen

### Empfänger Hardware
Der Empfänger besteht aus

- Arduino nano
- 433 MHz Empfänger
- (8 oder 16) IR-Empfänger mit 40 kHz Modulation
- TODO: weitere Bauelemente auflisten, Schaltplan hinzufügen

### 433MHz Strecke
Der Nordimpuls wird über eine 433 MHz Funkstecke übertragen. Diese hat eine Verzögerung zwischen Sendereinganz und Empfängerausgang von 40μs.


## Schnittstelle zum Roboter
Dem Roboter werden die Positionsdaten über die Serielle Schnittstelle (UART) des Arduino übergeben.  
Das Datenformat ist folgendes:
TODO
