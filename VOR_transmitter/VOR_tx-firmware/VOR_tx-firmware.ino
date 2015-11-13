// Mechatronisches Design - BYR - WS 2015 -- Semesterprojekt
// VOR transmitter
// Arduino Board: Arduino nano

// dependencies:
// none


// set this option for master tower (#3), comment out for slave tower (#1, #2)
#define VOR_MASTER

// -- system setup (same on transmitter and receiver)
const int VOR_frequency   = 50; // north impulse frequency
const int VOR_segments    = 32; // number of IR-LEDs, number of individual beams


// -- setup I/O Pins
#define SR_DATA       12    // shift registers data
#define SR_CLOCK      11    // shift registers clock
#define SR1_LATCH     10    // segment 1 latch
#define SR2_LATCH     9     // segment 2 latch
#define SR3_LATCH     8     // segment 3 latch
#define SR4_LATCH     7     // segment 4 latch
#ifdef VOR_MASTER
#define NORTH_TX      13    // radio transmitter
#else
#define STATUS_LED    13    // for debugging purposes
#define NORTH_RX      2     // radio receiver
#endif


// -- parameters for calibration and corrections
#define DELAY_ADJ     200   // [us] - corrects processing time: higher for less delay


// -- global variables
volatile byte edge      = 0x00;
int latches[4]          = {SR1_LATCH, SR2_LATCH, SR3_LATCH, SR4_LATCH};

unsigned long northPeriod_us  = 1000000 / VOR_frequency;
unsigned long segmentTime_us  = northPeriod_us / VOR_segments;
unsigned long beamDelay_us    = segmentTime_us - DELAY_ADJ;


void setup () {
  pinMode(SR_DATA, OUTPUT);
  pinMode(SR_CLOCK, OUTPUT);
  pinMode(SR1_LATCH, OUTPUT);
  pinMode(SR2_LATCH, OUTPUT);
  pinMode(SR3_LATCH, OUTPUT);
  pinMode(SR4_LATCH, OUTPUT);
#ifdef VOR_MASTER
  pinMode(NORTH_TX, OUTPUT);
#endif
#ifndef VOR_MASTER
  pinMode(STATUS_LED, OUTPUT);
#endif

  digitalWrite(SR1_LATCH, LOW);
  digitalWrite(SR2_LATCH, LOW);
  digitalWrite(SR3_LATCH, LOW);
  digitalWrite(SR4_LATCH, LOW);

#ifndef VOR_MASTER
  attachInterrupt(digitalPinToInterrupt(NORTH_RX), interruptroutine, RISING); // sync slave to master
#endif
}


void loop() {

#ifndef VOR_MASTER
  digitalWrite(STATUS_LED, HIGH);
  while (0x00 == edge) {
    ; // wait for rising edge of NORTH_RX (interrupt)
  }
  digitalWrite(STATUS_LED, LOW);
  edge = 0x00;
#endif


  // for each quarter
  for (int quarter = 0; quarter < 4; quarter++) {

#ifdef VOR_MASTER
      if (quarter < 2) {
        digitalWrite(NORTH_TX, HIGH);
      }else {
        digitalWrite(NORTH_TX, LOW);
      }
#endif

      unsigned int latch = latches[quarter]; // get latch of corresponding shift register

      // for each LED of one quarter
      for (int i = 0; i < 8; i++) {
        byte beam_mask = (0x01 << i); // set bit i (one LED at a time)

        digitalWrite(latch, LOW);
        shiftOut(SR_DATA, SR_CLOCK, MSBFIRST, beam_mask);
        digitalWrite(latch, HIGH);

        delayMicroseconds(beamDelay_us);
      }

      // set all outputs to LOW and latch
      digitalWrite(latch, LOW);
      shiftOut(SR_DATA, SR_CLOCK, MSBFIRST, 0x00);
      digitalWrite(latch, HIGH);

      // leave everything tidied up for next quarter
      delayMicroseconds(5);
      digitalWrite(latch, LOW);

  } // end for quarter

} // end loop


void interruptroutine() {
  edge = 0x01;
}
