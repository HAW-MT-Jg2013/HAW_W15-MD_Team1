#include "Arduino.h"
#include "towers.h"

unsigned int Tower::newValuesCounter = 0;


Tower::Tower(int id, float pos_x, float pos_y) {
  if ( (0 < id) && (id <= 3) ) {
    this->id = id;
    this->pos_x = pos_x;
    this->pos_y = pos_y;
    this->angleSum = 0;
    this->angleCounter = 0;
  } else {
    // error
  }
  has_newData = 0;
}

void Tower::set_angle(int angle) {
  if ( (0 <= angle) && (angle <= 360) ) {

    if (270 <= angle && angle < 360) { // shift angle from [270°, 360°] to [-90°, 0°]
      angle -= 360;
    }

    angleSum += angle;
    angleCounter++;

    if (0 == has_newData) { // prevent double counts
      newValuesCounter++;
    }
    has_newData = 1;

  }
}

boolean Tower::has_newValue() {
  return has_newData;
}

void Tower::get_parameters(float* m_param, float* b_param, int* angle) {

  float angle_mean      = angleSum / (float)angleCounter;
  float angle_rad       = angle_mean * PI / 180.0;
  float angle_corrected = (PI / 2.0) - angle_rad; // 90° - angle

  *m_param = tan(angle_corrected);
  *b_param = pos_y - (*m_param * pos_x);

  *angle = (int)angle_mean;

  if (1 == has_newData) { // prevent double subtractions
    newValuesCounter--;
  }
  has_newData = 0;

  angleSum = 0;
  angleCounter = 0;

  //debugging
  //Serial.print(id);Serial.print(":");Serial.print((int)angle_mean);Serial.print(" ");
}

int Tower::get_newValuesCounter() {
  return newValuesCounter;
}
