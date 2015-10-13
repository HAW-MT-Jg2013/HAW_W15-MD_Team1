// Mechatronisches Design - BYR - WS 2015 -- Semesterprojekt
// VOR transmitter

// dependencies:
// none


// set #define for master VOR, unset for slave VOR
#define VOR_MASTER

// sytem setup
const int VOR_frequency   = 5; // north impulse frequency
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
#else
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

  // alle Latch-Signale auf low
  digitalWrite(sr1_latchPin, LOW);
  digitalWrite(sr2_latchPin, LOW);
  digitalWrite(sr3_latchPin, LOW);
  digitalWrite(sr4_latchPin, LOW);

}


void loop() {

  unsigned int latch = latches[0];

  digitalWrite(latch, LOW);
  shiftOut(sr_dataPin, sr_clockPin, MSBFIRST, 0x01);
  digitalWrite(latch, HIGH);

  delayMicroseconds(beam_delay);


  // alle Ausgänge des Schieberegister auf low
  digitalWrite(latch, LOW);
  shiftOut(sr_dataPin, sr_clockPin, MSBFIRST, 0x00);
  digitalWrite(latch, HIGH);

  delayMicroseconds(beam_delay);

} // end loop

