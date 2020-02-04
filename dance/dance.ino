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
int state = st_stop;
bool dioda = false;
const char* choreo = "A1N c2 t120 d4 t0 b5 t0";
//const char* choreo = "A1N c2 t120";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(1000);
  Serial.println("setup start");
  mov = new Movement();
  sens = new Sensors();
  move_manager = new MoveManager(mov, sens);
 
  StringReader rdr(choreo);
  Serial.println("reader rdy");
  char * err_msg;
  Serial.println("setup before parse");

  Serial.println("read finished");
  if (itin.parse_input(rdr , &err_msg) != r_ok) {
    Serial.println(err_msg);
  }
  Serial.println("setup before driver");
  driver = new Driver(&itin, move_manager, &button);
  Serial.println("exception when init driver baby");

  mov->attach(left_pin, right_pin);
  Serial.println("setup before driver init from itin");
  driver->init_from_itin();

  Serial.println("setup complete");

  pinMode(button_pin, INPUT_PULLUP);
  pinMode(diode_pin, OUTPUT);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
}

int stage = 0;
void loop() {
  // update info from ir sensors
  sens->update();
  // button press makes something happen..
  button.notify_actual_state(!digitalRead(button_pin));
  if (button.was_pressed()) {
    Serial.println("btn pressed");
    stage = 0;
    if (state == st_stop) {
      state = st_drive;
    }
    else {
      state = st_stop;
    }
  }

  if (button.was_long_pressed()) {
    state = st_stop;
    driver->init_from_itin();
  }
  


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
  if (state == st_drive) {
    Serial.print("-in drive-ln\n");
    driver->loop();
  }
  button.reset_memory();
  

  /*if(button.was_long_pressed() ){
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
