#include <Servo.h>

const int left_pin = 12;
const int right_pin = 13;
const int button_pin = 2;
const int diode_pin = 11;

enum states{ st_stop = 0, st_drive_left, st_drive_right, st_num_states};
enum speeds{ stopped = 0, slow = 30, medium = 50, faster = 80, fast = 100};
enum turning{ turning_none = 0, turning_left, turning_right};

enum sensor {f_left = 0, c_left = 1, center = 2, c_right = 3, f_right = 4};


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
bool dioda = false;
float forward_error = 0;
float back_error = 0;
int turning_state = turning_none;
bool last_seen_middle = true;
int ignored_changes = -1;
bool previous_black = false;

void drive_forward() {
  if (!sens.is_white(center) && sens.is_white(c_left) && sens.is_white(c_right)) 
  {
    turning_state = turning_none;
    last_seen_middle = true;
    mov.forward(faster);
  }
  else if(!sens.is_white(c_left))
  {
    mov.left_forward(faster, faster + 10);
  }
  else
  {
    mov.right_forward(faster, faster + 10);
  }
}

bool turn_left(int & crossed_lines) {
  mov.left_inplace(faster);
  if (sens.changed_to_black(center))
    --crossed_lines;
  return crossed_lines <= 0;
}

bool turn_right(int & crossed_lines) {
  mov.right_inplace(faster);
  if (sens.changed_to_black(center))
    --crossed_lines;
  return crossed_lines <= 0;
}

void setup() {
  // put your setup code here, to run once:
  mov.attach();
  pinMode(button_pin, INPUT_PULLUP);
  pinMode(diode_pin, OUTPUT);
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
    ignored_changes = -1;
    previous_black = false;
    if (state == st_stop) {
      state = st_drive_left;
    }
    else {
      state = st_stop;
    }
  }

  if(button.was_long_pressed() ){
    if(dioda){
      digitalWrite(diode_pin, HIGH);
      }
    else {
      digitalWrite(diode_pin, LOW);
    }
    dioda = !dioda;
  }
  

  // cleanup
  button.reset_memory();

  // At the start finish the movement
  if (!sens.left_white() && !sens.right_white())
  {
    state = st_stop;
  }
  
  // movement disabled
  if (state == st_stop) {
    mov.stop();
    return;
  }

  // main algorithm

  int black_sensor = -1;

  // Check for left mark to specify the path
  if (!sens.left_white()) {
    if(ignored_changes == -1)
    {
      // If dioda is shining, choose other way
      if(dioda)
        state = st_drive_right;
       else
        state = st_drive_left;
        
      ignored_changes = 0;
    }
    black_sensor = st_drive_left;
  }

  // Check for right mark to specify the path
  if (!sens.right_white()) {
    if (ignored_changes == -1)
    {
      // If dioda is shining, choose other way
      if(dioda)    
        state = st_drive_right;
       else
        state = st_drive_left;
        
      ignored_changes = 0;
    }
    black_sensor = st_drive_right;
  }

  
  
  if (black_sensor != -1 && state != black_sensor)
  {
    // Black sensor is on ignored side
    previous_black = true; 
  }
  else
  {
    // Check if ignored sensor was blck before
    if (previous_black)
    {
      // Calculate how many times the sensor changed from black to white
      ++ignored_changes;
      previous_black = false;
    }
  }

  // Ignore other way at the split and merge
  if (ignored_changes >= 2)
  {
    ignored_changes = -1;
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
