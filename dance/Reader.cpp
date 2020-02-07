#ifndef Reader_c
#define Reader_c

#include <Arduino.h>
#include <EEPROM.h>

class Reader {
public:
    virtual char get_current() const = 0;
    virtual bool move_next() = 0;
    virtual unsigned text_position() const = 0;
};

class StringReader : public Reader {
public:
    StringReader(const char *str) : orig_(str),str_(nullptr) {

    }

    char get_current() const override {
        return str_ ? *str_ : '\0';
    } 

    bool move_next() override {
        if (!str_) {
            str_ = orig_;
            return *str_ != '\0';
        }

        if (*str_ == '\0') {
            return false;
        }

        return *(++str_) != '\0';
    }

    unsigned text_position() const override {
        return str_ - orig_;
    }
private:
    const char *orig_;
    const char *str_;
};

class EEPROMReader : public Reader {
public:
    EEPROMReader():addr_(0), c_char_('\0') {

    }

    char get_current() const override {
        return c_char_;
    }

    bool move_next() override {
        if (addr_ == 0) {
            c_char_ = EEPROM.read(addr_++);
            //If the magic number at address 0 is invalid (is not 0)
            if (c_char_ != 0) {
                c_char_ = '\0';
                return false;
            }
            else {
                // Magic number is valid, read the following choreography
                c_char_ = EEPROM.read(addr_++);
                return c_char_ != '\0';
            }
        }

        // If we had not read the whole choreography up to the \0 char
        // read the next char
        if (c_char_ != '\0') {
            c_char_ = EEPROM.read(addr_++);
            return c_char_ != '\0';
        }

        return false;
    }

    unsigned text_position() const override {
        return addr_ > 0 ? addr_ - 1 : 0;
    }

private:
    int addr_;
    char c_char_;
};

#endif // Reader_c
