#include "Enums.cpp"
#include "Button.cpp"
#include "Sensors.cpp"
#include "Movement.cpp"
#include "Itinerary.cpp"
#include "MoveManager.cpp"

const int left_pin = 12;
const int right_pin = 13;
const int button_pin = 2;
const int diode_pin = 11;

Movement *mov;
Sensors *sens;
Button button;
MoveManager *move_manager;
int state = st_stop;
bool dioda = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  mov = new Movement();
  sens = new Sensors();
  move_manager = new MoveManager(mov, sens);
  mov->attach(left_pin, right_pin);
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
    stage = 0;
    if (state == st_stop) {
      state = st_drive_left;
    }
    else {
      state = st_stop;
    }
  }
  button.reset_memory();


  // Movement manager usage example - go 2 segment forward, than rotate by one quarter to the left and go another two segments forward
  if (state == st_stop) {
    mov->stop();
    return;
  }

  if(stage == 0)
  {
    if(move_manager->move_forward(2))
      stage = 1;
  }
  else if(stage == 1)
  {
    if(move_manager->rotate_left(1))
      stage = 2;
  }
  else if(stage == 2)
  {
    if(move_manager->move_forward(2))
      stage = 3;
  }
  else
    mov->stop();

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
