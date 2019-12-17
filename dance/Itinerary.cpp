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

class Waypoint_Node {
public:
  Point p;
  bool col_first;
  unsigned tim;
  
  Waypoint_Node * next;
  
  Waypoint_Node(Point p, bool col_first, unsigned tim, Waypoint_Node * next) 
    :p(p), col_first(col_first), tim(tim), next(next)
  {
    
  }
};

class Itinerary {
public:
  result parse_input(Reader &in) {
    if (get_starting_pos(in, start_pos_, start_head_) != r_ok) {
      return r_err;
    }
    result res;
    Point new_point;
    bool new_col_first;
    unsigned new_tim;
    Waypoint_Node ** next_link = &point_list_;
    while ((res = get_next_target(in, new_point, new_col_first, new_tim)) == r_ok) {
      *next_link = new Waypoint_Node(new_point, new_col_first, new_tim, nullptr);
      next_link = &((*next_link)->next);
    }

    if (res == r_ok || res == r_eof) {
      target_ = point_list_;
      return r_ok;
    }
    else {
      delete_list();
      return r_err;
    }
  }

  Itinerary() 
    :point_list_(nullptr), start_pos_(0,0), start_head_(north), target_(nullptr)
  {
    
  }
  
  

  ~Itinerary() {
    delete_list();
  }
  
  Point get_start_pos() const {
    return start_pos_;
  }

  headings get_start_heading() const {
    return start_head_;
  }

  Waypoint_Node * get_target_waypoint() const {
    return target_;
  }

  bool advance_waypoint() {
    if (target_) {
      target_ = target_->next;
    }
    return target_ == nullptr;
  }

  void reset() {
    target_ = point_list_;
  }
private:

  Waypoint_Node * point_list_;

  Point start_pos_;
  headings start_head_;

  Waypoint_Node * target_;

  Itinerary(const Itinerary &other) = delete;
  Itinerary & operator =(const Itinerary &other) = delete;

  static result skip_whitespace(Reader &in) {
    if (in.get_current() == '\0') {
      return r_eof;
    }

    while (isWhitespace(in.get_current())) {
      if (!in.move_next()) {
        return r_eof;
      }
    }
    return r_ok;
  }

  static result get_number(Reader &in, unsigned &number) {
    number = 0;

    if (!isDigit(in.get_current())) {
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

  static result get_heading(Reader &in, headings &head) {
    char c = in.get_current();
    in.move_next();
    switch (c) {
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

  static result get_char_pos(Reader &in, byte &pos) {
    char char_pos = in.get_current();
    in.move_next();

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

  static result get_number_pos(Reader &in, byte &pos) {
    unsigned new_num;
    if (get_number(in, new_num) != r_ok) {
      return r_err;
    }
    if (new_num < 1 || 9 < new_num) {
      return r_err;
    }
    //Offset to 0 based indexing
    new_num -= 1;
    pos = (byte)new_num;
    return r_ok;
  }

  static result get_time(Reader &in, unsigned &number) {
    if (in.get_current() != 't' && in.get_current() != 'T') {
      number = 0;
      return r_err;
    }
    //Skip T
    if (!in.move_next()) {
      return r_err;
    }
    return get_number(in, number);
  }
  
  static result get_starting_pos(Reader &in, Point &starting_pos, headings &head) {
    if (skip_whitespace(in) != r_ok) {
      return r_err;
    }
    if (get_char_pos(in, starting_pos.col) != r_ok) {
      return r_err;
    }

    if (skip_whitespace(in) != r_ok) {
      return r_err;
    }
    if (get_number_pos(in, starting_pos.row) != r_ok) {
      return r_err;
    }

    if (skip_whitespace(in) != r_ok) {
      return r_err;
    }
    if (get_heading(in, head) != r_ok) {
      return r_err;
    }
    return r_ok;
  }

  static result get_next_target(Reader &in, Point &next_target, bool &col_first, unsigned &tar_time) {
    if (in.get_current() != '\0' && !isWhitespace(in.get_current())) {
      return r_err;
    }
    
    if (skip_whitespace(in) == r_eof) {
      return r_eof;
    }

    if (isAlpha(in.get_current())) {
      col_first = true;
      if (get_char_pos(in, next_target.col) != r_ok) {
        return r_err;
      }

      if (skip_whitespace(in) != r_ok) {
        return r_err;
      }
      if (get_number_pos(in, next_target.row) != r_ok) {
        return r_err;
      } 
    }
    else {
      col_first = false;
      if (get_number_pos(in, next_target.row) != r_ok) {
        return r_err;
      }

      if (skip_whitespace(in) != r_ok) {
        return r_err;
      }
      if (get_char_pos(in, next_target.col) != r_ok) {
        return r_err;
      }   
    }

    if (skip_whitespace(in) != r_ok) {
      return r_err;
    }

    if (get_time(in, tar_time) != r_ok) {
      return r_err;
    }
    return r_ok;
  }

  void delete_list() {
    while (point_list_) {
      Waypoint_Node * to_delete = point_list_;
      point_list_ = point_list_->next;
      delete(to_delete);
    }
  }
};

#endif // Itinerary_c