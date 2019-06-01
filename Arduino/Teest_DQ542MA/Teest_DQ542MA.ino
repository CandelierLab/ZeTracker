// Basic Arduino server

// === Definitions =================================================

int pDIR = 2;
int pPULSE = 3;
int pENABLE = 7;

long int period = 2000;
boolean bRun = false;

// === Setup =======================================================

void setup() {                

  // --- Pin setup
  pinMode(pDIR, OUTPUT);
  pinMode(pPULSE, OUTPUT);
  pinMode(pENABLE, OUTPUT);
  
  digitalWrite(pDIR, HIGH);
  digitalWrite(pPULSE, LOW);
  digitalWrite(pENABLE, LOW);

  // --- Serial communication
  Serial.begin(115200);
  Serial.setTimeout(5);
  
}

// === Main loop ===================================================

void loop() {

  // --- Manage inputs ---------------------------------------------
  
  if (Serial.available()) {   

    String input = Serial.readString();
    input.trim();
    
    // --- Get information
    if (input.equals("info")) {
      
      Serial.println("----------------------------");
      Serial.println("Basic serial server");
      Serial.println("Period: " + String(period) + " us");
      if (bRun) { Serial.println("Running: on"); } else { Serial.println("Running: off"); }
      Serial.println("----------------------------");
      
    // --- Start
    } else if (input.equals("start")) {
      
      bRun = true;
      Serial.println("Running");
     
    // --- Stop
    } else if (input.equals("stop")) {
      
      bRun = false;
      Serial.println("Stopped");

    // --- Left
    } else if (input.equals("left")) {
      
      digitalWrite(pDIR, HIGH);
      Serial.println("Going left");
     

    // --- Right
    } else if (input.equals("right")) {
      
      digitalWrite(pDIR, LOW);
      Serial.println("Going right");
     
    
    // --- Set period
    } else if (input.substring(0,6).equals("period")) {
      
      period = input.substring(7).toInt();
      Serial.println("Period: " + String(period) + " us");
      
    }
  }
  
  // --- Actions ---------------------------------------------------
  
  if (bRun) {

    // digitalWrite(pENABLE, HIGH); 
  
    digitalWrite(pPULSE, HIGH); 
    delayMicroseconds(period/2);
    
    digitalWrite(pPULSE, LOW);
    delayMicroseconds(period/2);
    /**/  
  } else {

    // digitalWrite(pENABLE, LOW); 
    digitalWrite(pPULSE, LOW); 

  }
  
}
