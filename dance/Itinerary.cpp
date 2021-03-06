#ifndef Itinerary_c
#define Itinerary_c

#include <Arduino.h>
#include "Enums.cpp"
#include "Reader.cpp"

class Point {
public:
  byte col,row;

  Point()
    :Point(0,0)
  {
    
  }

  Point(byte col, byte row)
    :col(col), row(row)
  {
    
  }
  
  byte col_dist_to(Point p) {
    return p.col - col;  
  }

  byte y_dist_to(Point p) {
    return p.row - row;
  }
};

class Waypoint {
public:
  Point pt;
  bool col_first;
  unsigned tim;
  
  Waypoint()
    :pt(), col_first(false), tim(0)
  {

  }

  Waypoint(Point pt, bool col_first, unsigned tim) 
    :pt(pt), col_first(col_first), tim(tim)
  {

  }

  void print_to_serial() {
    Serial.print('[');
    if (col_first) {
      Serial.print(char('A' + pt.col));
      Serial.print(", ");
      Serial.print(1 + pt.row);
    }
    else {
      Serial.print(1 + pt.row);
      Serial.print(", ");
      Serial.print(char('A' + pt.col));
    }
    Serial.print(", t");
    Serial.print(tim);
    Serial.print(']');
  }
};

class Waypoint_Node {
public:
  Waypoint wp;
  Waypoint_Node * next;
  
  Waypoint_Node(Waypoint wp, Waypoint_Node * next) 
    :wp(wp), next(next)
  {
    
  }
};

class Itinerary {
public:
  result parse_input(Reader &in, const char ** error_msg) {
    if (!in.move_next()) {
      *error_msg = "Unexpected empty input";
      return r_err;
    }

    Waypoint n_start_wp;
    headings n_start_heading;
    Waypoint_Node *n_point_list;
    if (get_starting_state(in, n_start_wp, n_start_heading, error_msg) != r_ok) {
      return r_err;
    }

    result res;
    Point new_point;
    bool new_col_first;
    unsigned new_tim;
    Waypoint_Node ** next_link = &n_point_list;
    while ((res = get_next_target(in, new_point, new_col_first, new_tim, error_msg)) == r_ok) {
      *next_link = new Waypoint_Node(Waypoint(new_point, new_col_first, new_tim), nullptr);
      next_link = &((*next_link)->next);
    }

    if (res == r_ok || res == r_eof) {
      start_wp_ = n_start_wp;
      start_head_ = n_start_heading;
      point_list_ = n_point_list;
      target_ = point_list_;
      return r_ok;
    }
    else {
      delete_list(n_point_list);
      return r_err;
    }
  }

  Itinerary() 
    :point_list_(nullptr), start_wp_(Point(0, 0), false, 0), start_head_(north), target_(nullptr)
  {
    
  }
  
  

  ~Itinerary() {
    delete_list(point_list_);
  }
  

  Waypoint get_start_waypoint() const {
    return start_wp_;
  }

  headings get_start_heading() const {
    return start_head_;
  }

  Waypoint get_target_waypoint() const {
    return target_->wp;
  }

  bool advance_waypoint() {
    if (target_) {
      target_ = target_->next;
    }
    return target_ != nullptr;
  }

  void reset() {
    target_ = point_list_;
  }

  void print_to_serial() {
    Serial.print("Starting position: "); 
    start_wp_.print_to_serial();
    Serial.println();
    
    for (Waypoint_Node * c_node = point_list_; c_node; c_node = c_node->next) {
      c_node->wp.print_to_serial();
      Serial.println();
    }
  }
private:

  Waypoint_Node * point_list_;

  Waypoint start_wp_;
  headings start_head_;

  Waypoint_Node * target_;

  Itinerary(const Itinerary &other) = delete;
  Itinerary & operator =(const Itinerary &other) = delete;

  static result skip_whitespace(Reader &in) {
    if (in.get_current() == '\0') {
      return r_eof;
    }

    while (isSpace(in.get_current())) {
      if (!in.move_next()) {
        return r_eof;
      }
    }
    return r_ok;
  }

  static result get_number(Reader &in, unsigned &number, const char **error_msg) {
    number = 0;

    if (!isDigit(in.get_current())) {
      Serial.println(in.get_current());
      *error_msg = "Expected number";
      return r_err;
    }

    while (isDigit(in.get_current())) {
      number = 10 * number + (in.get_current() - '0');
      if (!in.move_next()) {
        break;
      }
    }
    return r_ok;
  }

