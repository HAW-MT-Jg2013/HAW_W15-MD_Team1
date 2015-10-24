// Mechatronisches Design - BYR - WS 2015 -- Semesterprojekt
// VOR receiver - VEREINHEITLICHTE HARDWARE-VERSION !!!
// Arduino Board: Arduino nano

// dependencies:
// TimerOne-r11 (https://code.google.com/p/arduino-timerone/downloads/list)


#include <TimerOne.h>
#include "towers.h"

// Debug modus anschalten
#define DEBUG_MODE      // genereller debugging modus
//#define DEBUG_IR        // Zeigt ob und welcher Sender erkannt wurde
//#define DEBUG_IR_EXTREM //zeigt welcher IR empfangen hat


// sytem setup
const int VOR_frequency   = 50; // north impulse frequency
const int VOR_segments    = 32; // number of IR-LEDs, number of individual beams

// Positionen der Türme in Metern (Kommazahlen erlaubt)
#define T1_posX  0
#define T1_posY  3
#define T2_posX  2
#define T2_posY  3
#define T3_posX  1
#define T3_posY  0

// Variablen der I/O Pins
// Pin 0 und 1 frei für Kommunikation
// Timer1 -> kein analogWrite an Pin 9 und 10
#define IR1         2   // PD2
#define IR2         3   // PD3
#define IR3         4   // PD4
#define IR4         5   // PD5
#define IR5         6   // PD6
#define IR6         7   // PD7
#define IR7         8   // PB0
#define IR8         9   // PB1
#define NORTH_PULSE 11   // Funksignal auf Pin 10 und 11
#define STATUS_LED  13
#define IR_STATUS   A3   // an, wenn ANY_IR_INPUT
// ENDE: Hardware Setup


// Variablen für das Programm
#define ANY_IR_INPUT (digitalRead(IR1)&&digitalRead(IR2)&&digitalRead(IR3)&&digitalRead(IR4)&&digitalRead(IR5)&&digitalRead(IR6)&&digitalRead(IR7)&&digitalRead(IR8))

boolean northPulse_old  = 0;
boolean northPulse_new  = 0;
unsigned int northPulse_counter = 0;

unsigned long north_period_us = 1000000 / VOR_frequency;
int sampleTimeCorrection = 70; // in us, verschiebt den Timerwert nach links

Tower* tower1 = new Tower(1, T1_posX, T1_posY);
Tower* tower2 = new Tower(2, T2_posX, T2_posY);
Tower* tower3 = new Tower(3, T3_posX, T3_posY);

float pos_x = 0;
float pos_y = 0;


void setup() {
  pinMode(STATUS_LED, OUTPUT);
  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);
  pinMode(IR3, INPUT);
  pinMode(IR4, INPUT);
  pinMode(IR5, INPUT);
  pinMode(IR6, INPUT);
  pinMode(IR7, INPUT);
  pinMode(IR8, INPUT);
  pinMode(IR_STATUS, OUTPUT);

  pinMode(NORTH_PULSE, INPUT);

  // Timer konfigurieren
  Timer1.initialize();
  Timer1.setPeriod(north_period_us); // Periode in Mikrosekunden

  Serial.begin(115200); // Seriellen Monitor aktiviert -> Pin 1 & 2 belegt
}


