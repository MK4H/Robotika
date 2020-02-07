#ifndef Driver_c
#define Driver_c

#include <Arduino.h>
#include "Itinerary.cpp"
#include "MoveManager.cpp"
#include "Button.cpp"

class Driver {
public:
    Driver(Itinerary * itin, MoveManager * move, Button *button)
        : itin_(itin), move_(move), butt_(button), start_time_(0),
          pos_(), heading_(north), target_(), state_(state::startup), 
          going_home_(false), button_released_(false) {
    }

    void init_from_itin() {
        state_ = state::startup;
        itin_->reset();
        pos_ = itin_->get_start_waypoint().pt;
        heading_ = itin_->get_start_heading();
        target_ = itin_->get_target_waypoint();
        move_->reset();
        move_->SetHeading(heading_);
        going_home_ = false;
        button_released_ = false;
        drive_immediately_ = false;
    }

    void drive_immediately() {
        drive_immediately_ = true;
    }

    void loop() {
        if (butt_->was_unpressed()) {
            button_released_ = true;
        }

        // State machine
        switch (state_)
        {
        case state::startup: 
            if (check_and_reset(button_released_) || check_and_reset(drive_immediately_)) {
                Serial.println("Starting next run.");
                itin_->reset();
                target_ = itin_->get_target_waypoint();
                start_time_ = millis();
                follow_path();
            }
            break;
        case state::forward:
            if (move_forward(button_released_)) {
                follow_path();
            }
            break;
        case state::turn_left:
            if (turn(state::turn_left, button_released_)) {
                follow_path();
            }
            break;
        case state::turn_right:
            if (turn(state::turn_right, button_released_)) {
                follow_path();
            }
            break;
        case state::wait:
            if (wait(button_released_, drive_immediately_)) {
                follow_path();
            }
            break;
        case state::finished:
            if (check_and_reset(button_released_)) {
                Serial.println("Going home.");
                go_home();
                follow_path();
            }
            break;
        default:
            Serial.print("Unknown state in driver loop");
            break;
        }
    }
private:
    enum class state { startup, turn_left, turn_right, forward, wait, finished};
    
    Itinerary *itin_;
    MoveManager *move_;
    Button *butt_;
    
    unsigned long start_time_;

    Point pos_;
    headings heading_;

    Waypoint target_;
    state state_;
    int amount_;

    bool going_home_;
    bool button_released_;
    bool drive_immediately_;

    bool check_and_reset(bool &var) {
        bool old = var;
        var = false;
        return old;
    }

    void go_home() {
        going_home_ = true;
        target_ = itin_->get_start_waypoint();
    }

    bool move_forward(bool &button_released) {
        

        bool move_finished = move_->move_forward(amount_);
        if (move_finished) {
            update_pos(pos_, heading_, amount_);
            if (check_and_reset(button_released)) {
                go_home();
            }
        }
        return move_finished;
    }

    bool turn(state turn_state, bool &button_released) {
        bool turn_finished = turn_state == state::turn_left ? move_->rotate_left(amount_) : move_->rotate_right(amount_);
        if (turn_finished) {
            update_heading(heading_, turn_state, amount_);
            // We cannot reset the robot during a turn due to the implementation
            // Check after the turn is finished
            if (check_and_reset(button_released)) {
                go_home();
            }
        }
        
        return turn_finished;
    }

    bool wait(bool &button_released, bool &drive_immediately) {
        if (check_and_reset(button_released)) {
            go_home();
            return true;
        }
        
        return time_passed(target_.tim) || check_and_reset(drive_immediately);
    }

    void follow_path() {
        while (!get_next_move(pos_, heading_, target_, state_ , amount_)) {
            // Arrived at the target, it's time to move on
            if (going_home_) {
                if (heading_ != itin_->get_start_heading()) {
                    rotate(pos_, heading_, itin_->get_start_heading(), state_, amount_);
                    return;
                }
                
                init_from_itin();
                return;
            }
            
            if (!time_passed(target_.tim)) {
                amount_ = 0;
                state_ = state::wait;
                return;
            }

            if (itin_->advance_waypoint()) {
                // The itinerary continues
                target_ = itin_->get_target_waypoint();
            }
            else {
                // The itinerary is finished
                Serial.println("Finished run.");
                state_ = state::finished;
                return;
            }
        }
    }

    bool time_passed(unsigned long time) {
        unsigned long c_time = millis() - start_time_;
        // Convert from milliseconds to deciseconds (tenths of a second), as that is the precision of the time given
        c_time = c_time / 100;
        return time < c_time;
    }

