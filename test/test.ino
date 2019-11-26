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

  void forward(int percent) {
    percent = cap_percent(percent);
    left_.writeMicroseconds((int)(1500 + max_ms * percent/100.0f));
    right_.writeMicroseconds((int)(1500 - max_ms * percent/100.0f));
  }

  void backward(int percent) {
    forward(-percent);
  }

  void left_forward(int forw_perc, int left_perc) {
    left_speed(left_perc);
    right_speed(forw_perc);
  }

  void right_forward(int forw_perc, int right_perc) {
    left_speed(forw_perc);
    right_speed(right_perc);
  }

  void left_speed(int percent) {
    percent = cap_percent(percent);
    left_.writeMicroseconds((int)(1500 + max_ms * percent/100.0f));
    
  }

  void right_speed(int percent) {
    percent = cap_percent(percent);
    right_.writeMicroseconds((int)(1500 - max_ms * percent/100.0f));
  }

  void left_inplace(int percent) {
    percent = cap_percent(percent);
    left_.writeMicroseconds((int)(1500 - max_ms * percent/100.0f));
    right_.writeMicroseconds((int)(1500 - max_ms * percent/100.0f));

  }

  void right_inplace(int percent) {
    left_inplace(-percent);
  }
  
  void stop() {
    left_.writeMicroseconds(1500);
    right_.writeMicroseconds(1500);
  }
private:
  Servo left_, right_;

  const int max_ms = 500;

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
int state = stop;
bool left_mark = true;

const int forward_speed = 20;
const int turning_speed = 0;
const int ip_turning_speed = 50;

void drive_left() {
  if (!sens.center_white() && sens.cleft_white() & sens.cright_white()) {
    mov.forward(forward_speed);
  }
  else if (!sens.center_white() && !sens.cleft_white()){
    mov.left_forward(forward_speed, turning_speed);
  }
  else if (!sens.center_white() && !sens.cright_white()){
    mov.right_forward(forward_speed, turning_speed);
  }
  else if (!sens.cleft_white()) {
    mov.left_inplace(ip_turning_speed);
  }
  else if (!sens.cright_white()){
    mov.right_inplace(ip_turning_speed);
  }
  else {
    mov.right_inplace(ip_turning_speed);
  }
}

void drive_right() {
  if (!sens.center_white() && sens.cright_white()) {
    mov.left_forward(forward_speed, forward_speed/2);
  }
  else if (!sens.cright_white()) {
    mov.right_inplace(turning_speed);
  }
  else {
    mov.right_inplace(turning_speed);
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

  // cleanup
  button.reset_memory();

  // movement disabled
  if (state == stop) {
    mov.stop();
    return;
  }

  // main algorithm


  if (!sens.left_white()) {
    left_mark = true;
  }

  if (!sens.right_white()) {
    left_mark = false;
  }
 
  drive_left();
  delay(100);
  return;
  if (left_mark){
    drive_left();
  }
  else {
    drive_right();
  }


}
