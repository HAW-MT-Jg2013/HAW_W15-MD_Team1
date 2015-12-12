// Mechatronisches Design - BYR - WS 2015 -- Semesterprojekt
// VOR receiver
// Arduino Board: Arduino nano

// dependencies:
// TimerOne-r11 (https://code.google.com/p/arduino-timerone/downloads/list)


#include <TimerOne.h>
#include "towers.h"

// -- system setup (same on transmitter and receiver)
const int VOR_frequency   = 50; // north impulse frequency
const int VOR_segments    = 32; // number of IR-LEDs, number of individual beams

// -- debugging modes
//#define DEBUG_MODE      // general debugging mode (numbers as string, not binary)
#define BINARY_OUT        // send binary data (for integration with vehicle)
//#define DEBUG_IR        // shows what tower is detected


// -- setup tower positions in meters (floating point is ok)
#define T1_posX       0
#define T1_posY       2.95
#define T2_posX       1.95
#define T2_posY       2.95
#define T3_posX       1
#define T3_posY       0.05

#define MIN_X         0.0  // X value range min
#define MAX_X         2.0  // X value range max
#define MIN_Y         0.0  // Y value range min
#define MAX_Y         3.0  // Y value range max


// -- setup I/O Pins
#define IR1           2     // PD2
#define IR2           3     // PD3
#define IR3           4     // PD4
#define IR4           5     // PD5
#define IR5           6     // PD6
#define IR6           7     // PD7
#define IR7           8     // PB0
#define IR8           9     // PB1
#define NORTH_PULSE   11    // radio receiver in pin 10 and 11
#define STATUS_LED    13
#define IR_STATUS     A3    // shows ANY_IR_INPUT status for debugging


// -- parameters for calibration and corrections
#define FREQ_CORR     255   // [us]  - correct VOR period time: measured time - calculated time
#define SAMPLE_CORR   300   // [us]  - shift timer value to lower value
#define PARALLEL_THRE 7     // [deg] - angle threshold for parallel rays

#define AVG_ANGLE     5     // no. of north pulses per position calculation
#define AVG_VALUES    5     // no. of positions averaging before serial transmission (production mode)


// -- global variables and macros
#define ANY_IR_INPUT ( ((~PIND & 0xFC) | (~PINB & 0x03)) ) // more efficient than 8x digitalRead - LOW active

unsigned long northPeriod_us = 1000000 / VOR_frequency + FREQ_CORR;

Tower* tower1 = new Tower(1, T1_posX, T1_posY);
Tower* tower2 = new Tower(2, T2_posX, T2_posY);
Tower* tower3 = new Tower(3, T3_posX, T3_posY);

boolean northPulse_old  = 0;
boolean northPulse_new  = 0;

float pos_x, pos_y = 0;

unsigned int avg_angle_count = 0;
unsigned int avg_pos_count = 0;
float posSum_x, posSum_y = 0;
int   posSum_count = 0;

#define FILTER_ARR_SIZE  6
#define FILTER_DIFF_MAX  30
#define FILTER_ERR_THRES 4
int   filterArray[FILTER_ARR_SIZE][2] = {0};
int   filterErrorCnt = 0;


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

  // configure timer
  Timer1.initialize();
  Timer1.setPeriod(northPeriod_us);

  Serial.begin(115200);
}


