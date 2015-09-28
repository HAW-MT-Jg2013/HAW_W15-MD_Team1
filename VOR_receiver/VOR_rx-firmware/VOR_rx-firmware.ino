// Mechatronisches Design - BYR - WS 2015 -- Semesterprojekt
// VOR receiver

// dependencies:
// TimerOne-r11 (https://code.google.com/p/arduino-timerone/downloads/list)


#include "TimerOne.h"
#include "avr/interrupt.h"

// Debug modus anschalten
//#define DEBUG_MODE

// sytem setup
const int VOR_frequency   = 50; // north impulse frequency
const int VOR_segments    = 32; // number of IR-LEDs, number of individual beams

const float T1_posX   = 0; // TODO: implementieren
const float T1_posY   = 3;
const float T2_posX   = 2;
const float T2_posY   = 3;
const float T3_posX   = 1;
const float T3_posY   = 0;

// Variablen der I/O Pins
// Pin 0 und 1 frei für Kommunikation
// Pin 9 und 10 frei wegen timer
const int IR1         = 12;
const int IR2         = 3;
const int IR3         = 4;
const int IR4         = 5;
const int IR5         = 6;
const int IR6         = 7;
const int IR7         = 8;
const int IR8         = 11;
const int north_pulse = 2;   // Funksignal (Interrupt pin)
const int status_led  = 13;


// weitere Variablen
#define ANY_IR_INPUT (digitalRead(IR1)||digitalRead(IR2)||digitalRead(IR3)||digitalRead(IR4)||digitalRead(IR5)||digitalRead(IR6)||digitalRead(IR7)||digitalRead(IR8))

// Funksignal Variablen
int radioStatus_old    = 0;
int radioStatus_new    = 0;

//Kreisfrequenz definieren
float T        = 1 / VOR_frequency;
float pi       = 3.141592;
float w        = VOR_frequency * 2 * pi;  // angepasst für Berechung in Radiant
float T25      = T / 4;       // entspricht 90°
float T50      = T / 2;       // entspricht 180°
float T75      = T * 3 / 4;   // ensprichtt 270°

int   T1_data  = 0;   // bool: ob bereits Signal von Turm 1 empfangen wurde
float T1_m     = 0;   // Steigung Turm 1
float T1_b     = 3;   // Y-Achsen Abschnitt Turm 1
int   T2_data  = 0;
float T2_m     = 0;
float T2_b     = 0;
int   T3a_data = 0;   // Turm 3, wenn rechte Hälfte
float T3a_m    = 0;
float T3a_b    = 0;
int   T3b_data = 0;   // Turm 3, wenn linke Hälfte
float T3b_m    = 0;
float T3b_b    = 0;

float helptime = 0;
int   checksum = 0;   // wird hochgezählt, wenn IR Signal empfangen wird

float X = 0;      // X-Koordinate (absolutes Koordinatensystem)
float Y = 0;      // Y-Koordinate (absolutes Koordinatensystem)
float *Xp = &X;   // Zeiger auf X
float *Yp = &Y;   // Zeiger auf Y


void setup() {
  pinMode(status_led, OUTPUT);
  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);
  pinMode(IR3, INPUT);
  pinMode(IR4, INPUT);
  pinMode(IR5, INPUT);
  pinMode(IR6, INPUT);
  pinMode(IR7, INPUT);
  pinMode(IR8, INPUT);

  // Timer konfigurieren
  Timer1.initialize(200010);
  Timer1.setPeriod(200010); // Sets the period in microseconds

  Serial.begin(9600); // Seriellen Monitor aktiviert -> Pin 1 & 2 belegt
}


