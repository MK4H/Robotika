#ifndef Driver_c
#define Driver_c

#include "Itinerary.cpp"
#include "MoveManager.cpp"

class Driver {
public:
    Driver(Itinerary * itin, MoveManager * move)
        : itin_(itin), move_(move) {
        position_ = itin_->get_start_pt();
        heading_ = itin_->get_start_heading();
        target_ = itin_->get_target_waypoint();
        
    }

    ~Driver() {
        delete(itin_)
        delete(move_)
    }

    void loop() {
        
    }
private:
    enum class state {turn_left, turn_right, forward}

    Itinerary *itin_;
    MoveManager *move_;

    Point position_;
    headings heading_;

    Waypoint target_;
    state state_;

    state get_next_state() {
        if ((position_.col == target_.pt.col) && (position_.row == target_.pt.row)) {
            if (itin->advance_waypoint()) {
                target_ = itin->get_target_waypoint();
            }
            else {
                target_ = itin->get_start_waypoint();
            }
        }

        if (target_.col_first && (position_.col != target_.pt.col)) {
            return get_to_column();
        }
        else if (!target_.col_first && (position_.row != target_.pt.row)) {
            return get_to_row();
        }
        else if (position_.col != target_.pt.col) {
            return get_to_column();
        }
        else if (position_.row != target_.pt.row) {
            return get_to_row();
        }
        else {
            //TODO: Error
        }
    }

    state get_to_column() {

    }

    state get_
}


#endif Driver_c