#ifndef MoveManager_c
#define MoveManager_c

#include <Arduino.h>
#include "Movement.cpp"
#include "Sensors.cpp"
#include "Enums.cpp"

class MoveManager
{
public: 
  MoveManager(Movement *mov, Sensors *sens):mov(mov),sens(sens) 
  {
  }

  void SetHeading(int heading)
  {
    actual_heading = heading;
  }

  bool move_forward(int segments)
  {
    // Initialization
    if(forward_segments == -1)
    {
      forward_segments = segments;
    }
        
    if(forward_segments > 0)
    {
      drive_forward();
      if((sens->changed_to_black(f_left) && sens->is_white(f_right)) || (sens->changed_to_black(f_right) && sens->is_white(f_left)) || (sens->changed_to_black(f_right) && sens->changed_to_black(f_left)))
      {
        --forward_segments;
        if(forward_segments == 0)
        {
          forward_finish_time = millis();
          for (int i = 0; i < num_headings; ++i)
            actual_crossroad[i] = false;
        }
      }
    }
    else
    {
      // Move a bit further to get middle on the crossroad
      
      if(sens->is_black(f_left))
        actual_crossroad[(actual_heading + 1) % num_headings] = true;
      
      if(sens->is_black(f_right))
        actual_crossroad[(actual_heading - 1) % num_headings] = true;

      drive_forward();
         
      if(millis() - forward_finish_time > 300)
      {
        // if the middle is on the crossroad
        actual_crossroad[actual_heading] = sens->is_black(c_left) || sens->is_black(center) || sens->is_black(c_right);
        actual_crossroad[(actual_heading + 2) % num_headings] = true;
        reset();
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
          rotate_cross_lines +=  quarters / 4 + (quarters % 4 >= i) ? 1 : 0;
      }
      
      actual_heading = (actual_heading + quarters) % num_headings;
    }
    
    if(turn_left())
    {
      reset();
      return true;
    }
    return false;
  }
  
  bool rotate_right(int quarters)
  {
    if (rotate_cross_lines == 0)
    {
      if (quarters == 0)
        return true;

      for (int i = 1; i <= num_headings; ++i)
      {      
        if(actual_crossroad[(actual_heading - i + 4) % num_headings])
          rotate_cross_lines +=  quarters / 4 + ((quarters % 4 >= i) ? 1 : 0);
      }

      actual_heading = (actual_heading - quarters + 4) % num_headings;
    }
    
    if(turn_right())
    {
      reset();
      return true;
    }
    return false;
  }

  // Interupt current movement and reset variables - if movement is finished by itself, no need to call
  // Can break actual_heading if used during rotation
  void reset() 
  {
    mov->stop();
    forward_segments = -1;
    rotate_cross_lines = 0;
    forward_counter = 0;
  }
  
  int get_forward_segments() const {
    return forward_segments;
  }

private:
  // Forward variables
  int forward_segments = -1;    // Remaining segments for forward move
  int forward_counter = 0;      // Updates used for moving behing finish crossroad - should be changed to timer
  unsigned long forward_finish_time = 0; // Timer for moving behing finish crossroad
  
  // Rotating variable
  int rotate_cross_lines = 0; // Lines to be crossed by center sensor during rotation

  // Navigation variables
  int actual_heading = 0;   // actual heading of the robot based on rotating and init heading
  bool actual_crossroad[num_headings] = { true, true, true, true};    // Crossroad shape - initial shape assumed full crossroad

  Movement *mov;
  Sensors *sens;
  
  void drive_forward() {
    // TODO left_forward and right_forward seems to be broken
    if(sens->is_black(c_left) && sens->is_white(f_left))
    {
      mov->left_forward(faster, stopped);
    }
    else if(sens->is_black(c_right) && sens->is_white(f_right))
    {
      mov->right_forward(faster, stopped);
    }
    else
    {
      mov->forward(faster);
    }
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
