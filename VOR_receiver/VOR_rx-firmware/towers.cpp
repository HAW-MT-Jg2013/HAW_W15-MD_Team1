#include "Arduino.h"
#include "towers.h"

unsigned int Tower::newValuesCounter = 0;


Tower::Tower(int id, float pos_x, float pos_y) {
  if (0 < id <= 3) {
    this->id = id;
    this->pos_x = pos_x;
    this->pos_y = pos_y;
  }else {
    // TODO: error handling
  }
  has_newData = 0;
}

void Tower::set_angle(int angle) {
  if (0 <= angle <= 360) {
    float angle_rad       = angle * PI / 180.0;
    float angle_corrected = (PI / 2.0) - angle_rad; // 90° - angle
  
    m = tan(angle_corrected);
    b = pos_y - (m * pos_x);

    if (0 == has_newData) { // prevent double counts
      newValuesCounter++;
    }
    has_newData = 1;
  }
}

boolean Tower::has_newValue() {
  return has_newData;
}

void Tower::get_parameters(float* m_param, float* b_param) {
  *b_param = this->b;
  *m_param = this->m;

  if (1 == has_newData) { // prevent double subtractions
      newValuesCounter--;
  }
  has_newData = 0;
}

int Tower::get_newValuesCounter() {
  return newValuesCounter;
}
