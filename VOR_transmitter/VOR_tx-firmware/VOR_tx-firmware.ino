// Mechatronisches Design - BYR - WS 2015 -- Semesterprojekt
// VOR transmitter

// dependencies:
// none


// set #define for master VOR, unset for slave VOR
#define VOR_MASTER

// sytem setup
const int VOR_frequency   = 50; // north impulse frequency
const int VOR_segments    = 32; // number of IR-LEDs, number of individual beams

// Variablen der I/O Pins
const int sr_dataPin    = 12;
const int sr_clockPin   = 11;
const int sr1_latchPin  = 10;
const int sr2_latchPin  = 9;
const int sr3_latchPin  = 8;
const int sr4_latchPin  = 7;
#ifdef VOR_MASTER
const int north_tx      = 13;   // radio transmitter
#elseif
const int statusLED     = 13;   // for debugging purposes
const int north_rx      = 2;    // radio receiver
#endif


// weitere Variablen
volatile byte edge      = 0x00;
int latches[4]          = {sr1_latchPin, sr2_latchPin, sr3_latchPin, sr4_latchPin};

const int delay_adjustment = 200; // entspricht der Ausführungszeit: hoher, damit geringeres delayMicroseconds

unsigned long north_interval_us  = 1000000 / VOR_frequency;
unsigned long segment_time_us    = north_interval_us / VOR_segments;
unsigned long beam_delay         = segment_time_us - delay_adjustment;


void setup () {
  pinMode(sr_dataPin, OUTPUT);
  pinMode(sr_clockPin, OUTPUT);
  pinMode(sr1_latchPin, OUTPUT);
  pinMode(sr2_latchPin, OUTPUT);
  pinMode(sr3_latchPin, OUTPUT);
  pinMode(sr4_latchPin, OUTPUT);
#ifdef VOR_MASTER
  pinMode(north_tx, OUTPUT);
#endif
#ifndef VOR_MASTER
  pinMode(statusLED, OUTPUT);
#endif

  // alle Latch-Signale auf low
  digitalWrite(sr1_latchPin, LOW);
  digitalWrite(sr2_latchPin, LOW);
  digitalWrite(sr3_latchPin, LOW);
  digitalWrite(sr4_latchPin, LOW);

#ifndef VOR_MASTER
  attachInterrupt(digitalPinToInterrupt(north_rx), interruptroutine, RISING); // sync slave to master
#endif
}


void loop() {

#ifndef VOR_MASTER
  digitalWrite(statusLED, HIGH);
  while (0x00 == edge) {
    ;
  }
  digitalWrite(statusLED, LOW);
  edge = 0x00;
#endif


  for (int quarter = 0; quarter < 4; quarter++) {

#ifdef VOR_MASTER
      if (quarter < 2) {
        digitalWrite(north_tx, HIGH);
      }else {
        digitalWrite(north_tx, LOW);
      }
#endif

      unsigned int latch = latches[quarter];

      // Lauflicht
      for (int i = 0; i < 8; i++) {   // 8 LEDs
        byte beam_mask = (0x01 << i); // ein Bit durchlaufen lassen

        digitalWrite(latch, LOW);
        shiftOut(sr_dataPin, sr_clockPin, MSBFIRST, beam_mask);
        digitalWrite(latch, HIGH);

        delayMicroseconds(beam_delay);
      }

      // alle Ausgänge des Schieberegister auf low
      digitalWrite(latch, LOW);
      shiftOut(sr_dataPin, sr_clockPin, MSBFIRST, 0x00);
      digitalWrite(latch, HIGH);

      // aufräumen und weiterschalten
      delayMicroseconds(5);
      digitalWrite(latch, LOW);
    
  } // end for quarter

} // end loop


void interruptroutine() {
  edge = 0x01;
}
