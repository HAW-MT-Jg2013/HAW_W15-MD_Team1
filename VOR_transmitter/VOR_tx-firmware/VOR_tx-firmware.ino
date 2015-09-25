// Mechatronisches Design - BYR - WS 2015 -- Semesterprojekt
// VOR transmitter

// dependencies:
// none


// sytem setup
const int VOR_frequency   = 50; // north impulse frequency
const int VOR_segments    = 32; // number of IR-LEDs, number of individual beams

// Variablen der I/O Pins
const int statusLedPin  = 13;
const int sr_dataPin    = 12;
const int sr_clockPin   = 11;
const int sr1_latchPin  = 10;
const int sr2_latchPin  = 9;
const int sr3_latchPin  = 8;
const int sr4_latchPin  = 7;


// weitere Variablen
const int interruptNumber = 0; // Interrupt-Nummer
byte edge     = 0x00;
int  quarter  = 1;

const int delay_adjustment = 165; // um die Ausführungszeit zu kompensieren

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
  pinMode(statusLedPin, OUTPUT);

  // alle Latch-Signale auf low
  digitalWrite(sr1_latchPin, LOW);
  digitalWrite(sr2_latchPin, LOW);
  digitalWrite(sr3_latchPin, LOW);
  digitalWrite(sr4_latchPin, LOW);

  // Interrupt zur Synchronisierung der Slaves
  attachInterrupt(interruptNumber, interruptroutine, RISING);
}


void loop() {

  switch (quarter) {
    case 1:
      digitalWrite(statusLedPin, HIGH);

      // Lauflicht Schieberegister 1
      for (int i = 0; i < 8; i++) { // 8 LEDs
        byte beam_mask = (0x01 << i); // ein Bit durchlaufen lassen

        digitalWrite(sr1_latchPin, LOW);
        shiftOut(sr_dataPin, sr_clockPin, MSBFIRST, beam_mask);
        digitalWrite(sr1_latchPin, HIGH);

        delayMicroseconds(beam_delay);
      }

      // alle Ausgänge des Schieberegister auf low
      digitalWrite(sr1_latchPin, LOW);
      shiftOut(sr_dataPin, sr_clockPin, MSBFIRST, 0x00);
      digitalWrite(sr1_latchPin, HIGH);

      // aufräumen und weiterschalten
      delayMicroseconds(5);
      digitalWrite(sr1_latchPin, LOW);
      quarter = 2;
      break;

    case 2:
      digitalWrite(statusLedPin, HIGH);

      // Lauflicht Schieberegister 2
      for (int i = 0; i < 8; i++) { // 8 LEDs
        byte beam_mask = (0x01 << i); // ein Bit durchlaufen lassen

        digitalWrite(sr2_latchPin, LOW);
        shiftOut(sr_dataPin, sr_clockPin, MSBFIRST, beam_mask);
        digitalWrite(sr2_latchPin, HIGH);

        delayMicroseconds(beam_delay);
      }

      // alle Ausgänge des Schieberegister auf low
      digitalWrite(sr2_latchPin, LOW);
      shiftOut(sr_dataPin, sr_clockPin, MSBFIRST, 0x00);
      digitalWrite(sr2_latchPin, HIGH);

      // aufräumen und weiterschalten
      delayMicroseconds(5);
      digitalWrite(sr2_latchPin, LOW);
      quarter = 3;
      break;

    case 3:
      digitalWrite(statusLedPin, LOW);

      // Lauflicht Schieberegister 3
      for (int i = 0; i < 8; i++) { // 8 LEDs
        byte beam_mask = (0x01 << i); // ein Bit durchlaufen lassen

        digitalWrite(sr3_latchPin, LOW);
        shiftOut(sr_dataPin, sr_clockPin, MSBFIRST, beam_mask);
        digitalWrite(sr3_latchPin, HIGH);

        delayMicroseconds(beam_delay);
      }

      // alle Ausgänge des Schieberegister auf low
      digitalWrite(sr3_latchPin, LOW);
      shiftOut(sr_dataPin, sr_clockPin, MSBFIRST, 0x00);
      digitalWrite(sr3_latchPin, HIGH);

      // aufräumen und weiterschalten
      delayMicroseconds(5);
      digitalWrite(sr3_latchPin, LOW);
      quarter = 4;
      break;

    case 4:
      digitalWrite(statusLedPin, LOW);

      // Lauflicht Schieberegister 4
      for (int i = 0; i < 7; i++) { // 7 LEDs
        byte beam_mask = (0x01 << i); // ein Bit durchlaufen lassen

        digitalWrite(sr4_latchPin, LOW);
        shiftOut(sr_dataPin, sr_clockPin, MSBFIRST, beam_mask);
        digitalWrite(sr4_latchPin, HIGH);

        delayMicroseconds(beam_delay);
      }

      // alle Ausgänge des Schieberegister auf low
      digitalWrite(sr4_latchPin, LOW);
      shiftOut(sr_dataPin, sr_clockPin, MSBFIRST, 0x00);
      digitalWrite(sr4_latchPin, HIGH);

      // aufräumen und weiterschalten
      delayMicroseconds(5);
      digitalWrite(sr4_latchPin, LOW);
      quarter = 1;
      break;
  } // end switch

} // end loop


void interruptroutine() {
  edge == 0x01;
}
