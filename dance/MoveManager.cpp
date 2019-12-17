#ifndef MoveManager_c
#define MoveManager_c

#include <Arduino.h>
#include "Movement.cpp"
#include "Sensors.cpp"
#include "Enums.cpp"

class MoveManager
{
public: 
  MoveManager(Movement *mov, Sensors *sens):mov(mov),sens(sens) {

  }

  bool move_forward(int segments)
  {
    // Initialization
    if(forward_segments == -1)
      forward_segments = segments;
      
    drive_forward();
    if((sens->changed_to_black(f_left) && sens->is_white(f_right)) || (sens->changed_to_black(f_right) && sens->is_white(f_left)))
    {
      --forward_segments;
      for (int i = 0; i < num_headings; ++i)
        actual_crossroad[i] = false;
    }

    if(forward_segments == 0)
    {
      if(sens->is_black(f_left))
        actual_crossroad[(actual_heading + 1) % num_headings] = true;
      
      if(sens->is_black(f_right))
        actual_crossroad[(actual_heading - 1) % num_headings] = true;

      drive_forward();
      // Move a bit further to get middle on the crossroad
      // if middle on the crossroad
      {
        forward_segments = -1;
        actual_crossroad[actual_heading] = sens->is_black(c_left) || sens->is_black(center) || sens->is_black(c_right);
        actual_crossroad[(actual_heading + 2) % num_headings] = true;
      }
    }

    return forward_segments == -1;
  }

  bool rotate_left(int quarters)
  {
    if (rotate_cross_lines == 0)
    {
      if (quarters == 0)
        return true;

      for (int i = 1; i <= num_headings; ++i)
      {      
        if(actual_crossroad[(actual_heading + i) % num_headings])
          rotate_cross_lines =  quarters / 4 + (quarters % 4 >= i) ? 1 : 0;
      }

      actual_heading = (actual_heading + quarters) % num_headings;
    }
    return turn_left();
  }
  
  bool rotate_right(int quarters)
  {
    if (rotate_cross_lines == 0)
    {
      if (quarters == 0)
        return true;

      for (int i = 1; i <= num_headings; ++i)
      {      
        if(actual_crossroad[(actual_heading - i) % num_headings])
          rotate_cross_lines =  quarters / 4 + (quarters % 4 >= i) ? 1 : 0;
      }
      
      actual_heading = (actual_heading + quarters) % num_headings;
    }
    return turn_right();
  }
  
  void reset() 
  {
    mov->stop();
    forward_segments = -1;
    rotate_cross_lines = 0;
  }
  
private:

  bool actual_crossroad[num_headings];
  int forward_segments = -1;
  int rotate_cross_lines = 0;
  int actual_heading;

  Movement *mov;
  Sensors *sens;
  
  void drive_forward() {
    if(sens->is_black(c_left) && sens->is_white(f_left))
      mov->left_forward(faster, faster + 10);
    else if(sens->is_black(c_right) && sens->is_white(f_right))
      mov->right_forward(faster, faster + 10);
    else 
      mov->forward(faster);
    
  }
  
  bool turn_left() {
    mov->left_inplace(faster);
    if (sens->changed_to_black(center))
      --rotate_cross_lines;
    return rotate_cross_lines <= 0;
  }
  
  bool turn_right() {
    mov->right_inplace(faster);
    if (sens->changed_to_black(center))
      --rotate_cross_lines;
    return rotate_cross_lines <= 0;
  }
};

#endif // MoveManager_c