#ifndef Button_c
#define Button_c

#include <Arduino.h>

class Button {
public:

      Button(){
        is_pressed_ = false;
        was_pressed_ = false;
        was_unpressed_ = false;
        was_long_pressed_ = false;
      }

      void notify_actual_state(bool is_pressed_now){
        if(!is_pressed_ && is_pressed_now)
        {
          was_pressed_ = true;
          PressedTime = millis();
        }
        else if(is_pressed_ && !is_pressed_now)
        {
          was_unpressed_ = true;

          unsigned long CurrentTime = millis();
          if (CurrentTime - PressedTime > 3000){
            was_long_pressed_ = true;
          }

        }

        
        is_pressed_ = is_pressed_now;
      }

      bool was_pressed(){
        return was_pressed_;
      }

      bool was_unpressed(){
        return was_unpressed_;
      }

      bool was_long_pressed(){
        return was_long_pressed_;
      }

      void reset_memory(){
        was_pressed_ = false;
        was_unpressed_ = false;
        was_long_pressed_ = false;
      }
      
private:
  bool is_pressed_;
  bool was_pressed_;
  bool was_long_pressed_;
  bool was_unpressed_;
  unsigned long PressedTime = millis();
  };

  #endif // Button_c