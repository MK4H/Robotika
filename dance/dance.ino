#include <EEPROM.h>
#include "Enums.cpp"
#include "Button.cpp"
#include "Sensors.cpp"
#include "Movement.cpp"
#include "Itinerary.cpp"
#include "MoveManager.cpp"
#include "Driver.cpp"


const int left_pin = 12;
const int right_pin = 13;
const int button_pin = 2;
const int diode_pin = 11;

Movement *mov;
Sensors *sens;
Button button;
Itinerary itin;
MoveManager *move_manager;
Driver *driver;
int state = st_parsing;
bool dioda = false;
const char* choreo = "1AN b2 t0 c3 t100 d4 t150 c3 t200 b2 t200 a1 t0";
//const char* choreo = "A1N c2 t120";

void init_pins() {
  mov->attach(left_pin, right_pin);
  // pins
  pinMode(button_pin, INPUT_PULLUP);
  pinMode(diode_pin, OUTPUT);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
}

void init_choreography() {
  Serial.println("Reading from EEPROM");
  char * err_msg;
  EEPROMReader prom_reader;
  if (itin.parse_input(prom_reader, &err_msg) != r_ok) {
    // Parsing from EEPROM failed, parse the default choreo
    Serial.println(err_msg);
    Serial.println("EEPROM choreo not found - using default:");
    StringReader def_reader(choreo);
    if (itin.parse_input(def_reader, &err_msg) != r_ok) {
      Serial.println(err_msg);
      Serial.println("FATAL ERROR, DEFAULT CHOREOGRAPHY DOES NOT WORK");
    }
  }
  else{
    Serial.println("EEPROM choreo loaded:");
  }
  itin.print_to_serial();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Setup start.");
  
  mov = new Movement();
  sens = new Sensors();
  move_manager = new MoveManager(mov, sens);

  init_pins();

  init_choreography();

  driver = new Driver(&itin, move_manager, &button);
  driver->init_from_itin();
  
  Serial.println("Setup complete, to start a run, press the button shortly.");
}

int stage = 0;
void loop() {
  // update info from ir sensors
  sens->update();
  // button press makes something happen..
  button.notify_actual_state(!digitalRead(button_pin));


  if (state == st_parsing) {
    if (button.was_unpressed()) {
      if (button.was_over_5_pressed()) {  
        // clear EEPROM
        Serial.println("EEPROM cleared.");
        for (int i = 0 ; i < EEPROM.length() ; i++) {
          EEPROM.update(i, 255);
        }
      }
      else {
        // Start driving
        Serial.println("Driving, to set new choreography, hold button for 2-5 seconds.");
        driver->init_from_itin();
        driver->drive_immediately();
        state = st_driving;
      }
    }

    if(Serial.available() > 0){
      String fromSerial = Serial.readString();
      // +2 for magic number 0 at index 0 and \0 at the end of the string
      if (fromSerial.length() + 2 >= EEPROM.length()) {
        Serial.println("String too long for EEPROM!");
        return;
      }

      char * err_msg;
      char * charString = fromSerial.c_str();
      StringReader reader{charString};
      if (itin.parse_input(reader, &err_msg) != r_ok) {
        Serial.println(err_msg);
      }
      else {
        // Set magic number
        EEPROM.update(0,0);
        int eeprom_len = EEPROM.length();
        int i = 1; 
        for (; *charString != '\0'; charString++, i++) {
          EEPROM.update(i, *charString);
        }

        EEPROM.update(i, '\0');
        Serial.println("Got the following choreography:");
        itin.print_to_serial();
      }
    }
  }
  else { 
    if (button.was_2_to_5_pressed()) {
      Serial.println("Ready to receive new choreography.");
      button.reset_memory();
      driver->init_from_itin();
      state = st_parsing;
      return;
    }
    driver->loop();
    
  }
 
  button.reset_memory();
}


// Diode usage
/*if(button.was_2_to_5_pressed() ){
  if(dioda){
    digitalWrite(diode_pin, HIGH);
    }
  else {
    digitalWrite(diode_pin, LOW);
  }
  dioda = !dioda;
}*/
