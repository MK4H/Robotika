#ifndef Reader_c
#define Reader_c

class Reader {
public:
    virtual char get_current() const = 0;
    virtual bool move_next() = 0;
    virtual unsigned text_position() const = 0;
};

class StringReader : public Reader {
public:
    StringReader(char *str) : orig_(str),str_(str) {

    }

    char get_current() const override {
        return *str_;
    } 

    bool move_next() override {
        if (*str_ == '\0') {
            return false;
        }

        return *(++str_) != '\0';
    }

    unsigned text_position() const override {
        return str_ - orig_;
    }
private:
    char *orig_;
    char *str_;
};

#endif // Reader_c