    static void update_pos(Point &pos, headings heading, int amount) {
        switch (heading)
        {
        case north:
            pos.row += amount;
            break;
        case south:
            pos.row -= amount;
            break;
        case east:
            pos.col += amount;
            break;
        case west:
            pos.col -= amount;
            break;
        default:
            Serial.println("Unknown heading.");
            break;
        }
    }

    static void update_heading(headings &heading, state turn_state, int amount) {
        heading = (heading + num_headings + (turn_state == state::turn_left ? amount : -amount)) % num_headings;
    }

    /**
     * 
     * \return true if robot has not reached `target` and 
     *     `ret_state` and `ret_amount` hold valid values,
     *     false if we have reached the `target`.
     */
    static bool get_next_move(Point c_pos, headings c_head, Waypoint target, state &ret_state, int &ret_amount) {

        // If the robot is at the target position
        if ((c_pos.col == target.pt.col) && (c_pos.row == target.pt.row)) {
            // Arrived at target, ready to contiue
            return false;
        }

        // If the robot should go to target COLUMN first and is not at the COLUMN
        if (target.col_first && (c_pos.col != target.pt.col)) {
            get_to_column(c_pos, c_head, target, ret_state, ret_amount);
        }
        // If it should go to target ROW first and is not at the ROW
        else if (!target.col_first && (c_pos.row != target.pt.row)) {
            get_to_row(c_pos, c_head, target, ret_state, ret_amount);
        }
        // Is at the first target coord, check if the second is columns
        else if (c_pos.col != target.pt.col) {
            get_to_column(c_pos, c_head, target, ret_state, ret_amount);
        }
        // Or if it is rows
        else if (c_pos.row != target.pt.row) {
            get_to_row(c_pos, c_head, target, ret_state, ret_amount);
        }
        else {
            Serial.print("Error next move");
        }
        return true;
    }

    /**
     * Rotate the robot `from` heading `to` heading
     *  
     * \param pos The position where the robot is rotating.
     * \param from Initial heading.
     * \param to Target heading.
     * \param ret_state Output argument, returning the resulting state, 
     *     either state::turn_left or state.turn_right. 
     * \param ret_amount Output argument, returning the amount of quarters
     *     the robot should turn in the `ret_state` direction.
     */
    static void rotate(Point pos, headings from, headings to, state &ret_state, int &ret_amount) {
        int half_rotation = num_headings/2;
        int change = to - from;
        
        if (change < -half_rotation) {
            change = -half_rotation - change;
        }
        else if (change > half_rotation) {
            change = half_rotation - change;
        }
        
        ret_amount = change;

        if (change == half_rotation || change == -half_rotation) {
            // Ensure that we will always turn towards the middle of the map
            // if on the edge
            if (from == north) {
                ret_state = pos.col == 0 ? state::turn_right : state::turn_left;
            }
            else if (from == south) {
                ret_state = pos.col == 0 ? state::turn_left : state::turn_right;
            }
            else if (from == east) {
                ret_state = pos.row == 0 ? state::turn_left : state::turn_right;
            }
            else {
                ret_state = pos.row == 0 ? state::turn_right : state::turn_left;
            }
        }
        else if (change > 0) {
            ret_state = state::turn_left;
        }
        else {
            ret_state = state::turn_right;
        }
        ret_amount = abs(ret_amount);
    }

    static void get_to_column(Point c_pos, headings c_head, Waypoint target, state &ret_state, int &ret_amount) {
        int diff = target.pt.col - c_pos.col;
        if (diff < 0 && c_head != west) {
            rotate(c_pos, c_head, west, ret_state, ret_amount);
        }
        else if (diff > 0 && c_head != east) {
            rotate(c_pos, c_head, east, ret_state, ret_amount);
        }
        else {
            ret_amount = abs(diff);
            ret_state = state::forward;
        }
    }

    static void get_to_row(Point c_pos, headings c_head, Waypoint target, state &ret_state, int &ret_amount) {
        int diff = target.pt.row - c_pos.row;
        if (diff < 0 && c_head != south) {
            rotate(c_pos, c_head, south, ret_state, ret_amount);
        }
        else if (diff > 0 && c_head != north) {
            rotate(c_pos, c_head, north, ret_state, ret_amount);
        }
        else {
            ret_amount = abs(diff);
            ret_state = state::forward;
        }
    }
};


#endif // Driver_c
