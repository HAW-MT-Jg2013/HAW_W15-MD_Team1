#ifndef TOWERS_H
#define TOWERS_H

#include "Arduino.h"

// definitions:
// Tower 1: top left  - 90-180° (Hardware No. B)
// Tower 2: top right - 180°-270° (Hardware No. C)
// Tower 3: bottom    - 0-90° and 270-360° (Hardware No. A)

class Tower {
    int id;
    float pos_x, pos_y;
    boolean has_newData;
    float m = 0; // gradient
    float b = 0; // offset

    static unsigned int newValuesCounter; // sum off all new data

  public:
    Tower(int id, float pos_x, float pos_y);
    void set_angle(int angle); // degrees
    boolean has_newValue();
    void get_parameters(float* m_param, float* b_param);

    static int get_newValuesCounter();
};

#endif /* TOWERS_H */
