/*
* Programm fuer einen Arduino UNO zur Positionsbestimmung mittels VOR-Technik
* mit Umlauffrequenz 1Hz. Die Koordinaten werden ueber serielle Schnittstelle
* im Format (xKoord yKoord) als unsigned char-Werte in cm uebertragen, wobei
* die Y-Koordinate mit einem Faktor 1/1.2 multipliziert wurde, damit sie den
* Wert 255 nicht uebersteigt.
*/

#include "functions.h"
#define PI 3.14159265

#define DEBUG
//#define TEST

long start_time;

// Zeit in ms seit Nordimpuls bis entsprechender Winkel
double alphaTime;
double betaTime;
double kappaTime;

// errechnete Winkel
double alpha;
double beta;
double kappa;

// Hilfsfaktor, der fuer mehrere Rechnungen benoetigt wird, aber nur einmal berechnet werden soll
double gamma;

// Koordinaten
unsigned char xKoord;
unsigned char yKoord;


void setup() {

  Serial.begin(9600);

  // Inupts fuer IR-Empfaenger
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  pinMode(6, INPUT);
  pinMode(7, INPUT);
  pinMode(8, INPUT);
  pinMode(9, INPUT);
  // Input fuer NordImpuls
  pinMode(11, INPUT);

}

void loop() {

#ifndef DEBUG
  Serial.print("LOOP START");

  bool IRdetected = true;
  alphaTime = 0;
  betaTime = 0;
  kappaTime = 0;

  while (!digitalRead(12)) {    // warten auf Nordimpuls
    ;                           // ACHTUNG: WARTET UNENDLICH LANGE
  }

  start_time = millis();


  IRdetected = waitForIR(250);          // auf IR Signal unten warten
  if (IRdetected == true) {
    kappaTime = millis() - start_time;
  }
  waitMSfrom(start_time, 250);          // falls IR-Signale detektiert wird hier gewartet, bis Signal des Leuchtturms unten ausserhalb Spielfeld

  IRdetected = waitForIR(250);          // auf IR Signal oben links warten
  if (IRdetected == true) {
    alphaTime = millis() - start_time;
  }
  waitMSfrom(start_time, 500);          // falls IR-Signale detektiert wird hier gewartet, bis Signal des Leuchtturms links ausserhalb Spielfeld

  IRdetected = waitForIR(250);          // auf IR Signal oben rechts warten
  if (IRdetected == true) {
    betaTime = millis() - start_time;
  }
  waitMSfrom(start_time, 750);          // falls IR-Signale detektiert wird hier gewartet, bis Signal des Leuchtturms rechts ausserhalb Spielfeld

  if (kappaTime == 0) {                 // falls noch kein IR Signal von unten empfangen wurde:
    IRdetected = waitForIR(200);        // auf IR Signal unten warten
    if (IRdetected == true) {
      kappaTime = millis() - start_time;
    }
  }
#endif

#ifdef TEST
  // TEST: Vorgeben von 3 Zeiten fuer Winkel mit bekannten Koordinaten
  alphaTime = (double)(144.0 / 360 * 1);  // alpha = 144 Grad
  betaTime = (double)(197.0 / 360 * 1);   // beta  = 197 Grad
  kappaTime = (double)(21.0 / 360 * 1);   // kappa = 21 Grad
  // Erwartetes Ergebnis: X: 140cm
  //                      Y: 106cm
#endif

//Berechnung des Winkels aus den Zeiten
  alpha = 2 * PI * alphaTime;
  beta  = 2 * PI * betaTime;
  kappa = 2 * PI * kappaTime;

  calculate(&xKoord, &yKoord, alpha, beta, kappa);

#ifdef DEBUG
// DEBUG Ausgabe: Winkel in rad, Koordinaten in cm
  Serial.print(alpha); Serial.print("\t"); Serial.print(beta); Serial.print("\t"); Serial.print(kappa);  Serial.print("\t");
  Serial.print(xKoord); Serial.print("\t");
  Serial.println(yKoord * 1.2);
#endif

#ifndef DEBUG
  //Koordinaten uebermitteln via serielle Schnittstelle mit Arduino Standart-Parametern
  Serial.print(xKoord);
  Serial.println(yKoord);
#endif

}
