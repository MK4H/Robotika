#include <Servo.h>

const int left_pin = 12;
const int right_pin = 13;
const int button_pin = 2;

enum states{ stop = 0, forward, backward, rotate_left, rotate_right, num_states};

class sensors {
public:
  
  sensors()
    :state_(0) 
  { 

  }

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

class movement {
public:
  movement() {
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

movement mov;
sensors sens;
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
  // put your main code here, to run repeatedly:
  if (!digitalRead(button_pin)) {
    if (state == stop) {
      state = forward;
    }
    else {
      state = stop;
    }
  }

  if (state == stop) {
    mov.stop();
    return;
  }

  sens.update();


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
}
