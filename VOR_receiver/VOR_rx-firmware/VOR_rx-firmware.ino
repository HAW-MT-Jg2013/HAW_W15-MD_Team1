// Mechatronisches Design - BYR - WS 2015 -- Semesterprojekt
// VOR receiver - VEREINHEITLICHTE HARDWARE-VERSION !!!
// Arduino Board: Arduino nano

// dependencies:
// TimerOne-r11 (https://code.google.com/p/arduino-timerone/downloads/list)


#include <TimerOne.h>
#include "towers.h"

// Debug modus anschalten
#define DEBUG_MODE    // genereller debugging modus
//#define DEBUG_NORTH   // zusätzlich, um Nord-Flanken zu zählen


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
#define IR1         2
#define IR2         3
#define IR3         4
#define IR4         5
#define IR5         6
#define IR6         7
#define IR7         8
#define IR8         9
#define NORTH_PULSE 11   // Funksignal auf Pin 10 und 11
#define STATUS_LED  13
// ENDE: Hardware Setup


// Variablen für das Programm
#define ANY_IR_INPUT (digitalRead(IR1)||digitalRead(IR2)||digitalRead(IR3)||digitalRead(IR4)||digitalRead(IR5)||digitalRead(IR6)||digitalRead(IR7)||digitalRead(IR8))

boolean northPulse_old  = 0;
boolean northPulse_new  = 0;
#ifdef DEBUG_NORTH
int i;
#endif

unsigned long north_period_us = 1000000 / VOR_frequency;

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

  pinMode(NORTH_PULSE, INPUT);

  // Timer konfigurieren
  Timer1.initialize();
  Timer1.setPeriod(north_period_us); // Periode in Mikrosekunden

  Serial.begin(9600); // Seriellen Monitor aktiviert -> Pin 1 & 2 belegt
}

void loop() {

  // Phase 1: Steigende Flanke des Nordimpulses erkennen und Timer starten
  // -------------
  northPulse_new = digitalRead(NORTH_PULSE);
  if (northPulse_new && !northPulse_old ) {
    Timer1.restart();
    northPulse_old = northPulse_new;
#ifdef DEBUG_NORTH
    i++; Serial.print("Flanke #"); Serial.println(i);
#endif
  }else if (northPulse_new == 0)  {
    northPulse_old = northPulse_new;
  }

#ifdef DEBUG_NORTH
  if (i%50) {
    Serial.println("----1 sek----");
  }
#endif


  // Phase 2: IR-Strahl empfangen
  // -------------
  if (ANY_IR_INPUT == 1) {
    unsigned long timerValue = Timer1.read();
    unsigned int angle = 360 * north_period_us / timerValue  +0.5;

    if ( (0 <= angle < 90) || (270 <= angle < 360) ) {  // Sender 3
      tower3->set_angle(angle);

    }else if (90 <= angle < 180) {                      // Sender 1
      tower1->set_angle(angle);

    }else if (180 <= angle < 270) {                     // Sender 2
      tower2->set_angle(angle);

    }else {
      // TODO: error handling
    }
  }


  // Phase 3: Position berechnen und ausgeben
  // -------------
  if (Tower::get_newValuesCounter() > 1) {    // mind. 2 Signale empfangen
    if (Tower::get_newValuesCounter() == 3) {
      // TODO: 3 Schnittpunkte und das geom. Mittel berechnen

    }else {
      float m1, m2, b1, b2;
      if ( tower1->has_newValue() && tower2->has_newValue() ) {       // Sender 1 und 2
        tower1->get_parameters(&m1, &b1);
        tower2->get_parameters(&m2, &b2);
      }else if ( tower2->has_newValue() && tower3->has_newValue() ) { // Sender 2 und 3
        tower2->get_parameters(&m1, &b1);
        tower3->get_parameters(&m2, &b2);
      }else {                                                         // Sender 3 und 1
        tower3->get_parameters(&m1, &b1);
        tower1->get_parameters(&m2, &b2);
      }

      CalcIntersection(m1, m2, b1, b2, &pos_x, &pos_y);
    }

    // TODO: Datenausgabe
    Serial.print("X=");Serial.print(pos_x);Serial.print("\t");
    Serial.print("Y=");Serial.print(pos_y);Serial.print("\n");
  }


} /* end loop */


// Funktionen zur Positionsbestimmung
void CalcIntersection(float m_1, float m_2, float b_1, float b_2, float* x, float* y) {
  *x = (m_2 - m_2) / (m_2 - m_1);
  *y = ((m_2 / m_1) - (m_2 / m_2)) / ((1 / m_1) - (1 / m_2));

#ifdef DEBUG_MODE
  Serial.println("CalcIntersection");
#endif
}