  static result get_heading(Reader &in, headings &head, const char **error_msg) {
    char c = in.get_current();
    switch (c) {
      case 'n':
      case 'N':
        head = north;
        break;
      case 'e':
      case 'E':
        head = east;
        break;
      case 's':
      case 'S':
        head = south;
        break;
      case 'w':
      case 'W':
        head = west;
        break;
      default:
        *error_msg = "Unknown heading, expected [nN]|[eE]|[sS]|[wW]";
        return r_err;
    }
    in.move_next();
    return r_ok;
  }

  static result get_char_pos(Reader &in, byte &pos, const char **error_msg) {
    char char_pos = in.get_current();

    if (!isAlpha(char_pos)) {
      *error_msg = "Unknown character, expected a-iA-I";
      return r_err;
    }

    if (isLowerCase(char_pos)) {
      pos = char_pos - 'a';
    }
    else if (isUpperCase(char_pos)) {
      pos = char_pos - 'A';
    }
    else {
      *error_msg = "Unknown character, expected a-iA-I";
      return r_err;
    }

    if (pos > 8) {
      *error_msg = "Position out of range a-iA-I";
      return r_err;
    }

    in.move_next();
    return r_ok;
  }

  static result get_number_pos(Reader &in, byte &pos, const char ** error_msg) {
    unsigned new_num;
    if (get_number(in, new_num, error_msg) != r_ok) {
      return r_err;
    }
    if (new_num < 1 || 9 < new_num) {
      *error_msg = "Position out of range 1-9";
      return r_err;
    }
    //Offset to 0 based indexing
    new_num -= 1;
    pos = (byte)new_num;
    return r_ok;
  }

  static result get_time(Reader &in, unsigned &number, const char ** error_msg) {
    if (in.get_current() != 't' && in.get_current() != 'T') {
      *error_msg = "Expected t/T time delimiter";
      number = 0;
      return r_err;
    }
    //Skip T
    if (!in.move_next()) {
      *error_msg = "Unexpected end of input1";
      return r_err;
    }

    if (skip_whitespace(in) != r_ok) {
      *error_msg = "Unexpected end of input2";
      return r_err;
    }
    return get_number(in, number, error_msg);
  }
  
  static result get_starting_state(Reader &in, Waypoint &starting_wp, headings &head, const char **error_msg) {
    if (skip_whitespace(in) != r_ok) {
      *error_msg = "Unexpected end of input3";
      return r_err;
    }
    
    if (get_pos(in, starting_wp.pt, starting_wp.col_first, error_msg) != r_ok) {
      return r_err;
    }

    if (skip_whitespace(in) != r_ok) {
      *error_msg = "Unexpected end of input4";
      return r_err;
    }
    if (get_heading(in, head, error_msg) != r_ok) {
      return r_err;
    }
    return r_ok;
  }

  static result get_next_target(Reader &in, Point &next_target, bool &col_first, unsigned &tar_time, const char ** error_msg) {
    if (in.get_current() != '\0' && !isSpace(in.get_current())) {
      Serial.print(byte(in.get_current()));
      Serial.print(in.text_position());
      *error_msg = "Expected whitespace";
      return r_err;
    }
    
    if (skip_whitespace(in) == r_eof) {
      return r_eof;
    }

    if (get_pos(in, next_target, col_first, error_msg) != r_ok) {
      return r_err;
    }

    if (skip_whitespace(in) != r_ok) {
      *error_msg = "Unexpected end of input5";
      return r_err;
    }

    if (get_time(in, tar_time, error_msg) != r_ok) {
      return r_err;
    }
    return r_ok;
  }

  static result get_pos(Reader &in, Point &pos, bool &col_first, const char ** error_msg) {
    if (isAlpha(in.get_current())) {
      col_first = true;
      if (get_char_pos(in, pos.col, error_msg) != r_ok) {
        return r_err;
      }

      if (skip_whitespace(in) != r_ok) {
        *error_msg = "Unexpected end of input6";
        return r_err;
      }

      if (get_number_pos(in, pos.row, error_msg) != r_ok) {
        return r_err;
      } 
    }
    else {
      col_first = false;
      if (get_number_pos(in, pos.row, error_msg) != r_ok) {
        return r_err;
      }

      if (skip_whitespace(in) != r_ok) {
        *error_msg = "Unexpected end of input7";
        return r_err;
      }
      if (get_char_pos(in, pos.col, error_msg) != r_ok) {
        return r_err;
      }   
    }
    return r_ok;
  }

  void delete_list(Waypoint_Node *point_list) {
    while (point_list) {
      Waypoint_Node * to_delete = point_list;
      point_list = point_list->next;
      delete(to_delete);
    }
  }
};

#endif // Itinerary_c