void loop() {

  // Phase 1: Steigende Flanke des Nordimpulses erkennen und Timer starten
  // -------------
  northPulse_new = digitalRead(NORTH_PULSE);
  if (northPulse_new && !northPulse_old ) {
    Timer1.restart();
    northPulse_counter++;
    northPulse_old = northPulse_new;
  } else if (northPulse_new == 0)  {
    northPulse_old = northPulse_new;
  }


  if ( (northPulse_counter != 0) && (northPulse_counter < 5) ) { // 4 Mal Daten sammeln
    // Phase 2: IR-Strahl empfangen
    // -------------
    digitalWrite(IR_STATUS, LOW);

#ifndef DEBUG_IR_EXTREM
    if (ANY_IR_INPUT == 0) { // Achtung! IR-Empfänger LOW-aktiv
      digitalWrite(IR_STATUS, HIGH);

#else
    boolean IRs[8];
    int IRPINS[8] = {IR1, IR2, IR3, IR4, IR5, IR6, IR7, IR8};
    for (int n = 0 ; n < 8 ; n++) {
      IRs[n] = digitalRead(IRPINS[n]);
      Serial.print(IRs[n]);
      Serial.print(" - ");
    }
    Serial.println("\n");
    if ((IRs[0] || IRs[1] || IRs[2] || IRs[3] || IRs[4] || IRs[5] || IRs[6] || IRs[7]) == 1) {
#endif

      unsigned long timerValue = Timer1.read() - sampleTimeCorrection;
      unsigned int angle = 360 * timerValue / north_period_us  + 0.5;

      if ( (0 < angle && angle < 90) || (270 <= angle && angle < 360) ) {  // Sender 3
        tower3->set_angle(angle);
#ifdef DEBUG_IR
        Serial.print("\t T3");
#endif

      } else if (90 <= angle && angle < 180) {                            // Sender 1
        tower1->set_angle(angle);
#ifdef DEBUG_IR
        Serial.print("\t T1");
#endif

      } else if (180 <= angle && angle < 270) {                           // Sender 2
        tower2->set_angle(angle);
#ifdef DEBUG_IR
        Serial.print("\t T2");
#endif

      } else {
        // TODO: error handling
      }

#ifdef DEBUG_IR
      Serial.print("\t"); Serial.println(angle);
#endif
    }

  } /* end Datensammel-Intervalle */


  if (northPulse_counter >= 5) { // dann 1 Mal rechnen
    // Phase 3: Position berechnen und ausgeben
    // -------------
    digitalWrite(STATUS_LED, HIGH);

    if (Tower::get_newValuesCounter() > 1) {    // mind. 2 Signale empfangen
      if (Tower::get_newValuesCounter() == 3) {
        // 3 Schnittpunkte und das geom. Mittel berechnen
        float m1, m2, m3, b1, b2, b3;
        tower1->get_parameters(&m1, &b1);
        tower2->get_parameters(&m2, &b2);
        tower3->get_parameters(&m3, &b3);

        float x1, x2, x3, y1, y2, y3;
        CalcIntersection(m1, m2, b1, b2, &x1, &y1); // Sender 1 und 2
        CalcIntersection(m2, m3, b2, b3, &x2, &y2); // Sender 2 und 3
        CalcIntersection(m3, m1, b3, b1, &x3, &y3); // Sender 3 und 1

        // Mittelwert als arithmetisches Mittel
        if (x1 != 0.0 || x2 != 0.0 || x3 != 0.0) { // da wenn Fehler: x=y=0.0
          pos_x = (x1 + x2 + x3) / 3.0;
          pos_y = (y1 + y2 + y3) / 3.0;
        } else {
          // ERROR
          pos_x = 0.0;
          pos_y = 0.0;
        }
        // TODO: Mittelwert als Mittelpunkt des Dreieck zwischen den Schnittpunkten

#ifdef DEBUG_MODE
        Serial.print("   ");
#endif

      } else {
        float m1, m2, b1, b2;
        if ( tower1->has_newValue() && tower2->has_newValue() ) {       // Sender 1 und 2
          tower1->get_parameters(&m1, &b1);
          tower2->get_parameters(&m2, &b2);
#ifdef DEBUG_MODE
          Serial.print("12 ");
#endif
        } else if ( tower2->has_newValue() && tower3->has_newValue() ) { // Sender 2 und 3
          tower2->get_parameters(&m1, &b1);
          tower3->get_parameters(&m2, &b2);
#ifdef DEBUG_MODE
          Serial.print("23 ");
#endif
        } else {                                                         // Sender 3 und 1
          tower3->get_parameters(&m1, &b1);
          tower1->get_parameters(&m2, &b2);
#ifdef DEBUG_MODE
          Serial.print("31 ");
#endif
        }

        CalcIntersection(m1, m2, b1, b2, &pos_x, &pos_y);
      }


      /*
       * Koordinaten versenden
       */
      unsigned int x = pos_x * 100 + 0.5;
      unsigned int y = pos_y * 100 + 0.5;

#ifndef DEBUG_MODE
      Serial.write('@');
      Serial.write(x >> 8); Serial.write(x & 0x00FF);
      Serial.write(y >> 8); Serial.write(y & 0x00FF);
#else
      Serial.print("X="); Serial.print(x); Serial.print("\t");
      Serial.print("Y="); Serial.print(y); Serial.print("\n");
#endif

    } /* end Signale empfangen */

    northPulse_counter = 0;
    digitalWrite(STATUS_LED, LOW);
    digitalWrite(IR_STATUS, LOW);
  } /* end Rechenintervall */


} /* end loop */


// Funktionen zur Positionsbestimmung
void CalcIntersection(float m_1, float m_2, float b_1, float b_2, float* x, float* y) {
  *x = (b_1 - b_2) / (m_2 - m_1);
  *y = ((b_1 / m_1) - (b_2 / m_2)) / ((1 / m_1) - (1 / m_2));

  if (*x < 0.0 || *y < 0.0 ||  *x > 2.0 || *y > 3.0) {
    *x = 0.0; // ungültig
    *y = 0.0; // ungültig
  }

#ifdef DEBUG_MODE
  //Serial.println("CalcIntersection");
#endif
}

