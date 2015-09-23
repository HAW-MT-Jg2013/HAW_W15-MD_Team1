// Mechatronisches Design - BYR - WS 2015 -- Semesterprojekt
// VOR transmitter

// dependencies:
// none


// Variablen der I/O Pins
const int led       = 13;
const int dataPin   = 12;
const int clockPin  = 11;
const int latch1Pin = 10;
const int latch2Pin = 9;
const int latch3Pin = 8;
const int latch4Pin = 7;

int interruptNumber = 0; // Interrupt-Nummer
byte Flanke   = 0x00;
byte Register = 0x01;
byte count    = 0x00;
byte SR_res   = 0x00;


void setup () {
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(latch1Pin, OUTPUT);
  pinMode(latch2Pin, OUTPUT);
  pinMode(latch3Pin, OUTPUT);
  pinMode(latch4Pin, OUTPUT);
  pinMode(led, OUTPUT);

  // alle Latch-Signale auf low
  digitalWrite(latch1Pin, LOW);
  digitalWrite(latch2Pin, LOW);
  digitalWrite(latch3Pin, LOW);
  digitalWrite(latch4Pin, LOW);

  count = 0x01;
  attachInterrupt(interruptNumber, interruptroutine, RISING);
}

void interruptroutine() {
  Flanke == 0x01; //TODO - MUSS DAS SO? - MFG - DANIEL
}

void process(int latch, boolean ledStatus, int indexMax) {
  digitalWrite(led, ledStatus);
  // 1 x Lauflicht Schieberegister 
  for (int i = 0; i < indexMax; i++) { // Impulse mit for-Schleife
    shiftOut(dataPin, clockPin, MSBFIRST, count);
    // Latch-Impuls für Schieberegister
    digitalWrite(latch, HIGH);
    delayMicroseconds(230);
    digitalWrite(latch, LOW);
    delayMicroseconds(230);

    count *= 2;
    if (count == 0) {
      count = 0x01;
    }
  }
  // alle Ausgänge des Schieberegisters auf low
  shiftOut(dataPin, clockPin, MSBFIRST, SR_res);
  // Latch-Impuls für Schieberegister 
  digitalWrite(latch, HIGH);
  delayMicroseconds(5);
  digitalWrite(latch, LOW);
}

void loop() {

  switch (Register) {
    case 1:
      process(latch1Pin, 1, 8);
      Register = 0x02;
      break;

    case 2:
      process(latch2Pin, 1, 8);
      Register = 0x03;
      break;

    case 3:
      process(latch3Pin, 0, 8);
      Register = 0x04;
      break;

    case 4:
      process(latch4Pin, 0, 7);
      Register = 0x01;
      break;
  } // end switch

} // end loop
