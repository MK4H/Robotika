#ifndef Itinerary_c
#define Itinerary_c

#include <Arduino.h>
#include "Sensors.cpp"
#include "Enums.cpp"

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

#endif // Itinerary_c