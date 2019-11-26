#include <Servo.h>

const int left_pin = 12;
const int right_pin = 13;
const int button_pin = 2;

enum states{ stop = 0, forward, backward, rotate_left, rotate_right, num_states};
enum speeds{ stopped = 0, slow = 20, medium = 50, fast = 100, faster = 500};

class Button {
public:

      Button(){
        is_pressed_ = false;
        was_pressed_ = false;
        was_unpressed_ = false;
      }

      void notify_actual_state(bool is_pressed_now){
        if(!is_pressed_ && is_pressed_now)
        {
          was_pressed_ = true;
        }
        else if(is_pressed_ && !is_pressed_now)
        {
          was_unpressed_ = true;
        }

        is_pressed_ = is_pressed_now;
      }

      bool was_pressed(){
        return was_pressed_;
      }

      bool was_unpressed(){
        return was_unpressed_;
      }

      void reset_memory(){
        was_pressed_ = false;
        was_unpressed_ = false;
      }
      
private:
  bool is_pressed_;
  bool was_pressed_;
  bool was_unpressed_;
  };

class Sensors {
public:
  
  Sensors()
    :state_(0) {}

  void update() {
    state_ = PIND;
  }

  bool get_left() {
    return state_ & (1 << 3); 
  }

  bool get_c_left(){
    return state_ & (1 << 4); 
  }

  bool get_center() {
    return state_ & (1 << 5); 
  }

  bool get_c_right() {
    return state_ & (1 << 6); 
  }

  bool get_right() {
    return state_ & (1 << 7); 
  }
private:
  int state_;
};

class Movement {
public:
  Movement() {
    turning_ = false;
  }

  void attach() {
    left_.attach(left_pin,500,2500);
    right_.attach(right_pin,500,2500);
  }

  void forward() {
    left_.writeMicroseconds(2500);
    right_.writeMicroseconds(500);
    turning_ = false;
  }

  void forward_left() {
    left_.writeMicroseconds(2000);
    right_.writeMicroseconds(500);
    turning_ = false;
  }

  void forward_right() {
    left_.writeMicroseconds(2500);
    right_.writeMicroseconds(1000);
    turning_ = false;
  }

  void left() {
    left_.writeMicroseconds(500);
    right_.writeMicroseconds(500);
    turning_ = true;
  }

  void right() {
    left_.writeMicroseconds(2500);
    right_.writeMicroseconds(2500);
    turning_ = true;
  }

  bool is_turning() {
    return turning_;
  }

  void stop() {
    left_.writeMicroseconds(1500);
    right_.writeMicroseconds(1500);
    turning_ = false;
  }
private:
  Servo left_, right_;
  bool turning_;
};

class SideSensors {
public:



private:



  };

Movement mov;
Sensors sens;
Button button;
int state = stop;
bool left_mark = true;

void drive_left() {
  if (!sens.get_center() && sens.get_c_left()) {
    mov.forward_right();
  }
  else if (!sens.get_c_left()){
    mov.left();
  }
  else {
    mov.left();
  }
}

void drive_right() {
  if (!sens.get_center() && sens.get_c_right()) {
    mov.forward_left();
  }
  else if (!sens.get_c_right()) {
    mov.right();
  }
  else {
    mov.right();
  }
}

void setup() {
  // put your setup code here, to run once:
  mov.attach();
  pinMode(button_pin, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
}

void loop() {
  // update info from ir sensors
  sens.update();
  
  // button press makes something happen..
  button.notify_actual_state(!digitalRead(button_pin));
  if (button.was_pressed()) {
    if (state == stop) {
      state = forward;
    }
    else {
      state = stop;
    }
  }

  // movement disabled
  if (state == stop) {
    mov.stop();
    return;
  }

  // main algorithm


  if (!sens.get_left()) {
    left_mark = true;
  }

  if (!sens.get_right()) {
    left_mark = false;
  }
 
  
  if (left_mark){
    drive_left();
  }
  else {
    drive_right();
  }

  // cleanup
  button.reset_memory();
}
