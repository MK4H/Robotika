#include <Servo.h>

const int left_pin = 12;
const int right_pin = 13;
const int button_pin = 2;
const int diode_pin = 11;

enum states{ st_stop = 0, st_drive_left, st_drive_right, st_num_states};
enum speeds{ stopped = 0, slow = 30, medium = 50, faster = 80, fast = 100};
enum headings { north = 0, west, south, east, num_headings};

enum sensor {f_left = 0, c_left = 1, center = 2, c_right = 3, f_right = 4};
enum result {r_ok, r_err, r_eof};

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

class Point {
public:
  unsigned x,y;

  Point()
    :Point(0,0)
  {
    
  }

  Point(unsigned x, unsigned y)
    :x(x), y(y)
  {
    
  }
  
  unsigned x_dist_to(Point p) {
    return p.x - x;  
  }

  unsigned y_dist_to(Point p) {
    return p.y - y;
  }
};

class Waypoint_Node {
public:
  Point p;
  unsigned tim;
  Waypoint_Node * next;
  
  Waypoint_Node(Point p, unsigned tim, Waypoint_Node * next) 
    :p(p), tim(tim), next(next)
  {
    
  }
};

class Itinerary {
public:
  static result parse_input(char * input, Itinerary &it) {
    char * input_left = input;
    
    if (get_starting_pos(&input_left, it.c_pos_, it.c_head_) != r_ok) {
      return r_err;
    }
    result res;
    Point new_point;
    unsigned new_tim;
    Waypoint_Node ** next_link = &it.point_list_;
    while ((res = get_next_target(&input_left, new_point, new_tim)) == r_ok) {
      *next_link = new Waypoint_Node(new_point, new_tim, nullptr);
      next_link = &((*next_link)->next);
    }

    if (res == r_ok || res == r_eof) {
      it.target_ = it.point_list_->next;
      return r_ok;
    }
    else {
      it.delete_list();
      return r_err;
    }
  }

  Itinerary() 
    :point_list_(nullptr), c_pos_(0,0), c_head_(north), target_(nullptr)
  {
    
  }
  
  ~Itinerary() {
    delete_list();
  }
  
private:

  Waypoint_Node * point_list_;

  Point c_pos_;
  headings c_head_;

  Waypoint_Node * target_;

  

  static char get_and_move(char ** text) {
    return *((*text)++);
  }
  
  static void skip_whitespace(char ** text) {
    while (isWhitespace(**text)) {
      ++(*text);
    }
  }

  static result get_number(char ** text, unsigned &number) {
    unsigned num = 0;

    if (!isDigit(**text)) {
      return r_err;
    }
    while (isDigit(get_and_move(text))) {
      num = 10 * num + (**text - '0');
    }
    number = num;
    return r_ok;
  }

  static result get_heading(char ** text, headings &head) {
    switch (get_and_move(text)) {
      case 'n':
      case 'N':
        head = north;
        return r_ok;
      case 'e':
      case 'E':
        head = east;
        return r_ok;
      case 's':
      case 'S':
        head = south;
        return r_ok;
      case 'w':
      case 'W':
        head = west;
        return r_ok;
      default:
        return r_err;
    }
  }

  static result get_char_pos(char ** text, unsigned &pos) {
    char char_pos = get_and_move(text);
    if (!isAlpha(char_pos)) {
      return r_err;
    }

    if (isLowerCase(char_pos)) {
      pos = char_pos - 'a';
      return r_ok;
    }

    if (isUpperCase(char_pos)) {
      pos = char_pos - 'A';
      return r_ok;
    }
    return r_err;
  }

  static result get_number_pos(char ** text, unsigned &pos) {
    if (get_number(text, pos) != r_ok) {
      return r_err;
    }
    if (pos < 1 || 9 < pos) {
      return r_err;
    }
    //Offset to 0 based indexing
    pos -= 1;
    return r_ok;
  }

  static result get_time(char ** text, unsigned &number) {
    if (**text != 't' && **text != 'T') {
      number = 0;
      return r_err;
    }
    //Skip T
    (*text)++;
    return get_number(text, number);
  }
  
  static result get_starting_pos(char ** text, Point &starting_pos, headings &head) {
    skip_whitespace(text);
    if (get_char_pos(text, starting_pos.x) != r_ok) {
      return r_err;
    }

    if (get_number_pos(text, starting_pos.y) != r_ok) {
      return r_err;
    }

    if (get_heading(text, head) != r_ok) {
      return r_err;
    }
    return r_ok;
  }

  static result get_next_target(char ** text, Point &next_target, unsigned &tar_time) {
    skip_whitespace(text);
    if (**text == '\0') {
      return r_eof;
    }
    
    if (get_char_pos(text, next_target.x) != r_ok) {
      return r_err;
    }

    if (get_number_pos(text, next_target.y) != r_ok) {
      return r_err;
    }

    if (!isWhitespace(**text)) {
      return r_err;
    }
    
    skip_whitespace(text);
    
    if (get_time(text, tar_time) != r_ok) {
      return r_err;
    }
    return r_ok;
  }

  void delete_list() {
    Waypoint_Node * current = point_list_;
    while (current) {
       Waypoint_Node * to_delete = current;
       current = current->next;
       delete(to_delete);
    }
  }
};


Movement mov;
Sensors sens;
Button button;
int state = st_stop;
bool dioda = false;

class Move_manager
{
public: 
  bool move_forward(int segments)
  {
    // Initialization
    if(forward_segments == -1)
      forward_segments = segments;
      
    drive_forward();
    if((sens.changed_to_black(f_left) && sens.is_white(f_right)) || (sens.changed_to_black(f_right) && sens.is_white(f_left)))
    {
      --forward_segments;
      for (int i = 0; i < num_headings; ++i)
        actual_crossroad[i] = false;
    }

    if(forward_segments == 0)
    {
      if(sens.is_black(f_left))
        actual_crossroad[(actual_heading + 1) % num_headings] = true;
      
      if(sens.is_black(f_right))
        actual_crossroad[(actual_heading - 1) % num_headings] = true;

      drive_forward();
      // Move a bit further to get middle on the crossroad
      // if middle on the crossroad
      {
        forward_segments = -1;
        actual_crossroad[actual_heading] = sens.is_black(c_left) || sens.is_black(center) || sens.is_black(c_right);
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
    mov.stop();
    forward_segments = -1;
    rotate_cross_lines = 0;
  }
  
private:

  bool actual_crossroad[num_headings];
  int forward_segments = -1;
  int rotate_cross_lines = 0;
  int actual_heading;
  
  void drive_forward() {
    if(sens.is_black(c_left) && sens.is_white(f_left))
      mov.left_forward(faster, faster + 10);
    else if(sens.is_black(c_right) && sens.is_white(f_right))
      mov.right_forward(faster, faster + 10);
    else 
      mov.forward(faster);
    
  }
  
  bool turn_left() {
    mov.left_inplace(faster);
    if (sens.changed_to_black(center))
      --rotate_cross_lines;
    return rotate_cross_lines <= 0;
  }
  
  bool turn_right() {
    mov.right_inplace(faster);
    if (sens.changed_to_black(center))
      --rotate_cross_lines;
    return rotate_cross_lines <= 0;
  }
};

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
  
  // movement disabled
  if (state == st_stop) {
    mov.stop();
    return;
  }

  // main algorithm
 
  switch(state) {
    case st_stop:
      mov.stop();
      break;
  }
}
