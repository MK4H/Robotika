#include <Servo.h>

const int left_pin = 12;
const int right_pin = 13;
const int button_pin = 2;

enum states{ st_stop = 0, st_drive_left, st_drive_right, st_num_states};
enum speeds{ stopped = 0, slow = 20, medium = 50, fast = 80, faster = 100};

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

  bool left_white() {
    return state_ & (1 << 3); 
  }

  bool cleft_white(){
    return state_ & (1 << 4); 
  }

  bool center_white() {
    return state_ & (1 << 5); 
  }

  bool cright_white() {
    return state_ & (1 << 6); 
  }

  bool right_white() {
    return state_ & (1 << 7); 
  }
private:
  int state_;
};

class Movement {
public:
  Movement() {
  }

  void attach() {
    left_.attach(left_pin,500,2500);
    right_.attach(right_pin,500,2500);
  }

  void forward(float percent) {
    percent = cap_percent(percent);
    left_.writeMicroseconds((int)(1500 + max_spd * (percent/100.0f)));
    right_.writeMicroseconds((int)(1500 - max_spd * (percent/100.0f)));
  }

  void backward(float percent) {
    forward(-percent);
  }

  void left_forward(float forw_perc, float left_perc) {
    left_speed(left_perc);
    right_speed(forw_perc);
  }

  void right_forward(float forw_perc, float right_perc) {
    left_speed(forw_perc);
    right_speed(right_perc);
  }

  void left_speed(float percent) {
    percent = cap_percent(percent);
    left_.writeMicroseconds((int)(1500 + max_spd * (percent/100.0f)));
    
  }

  void right_speed(float percent) {
    percent = cap_percent(percent);
    right_.writeMicroseconds((int)(1500 - max_spd * (percent/100.0f)));
  }

  void left_inplace(float percent) {
    percent = cap_percent(percent);
    left_.writeMicroseconds((int)(1500 - max_spd * (percent/100.0f)));
    right_.writeMicroseconds((int)(1500 - max_spd * (percent/100.0f)));

  }

  void right_inplace(float percent) {
    left_inplace(-percent);
  }
  
  void stop() {
    left_.writeMicroseconds(1500);
    right_.writeMicroseconds(1500);
  }
private:
  Servo left_, right_;

  const int max_spd = 500;

  int cap_percent(int percent) {
    if (percent > 100) {
      return 100;
    }

    if (percent < -100) {
      return -100;
    }
    return percent;
  }
};


Movement mov;
Sensors sens;
Button button;
int state = st_stop;

void drive_left() {
  if (!sens.center_white() && sens.cleft_white() && sens.cright_white()) {
    mov.forward(faster);
  }
  else if (!sens.center_white() && !sens.cleft_white()){
    mov.left_forward(medium, slow);
  }
  else if (!sens.center_white() && !sens.cright_white()){
    mov.right_forward(medium, slow);
  }
  else if (!sens.cleft_white()) {
    mov.left_inplace(slow);
  }
  else if (!sens.cright_white()){
    mov.right_inplace(slow);
  }
  else {
    mov.right_inplace(slow);
  }
}

void drive_right() {
  if (!sens.center_white() && sens.cright_white()) {
    mov.left_forward(fast, slow);
  }
  else if (!sens.cright_white()) {
    mov.right_inplace(slow);
  }
  else {
    mov.right_inplace(slow);
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
    if (state == st_stop) {
      state = st_drive_left;
    }
    else {
      state = st_stop;
    }
  }

  // cleanup
  button.reset_memory();

  // movement disabled
  if (state == st_stop) {
    mov.stop();
    return;
  }

  // main algorithm


  if (!sens.left_white()) {
    state = st_drive_left;
  }

  if (!sens.right_white()) {
    state = st_drive_right;
  }
 
  switch(state) {
    case st_stop:
      mov.stop();
      break;
    case st_drive_left:
      drive_left();
      break;
    case st_drive_right:
      drive_right();
      break;
  }
}