void loop() {

  // phase 1: wait for rising edge of north pulse and start timer
  // -------------
  northPulse_new = digitalRead(NORTH_PULSE);
  if (northPulse_new && !northPulse_old ) {
    Timer1.restart();
    avg_angle_count++;
    northPulse_old = northPulse_new;
  } else if (northPulse_new == 0) {
    northPulse_old = northPulse_new;
  }


  // phase 2: receiver IR beam
  // -------------
  if ( (avg_angle_count != 0) && (avg_angle_count < AVG_ANGLE) ) { // collect data n-1 times
    digitalWrite(IR_STATUS, LOW);

    if (ANY_IR_INPUT != 0) {
      digitalWrite(IR_STATUS, HIGH);

      unsigned long timerValue = Timer1.read() - SAMPLE_CORR;
      unsigned int angle = 360 * timerValue / northPeriod_us  + 0.5;

      if ( (0 <= angle && angle < 90) || (270 < angle && angle <= 360) ) {  // tower 3
        tower3->set_angle(angle);
#ifdef DEBUG_IR
        Serial.print("\t T3");
#endif

      } else if (90 <= angle && angle <= 180) {                             // tower 1
        tower1->set_angle(angle);
#ifdef DEBUG_IR
        Serial.print("\t T1");
#endif

      } else if (180 < angle && angle <= 270) {                             // tower 2
        tower2->set_angle(angle);
#ifdef DEBUG_IR
        Serial.print("\t T2");
#endif

      } else {
        // error
      }

#ifdef DEBUG_IR
      Serial.print("\t"); Serial.println(angle);
#endif
    }

  } /* end phase 2: data collection */


  // phase 3: calculate position and send data via serial
  // -------------
  if (avg_angle_count >= AVG_ANGLE) {   // after collecting n-1 times, calculate one north pulse period
    digitalWrite(STATUS_LED, HIGH);

    if (Tower::get_newValuesCounter() > 1) {      // if 2 signals or more

      /* calculate position */

      if (Tower::get_newValuesCounter() == 3) {   // 3 towers received ?

        // calculate 3 intersections and mean value of these three
        float m1, m2, m3, b1, b2, b3;
        int angle1, angle2, angle3;
        tower1->get_parameters(&m1, &b1, &angle1);
        tower2->get_parameters(&m2, &b2, &angle2);
        tower3->get_parameters(&m3, &b3, &angle3);

        // theat (nearly) parallel rays differently
        if ( abs(angle3 - abs(angle1 - 180)) <= PARALLEL_THRE ) {
          float x1, x2, y1, y2;
          CalcIntersection(m1, m2, b1, b2, &x1, &y1); // tower 1 und 2
          CalcIntersection(m2, m3, b2, b3, &x2, &y2); // tower 2 und 3

          // mean position
          if (x1 != 0.0 || x2 != 0.0) { // check only X, because x=y=0.0 at error case
            pos_x = (x1 + x2) / 2.0;
            pos_y = (y1 + y2) / 2.0;
          } else {
            // ERROR
            pos_x = 0.0;
            pos_y = 0.0;
          }

        } else if ( abs(angle3 - abs(angle2 - 180)) <= PARALLEL_THRE ) {
          float x1, x2, y1, y2;
          CalcIntersection(m1, m2, b1, b2, &x1, &y1); // tower 1 und 2
          CalcIntersection(m3, m1, b3, b1, &x2, &y2); // tower 3 und 1

          // mean position
          if (x1 != 0.0 || x2 != 0.0) { // check only X, because x=y=0.0 at error case
            pos_x = (x1 + x2) / 2.0;
            pos_y = (y1 + y2) / 2.0;
          } else {
            // ERROR
            pos_x = 0.0;
            pos_y = 0.0;
          }

        } else { // normal calculation
          float x1, x2, x3, y1, y2, y3;
          CalcIntersection(m1, m2, b1, b2, &x1, &y1); // tower 1 und 2
          CalcIntersection(m2, m3, b2, b3, &x2, &y2); // tower 2 und 3
          CalcIntersection(m3, m1, b3, b1, &x3, &y3); // tower 3 und 1

          // mean position
          if (x1 != 0.0 || x2 != 0.0 || x3 != 0.0) { // check only X, because x=y=0.0 at error case
            pos_x = (x1 + x2 + x3) / 3.0;
            pos_y = (y1 + y2 + y3) / 3.0;
          } else {
            // ERROR
            pos_x = 0.0;
            pos_y = 0.0;
          }

        }

#ifdef DEBUG_MODE
        Serial.print("   ");
#endif

      } else {    // only 2 towers received
        float m1, m2, b1, b2;
        int angle;
        if ( tower1->has_newValue() && tower2->has_newValue() ) {         // tower 1 und 2
          tower1->get_parameters(&m1, &b1, &angle);
          tower2->get_parameters(&m2, &b2, &angle);
#ifdef DEBUG_MODE
          Serial.print("12 ");
#endif
        } else if ( tower2->has_newValue() && tower3->has_newValue() ) {  // tower 2 und 3
          tower2->get_parameters(&m1, &b1, &angle);
          tower3->get_parameters(&m2, &b2, &angle);
#ifdef DEBUG_MODE
          Serial.print("23 ");
#endif
        } else {                                                          // tower 3 und 1
          tower3->get_parameters(&m1, &b1, &angle);
          tower1->get_parameters(&m2, &b2, &angle);
#ifdef DEBUG_MODE
          Serial.print("31 ");
#endif
        }

        CalcIntersection(m1, m2, b1, b2, &pos_x, &pos_y);
      }


      /* send position */

#ifndef DEBUG_MODE
      avg_pos_count++;
      if (pos_x != 0) { // don´t count invalid positions
        posSum_x += pos_x;
        posSum_y += pos_y;
        posSum_count ++;
      }

      if (avg_pos_count >= AVG_VALUES) {
        avg_pos_count = 0;
        unsigned int intX = posSum_x * 100 / (float)posSum_count + 0.5;
        unsigned int intY = posSum_y * 100 / (float)posSum_count + 0.5;

        posSum_x = 0;
        posSum_y = 0;
        posSum_count = 0;

        // filter impossible values (too big differences)
        if (intX != 0 ) {

          if (filterErrorCnt < FILTER_ERR_THRES) {
            filterErrorCnt++;

            int filterMeanX = 0;
            int filterMeanY = 0;
            for (int i = 0; i < FILTER_ARR_SIZE; i++) {
              filterMeanX += filterArray[i][0];
              filterMeanY += filterArray[i][1];
            }
            filterMeanX = (float)filterMeanX / FILTER_ARR_SIZE;
            filterMeanY = (float)filterMeanY / FILTER_ARR_SIZE;

            if ( (abs(filterMeanX - intX) < FILTER_DIFF_MAX) && (abs(filterMeanY - intY) < FILTER_DIFF_MAX) ) {
              filterErrorCnt = 0;

              // shift old values
              for (int i = (FILTER_ARR_SIZE - 2); i == 0; i--) {
                filterArray[i + 1][0] = filterArray[i][0];
                filterArray[i + 1][1] = filterArray[i][1];
              }
              filterArray[0][0] = intX;
              filterArray[0][1] = intY;
            }
            intX = filterMeanX;
            intY = filterMeanY;

          } else {
            filterErrorCnt = 0;

            for (int i = 0; i < FILTER_ARR_SIZE; i++) {
              filterArray[i][0] = intX;
              filterArray[i][1] = intY;
            }
          }

        }


#ifdef BINARY_OUT
        Serial.write('@');
        Serial.write(intX >> 8); Serial.write(intX & 0x00FF);
        Serial.write(intY >> 8); Serial.write(intY & 0x00FF);
#else
        Serial.print("X= "); Serial.print(intX); Serial.print("\t");
        Serial.print("Y= "); Serial.print(intY); Serial.print("\n");
#endif
      }
#else
      pos_x = (int)(pos_x * 100 + 0.5);
      pos_y = (int)(pos_y * 100 + 0.5);
      Serial.print("X= "); Serial.print(pos_x); Serial.print("\t");
      Serial.print("Y= "); Serial.print(pos_y); Serial.print("\n");
#endif

    } /* end calculation and data send */

    avg_angle_count = 0;
    digitalWrite(STATUS_LED, LOW);
    digitalWrite(IR_STATUS, LOW);
  } /* end phase 3: calculation and sending data */


} /* end loop */


// calculate intersection of two linear functions
void CalcIntersection(float m_1, float m_2, float b_1, float b_2, float* x, float* y) {
  *x = (b_1 - b_2) / (m_2 - m_1);
  *y = ((b_1 / m_1) - (b_2 / m_2)) / ((1 / m_1) - (1 / m_2));

  if (*x < MIN_X || *y < MIN_Y ||  *x > MAX_X || *y > MAX_Y) {
    *x = 0.0; // return value indicating invalid calculation result
    *y = 0.0; // return value indicating invalid calculation result
  }
}
