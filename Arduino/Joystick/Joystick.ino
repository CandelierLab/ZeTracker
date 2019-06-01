// Basic Arduino server

// === Definitions =================================================

int pLR = A1;
int pUD = A2;
int pJBut = A0;

int pWBut = 7;
int pWled = 6;
int pBBut = 8;
int pBled = 9;

int motion_state;
int debounce_delay;

// === Setup =======================================================

void setup() {                

  // --- Pin setup
  pinMode(pLR, INPUT);
  pinMode(pUD, INPUT);
  
  pinMode(pJBut, INPUT_PULLUP);
  pinMode(pWBut, INPUT_PULLUP);
  pinMode(pBBut, INPUT_PULLUP);

  pinMode(pWled, OUTPUT);
  pinMode(pBled, OUTPUT);
  digitalWrite(pWled, LOW);
  digitalWrite(pBled, LOW);

  // --- Serial communication
  Serial.begin(115200);
  Serial.setTimeout(5);

  // --- Initialization
  motion_state = 0;       // See code below
  debounce_delay = 10;    // in ms

  Serial.print("ZeJoystick");

}

// === Main loop ===================================================

void loop() {

  // --- Manage inputs ---------------------------------------------
  
  if (Serial.available()) {   

    String input = Serial.readString();
    input.trim();
    
    // --- Get information
    if (input.equals("id")) {
      
      Serial.print("ZeJoystick");
      
    // --- Image processing status
    } else if (input.substring(0,2).equals("IP")) {
      
      digitalWrite(pWled, input.substring(3).toInt()? HIGH : LOW);

    // --- Running status
    } else if (input.substring(0,3).equals("Run")) {
      
      digitalWrite(pBled, input.substring(4).toInt()? HIGH : LOW);
    
    }
    
  }
  
  // --- Manage outputs --------------------------------------------

  // --- Toggle laser state
  
  if (!digitalRead(pJBut)) {

    Serial.print("Laser");
    delay(debounce_delay);
    while (!digitalRead(pJBut)) { delay(debounce_delay); }
    
  }

  // --- Toggle tracking state
  
  if (!digitalRead(pWBut)) {

    Serial.print("Track");
    delay(debounce_delay);
    while (!digitalRead(pWBut)) { delay(debounce_delay); }
    
  }

  // --- Toggle running state
  
  if (!digitalRead(pBBut)) {

    Serial.print("Run");
    delay(debounce_delay);
    while (!digitalRead(pBBut)) { delay(debounce_delay); }
    
  }

  /* --- Motion code ------------------------
   *  0: OFF
   *  1: L
   *  2: R
   *  3: U
   *  4: D
   *  5: UL
   *  6: UR
   *  7: DL
   *  8: DR
   * ----------------------------------------- */

/*
  Serial.print("LR:");
  Serial.print(analogRead(pLR));
  Serial.print(" / UD:");
  Serial.println(analogRead(pUD));
*/

  bool isLeft = analogRead(pLR)>680;
  bool isRight = analogRead(pLR)<340;

  bool isUp = analogRead(pUD)>680;
  bool isDown = analogRead(pUD)<340;

  // Inverse U/D
  // bool isUp = analogRead(pUD)<340;
  // bool isDown = analogRead(pUD)>680;

  if (isUp) {

    if (isLeft) {
      
      if (motion_state!=5) {
        Serial.print("MOVE_UL");
        delay(debounce_delay);        
        motion_state = 5;
      }
      
    } else if (isRight) {
      
      if (motion_state!=6) {
        Serial.print("MOVE_UR");
        delay(debounce_delay);
        motion_state = 6;
      }
      
    } else if (motion_state!=3) {

      Serial.print("MOVE_U");
      delay(debounce_delay);
      motion_state = 3;
      
    }

  } else if (isDown) {

    if (isLeft) {
      
      if (motion_state!=7) {
        Serial.print("MOVE_DL");        
        delay(debounce_delay);
        motion_state = 7;
      }
      
    } else if (isRight) {
      
      if (motion_state!=8) {
        Serial.print("MOVE_DR");
        delay(debounce_delay);
        motion_state = 8;
      }
      
    } else if (motion_state!=4) {

      Serial.print("MOVE_D");
      delay(debounce_delay);
      motion_state = 4;
      
    }
  
  } else if (isLeft) {

    if (motion_state!=1) {
      Serial.print("MOVE_L");
      delay(debounce_delay);
      motion_state = 1;
    }

  } else if (isRight) {

    if (motion_state!=2) {
      Serial.print("MOVE_R");
      delay(debounce_delay);
      motion_state = 2;
    }
  
  } else {

    if (motion_state) {
      Serial.print("MOVE_OFF");
      delay(debounce_delay);
      motion_state = 0;  
    }
    
  }
  
}