void loop() {

  // Nordimuls als Flankenerkennung
  radioStatus_new = digitalRead(north_pulse);
  if (radioStatus_new > radioStatus_old ) {
    Timer1.restart();
    radioStatus_old = radioStatus_new;

#ifdef DEBUG_MODE
    Serial.print("Funk");
    Serial.println(radioStatus_new);
#endif
  }
  // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

  // TODO: Die if-Anweisung als Interrupt umsetzen
  if (ANY_IR_INPUT == LOW) {

    helptime = Timer1.read();
    helptime = helptime / 1000000;
    Serial.print("helptime="); Serial.println(helptime, 8);

    // Auswahl des Quadranten
    if ((helptime >= 0 && helptime <= T25) && T3a_data == 0 && T3b_data == 0) { // Turm 3 rechts
      T3a_m = tan((pi / 2) - (w * helptime));                      // Steigung berechnet
      T3a_b = -T3a_m;    // Offset berechnet
      T3a_data = 1;
      checksum++;
      Serial.println("Turm3a");

    }else if (helptime > T25 && helptime <= T50 && T1_data == 0) { // Turm 1
      T1_m = -tan((helptime * w) - (pi / 2));
      T1_data = 1;
      checksum++;
      Serial.println("Turm1");

    }else if (helptime > T50 && helptime <= T75 && T2_data == 0) { // Turm 2
      T2_m = tan((3 * pi / 2) - (helptime * w));
      T2_b = 3 - 2 * T2_m;
      T2_data = 1;
      checksum++;
      Serial.println("Turm2");

    }else if (helptime > T75 && helptime < T && T3b_data == 0 && T3a_data == 0) { // Turm 3 links
      T3b_m = -tan((helptime * w) - (3 * pi / 2));
      T3b_b = -T3b_m;
      T3b_data = 1;
      checksum++;
      Serial.println("Turm3b");

    }else;

  }


  if (checksum >= 2) { // Wenn sich zwei oder mehr Signale im Speicher befinden

    // Auswerten welche Türme für die Berechnung verwendet werden
    if (T1_data == 1 && T2_data == 1) {
      CalcIntersection(T1_m, T2_m, T1_b, T2_b, Xp, Yp);

    }else {

      if (T1_data == 1 && T3a_data == 1) {
        CalcIntersection(T1_m, T3a_m, T1_b, T3a_b, Xp, Yp);

      }else if (T1_data == 1 && T3b_data == 1) {
        CalcIntersection(T1_m, T3b_m, T1_b, T3b_b, Xp, Yp);

      }else if (T2_data == 1 && T3a_data == 1) {
        CalcIntersection(T2_m, T3a_m, T2_b, T3a_b, Xp, Yp);

      }else if (T2_data == 1 && T3b_data == 1) {
        CalcIntersection(T2_m, T3b_m, T2_b, T3b_b, Xp, Yp);

      }else if (T3a_data == 1 && T3b_data == 1) {
        T3a_data = T3b_data = 0;

      }else {
        X = 10; // Um zu testen, ob merker nicht funktionieren
        Y = 10;
      }

    }

    Serial.print('H'); //Heder zur Identifikation einer neuen Nachricht
    Serial.print(","); //Komma zum Trennen von Elementen
    Serial.print(X);
    Serial.print(",");
    Serial.print(Y);
    Serial.print(","); //Komma zum beenden des letzten Elements

#ifdef DEBUG_MODE
    Serial.print("X="); Serial.println(X);
    Serial.print("Y="); Serial.println(Y);
    Serial.print("T1_m="); Serial.println(T1_m);
    Serial.print("T2_m="); Serial.println(T2_m);
    Serial.print("T3a_m="); Serial.println(T3a_m);
    Serial.print("T3b_m="); Serial.println(T3b_m);
    Serial.print("T1_b="); Serial.println(T1_b);
    Serial.print("T2_b="); Serial.println(T2_b);
    Serial.print("T3a_b="); Serial.println(T3a_b);
    Serial.print("T3b_b="); Serial.println(T3b_b);
    Serial.print("T1_data="); Serial.println(T1_data);
    Serial.print("T2_data="); Serial.println(T2_data);
    Serial.print("T3a_data="); Serial.println(T3a_data);
    Serial.print("T3b_data="); Serial.println(T3b_data);
#endif

    checksum = T1_m = T2_m = T3a_m = T3b_m = T2_b = T3a_b = T3b_b = T1_data = T2_data = T3a_data = T3b_data = 0;

  } // endif checksum

} // end loop


// Funktionen zur Positionsbestimmung
void CalcIntersection (float m_1, float m_2, float b_1, float b_2, float* var_x, float* var_y) {
  *var_x = (m_2 - m_2) / (m_2 - m_1);
  *var_y = ((m_2 / m_1) - (m_2 / m_2)) / ((1 / m_1) - (1 / m_2));

#ifdef DEBUG_MODE
  Serial.println("Funktion wurde aufgerufen");
#endif
}

