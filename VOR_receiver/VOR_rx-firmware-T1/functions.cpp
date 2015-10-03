#include "Arduino.h"
#include "functions.h"

extern double gamma;

// wartet bis IR-Signal detektiert wird, maximal aber maxWaitTimeMS Millisekunden
// gibt true zurueck, falls IR-Signal detektiert, sonst false
bool waitForIR(short maxWaitTimeMS) {
  long startTime = millis();
  bool result = true;
  while ((!(digitalRead(2) | digitalRead(3) | digitalRead(4) | digitalRead(5)
            | digitalRead(6) | digitalRead(7) | digitalRead(8) | digitalRead(9)))
         && (millis() < (startTime + maxWaitTimeMS))) { // Warten auf IR-Impuls von IR-Sender unten
    ;
  }
  if (millis() >= (startTime + maxWaitTimeMS)) {
    result = false;
  }
  return result;
}


// wartet bis waitTimeMS Millisekunden nach startTimeMS vergangen sind
void waitMSfrom(long startTimeMS, short waitTimeMS) {
  while (millis() < (startTimeMS + waitTimeMS)) {
    ;
  }
}

// Berechnung der Koordinaten aus den errechneten Winkeln, Rueckgabe per Call-by-Reference
void calculate(unsigned char* xKoord, unsigned char* yKoord, double alpha, double beta, double kappa) {

  // falls kein Signal fuer Beta empfangen wurde
  if ((0 == beta) && (alpha != 0) && (kappa != 0)) {
    gamma = (300 * cos(alpha - PI / 2) - 100 * sin(alpha - PI / 2)) / (sin(kappa) * sin(alpha - PI / 2) + cos(alpha - PI / 2) * cos(kappa));
    *xKoord = 100 + gamma * sin(kappa);
    *yKoord = gamma * cos(kappa)/1.2;
  } else if ((0 == alpha) && (beta != 0) && (kappa != 0)) {
    // falls keine Zeit fuer Alpha empfangen wurde
    gamma = (100 * cos(kappa) - 300 * sin(kappa)) / (sin(beta - PI) * cos(kappa) - cos(beta - PI) * sin(kappa));
    *xKoord = 200 - gamma * sin(beta - PI);
    *yKoord = (300 - gamma * cos(beta - PI)) / 1.2;  // 1.2 als Aufloesungs-Faktor (muss nach Empfang wieder draufmultipliziert werden)
  } else if ((alpha != 0) && (beta != 0)) {
    // falls keine Zeit fuer Kappa empfangen wurde oder alle Signale empfangen wurden
    gamma = (200 * cos(PI - alpha)) / (cos(beta - PI) * sin(PI - alpha) + sin(beta - PI) * cos(PI - alpha));
    *xKoord = 200 - gamma * sin(beta - PI);
    *yKoord = (300 - gamma * cos(beta - PI)) / 1.2;  // 1.2 als Aufloesungs-Faktor  (muss nach Empfang wieder draufmultipliziert werden)
  } else {
    // FEHLERFALL: falls 2 oder mehr Signale nicht empfangen wurden
    *xKoord = 255;
    *yKoord = 255;
  }
}
