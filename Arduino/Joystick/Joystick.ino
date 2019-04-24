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
  debounce_delay = 50;    // in ms

}

// === Main loop ===================================================

void loop() {

  // --- Manage inputs ---------------------------------------------
  
  if (Serial.available()) {   

    String input = Serial.readString();
    input.trim();
    
    // --- Get information
    if (input.equals("id")) {
      
      Serial.println("ZeJoystick");
      
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

    Serial.println("Laser");
    delay(debounce_delay);
    while (!digitalRead(pJBut)) { delay(debounce_delay); }
    
  }

  // --- Toggle tracking state
  
  if (!digitalRead(pWBut)) {

    Serial.println("Track");
    delay(debounce_delay);
    while (!digitalRead(pWBut)) { delay(debounce_delay); }
    
  }

  // --- Toggle running state
  
  if (!digitalRead(pBBut)) {

    Serial.println("Run");
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
  bool isUp = analogRead(pUD)<340;
  bool isDown = analogRead(pUD)>680;

  if (isUp) {

    if (isLeft) {
      
      if (motion_state!=5) {
        Serial.println("Motion UL");        
        motion_state = 5;
      }
      
    } else if (isRight) {
      
      if (motion_state!=6) {
        Serial.println("Motion UR");
        motion_state = 6;
      }
      
    } else if (motion_state!=3) {

      Serial.println("Motion U");
      motion_state = 3;
      
    }

  } else if (isDown) {

    if (isLeft) {
      
      if (motion_state!=7) {
        Serial.println("Motion DL");        
        motion_state = 7;
      }
      
    } else if (isRight) {
      
      if (motion_state!=8) {
        Serial.println("Motion DR");
        motion_state = 8;
      }
      
    } else if (motion_state!=4) {

      Serial.println("Motion D");
      motion_state = 4;
      
    }
  
  } else if (isLeft) {

    if (motion_state!=1) {
      Serial.println("Motion L");
      motion_state = 1;
    }

  } else if (isRight) {

    if (motion_state!=2) {
      Serial.println("Motion R");
      motion_state = 2;
    }
  
  } else {

    if (motion_state) {
      Serial.println("Motion OFF");
      motion_state = 0;  
    }
    
  }
  
}
