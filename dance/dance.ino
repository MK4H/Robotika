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
    Serial.println("EEPROM choreo not found - using default");
    StringReader def_reader(choreo);
    if (itin.parse_input(def_reader, &err_msg) != r_ok) {
      Serial.println(err_msg);
      Serial.println("FATAL ERROR, DEFAULT CHOREOGRAPHY DOES NOT WORK");
    }
  }
  else{
    Serial.println("EEPROM choreo loaded");
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("setup start");
  
  mov = new Movement();
  sens = new Sensors();
  move_manager = new MoveManager(mov, sens);

  init_pins();

  init_choreography();

  driver = new Driver(&itin, move_manager, &button);
  driver->init_from_itin();
  
  Serial.println("setup complete");
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
        Serial.println("over 5 pressed");
        for (int i = 0 ; i < EEPROM.length() ; i++) {
          EEPROM.update(i, 255);
        }
      }
      else {
        // Start driving
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
      }
    }
  }
  else { 
    if (button.was_2_to_5_pressed()) {
      Serial.println("2 to 5 pressed");
      button.reset_memory();
      state = st_parsing;
      return;
    }
    driver->loop();
    
  }
 
  button.reset_memory();
  // Movement manager usage example - go 2 segment forward, than rotate by one quarter to the left and go another two segments forward
  // if (state == st_stop) {
  //   mov->stop();
  //   return;
  // }

  // if(stage == 0)
  // {
  //   if(move_manager->move_forward(2))
  //     stage = 1;
  // }
  // else if(stage == 1)
  // {
  //   if(move_manager->rotate_left(1))
  //     stage = 2;
  // }
  // else if(stage == 2)
  // {
  //   if(move_manager->move_forward(2))
  //     stage = 3;
  // }
  // else
  //   mov->stop();

  // Moje myslenka je naparsovat to do itinerary a cekat na button press,
  // pri button pressu pak nastavit state na st_drive a jedeme
  
  

  //WHEN YOU ARE READY TO HAND OFF CONTROL TO THE DRIVER, JUST START CALLING THE LOOP
  // The itinerary has to be already parsed and filled, from wherever
  // BUTTON RESET MEMORY MUST BE CALLED AFTER THE driver->loop() call
;
  

  /*if(button.was_2_to_5_pressed() ){
    if(dioda){
      digitalWrite(diode_pin, HIGH);
      }
    else {
      digitalWrite(diode_pin, LOW);
    }
    dioda = !dioda;
  }
  

  // cleanup
  
  // movement disabled
  if (state == st_stop) {
    mov->stop();
    return;
  }

  // main algorithm
 
  switch(state) {
    case st_stop:
      mov->stop();
      break;
  }*/
}
