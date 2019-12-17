#ifndef Sensors_c
#define Sensors_c

#include <Arduino.h>
#include "Enums.cpp"

class Sensors {
public:
  
  Sensors()
    :curr_state_(0), prev_state_(0) {}

  void update() {
    prev_state_ = curr_state_;
    curr_state_ = PIND;
  }

  bool is_white(sensor sensor) {
    return get_state(curr_state_, sensor);
  }

  bool is_black(sensor sensor) {
    return !is_white(sensor);
  }

  bool has_changed(sensor sensor) {
    return get_state(curr_state_, sensor) == get_state(prev_state_, sensor);
  }

  bool changed_to_white(sensor sensor) {
    return is_white(sensor) && has_changed(sensor);
  }

  bool changed_to_black(sensor sensor) {
    return is_black(sensor) && has_changed(sensor);
  }
  
private:
  int curr_state_;
  int prev_state_;

  bool get_state(int state, sensor sensor) {
    return state & (1 << (3 + sensor)); 
  }
};

#endif // Sensors_c