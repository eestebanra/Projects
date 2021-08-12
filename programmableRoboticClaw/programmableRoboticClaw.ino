/*  Programmable Robotic Claw
 *  Developed by Esteban Rosero
 *  Electrical Engineering Co-op Student at the University of Alberta
*/
 
#include <Servo.h>
#include <LiquidCrystal.h>
#include <stdlib.h>

// This function returns the angle, in degrees, at which we wish to move a servomotor from an Analog input.
int servoPos(const int servoPotPin) {
  int servoPotVal = analogRead(servoPotPin);
  int servoPos = map(servoPotVal, 0, 1023, 0, 180);
  return servoPos;
}

// Even though this function has many parameters, its main purpose is to move a servomotor to a any input location slowly.
// This is done with a for loop which delays its iterations every single time producing a one step at a time effect.
// To control the speed at which we want a servomotor to move, we need to change to value of the "servoSpeed" variable.
void writeSlowly(const int newPos, const int oldPos, const int columnLCD, const int rowLCD, Servo writeTo, LiquidCrystal lcd) {
  const int servoSpeed = 10;
  int i;
  
  if (newPos > oldPos) {
    for (i = oldPos; i <= newPos; i++) {
      lcd.setCursor(columnLCD,rowLCD);
      lcd.print(i);
      lcd.print(" ");
      writeTo.write(i);
      delay(servoSpeed);
    }
  }

  if (newPos < oldPos) {
    for (i = oldPos; i >= newPos ; i--) {
      lcd.setCursor(columnLCD,rowLCD);
      lcd.print(i);
      lcd.print(" ");
      writeTo.write(i);
      delay(servoSpeed);
    }
  }

  if (newPos == oldPos) {
    lcd.setCursor(columnLCD,rowLCD);
    lcd.print(newPos);
    lcd.print(" ");
    writeTo.write(newPos);
    delay(servoSpeed);
  }
}

// Servomotors parameters
const int liftPin = 2;
const int basePin = 3;
const int clawPin = 4;
const int armPin = 5;

int basePos;
int armPos;
int liftPos;
int clawPos;

int armPosOld;
int liftPosOld;
int basePosOld;

// Potentiometers parameters
const int liftPotPin = A0;
const int armPotPin = A1;
const int basePotPin = A2;

// Claw toggle switch and state parameters | CSW stands for Claw Switch.
const int CSWPin = 6;
int CSWValNew;
int CSWValOld = 1;
int clawState = 0;

// LCD display parameters
const int RS = 13;
const int EN = 12;
const int D4 = 11;
const int D5 = 10;
const int D6 = 9;
const int D7 = 8;

// Save position set parameters | SSW and PSW stand for Save Switch and Positions Switch respectively.
const int SSWPin = 7;
int SSWVal;
int PSWVal = 0;

// Wait times
const int welcomeTime = 3000;
const int savingTime = 2000;
const int loadingTime = 200;
const int transitionTime = 250;
const int messageTime = 1300;

// Read or erase memory parameters
const int potPin = A3;
int potVal; 

int loadingColumn;
int readingIndex;
int savingIndex;

int memoryPos;

int positionSets;

int savingCounter;

int maxData;

int *storageArray;

// Initialize servomotors
Servo claw;
Servo base;
Servo arm;
Servo lift;

// Initialize LCD scren
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

void setup() {
  Serial.begin(9600);
  
  claw.attach(clawPin);
  base.attach(basePin);
  arm.attach(armPin);
  lift.attach(liftPin);

  lcd.begin(16,2);
  
  pinMode(basePotPin, INPUT);
  pinMode(armPotPin, INPUT);
  pinMode(liftPotPin, INPUT);
  
  pinMode(potPin, INPUT);
  pinMode(CSWPin, INPUT_PULLUP);
  pinMode(SSWPin, INPUT_PULLUP);

  // Initiate servomotors
  lift.write(servoPos(liftPotPin));
  base.write(servoPos(basePotPin));
  arm.write(servoPos(armPotPin));
  claw.write(25);

  // Welcome screen
  lcd.setCursor(2,0);
  lcd.print("Programmable");
  lcd.setCursor(2,1);
  lcd.print("Robotic Claw");
  delay(welcomeTime);
  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("How many");
  lcd.setCursor(0,1);
  lcd.print("position sets?");
  delay(messageTime);
  
  lcd.setCursor(11,0);
  lcd.print("=>");

  PSWVal = digitalRead(SSWPin);
  
  while(PSWVal == 1) {
    lcd.setCursor(14,0);
    positionSets = map(analogRead(A3), 0, 1023, 20, 1);
    lcd.print(positionSets);
    lcd.print(" ");
    PSWVal = digitalRead(SSWPin);
    delay(1);
  }
  
  delay(200); // This delay is necessary to avoid reading the push button after the welcome screen.
  lcd.clear();

  // If the potentiometer is at its extremas, we ask the user to slide it to its neutral position (500).
  // This is necessary to avoid accessing unwanted options in the main screen.
  if (analogRead(A3) < 10 || analogRead(A3) > 1013) {
    lcd.setCursor(0,0);
    lcd.print("Slide to neutral");
    lcd.setCursor(0,1);
    lcd.print("pos.(500)");
    while (analogRead(A3) != 500) {
      lcd.setCursor(10,1);
      lcd.print("=>");
      lcd.setCursor(13,1);
      lcd.print(analogRead(A3));
      lcd.print(" ");
    }
    lcd.clear();
  }
  
  // Allocate memory
  savingCounter = positionSets;
  maxData = positionSets*4;
  storageArray = (int*)malloc(maxData*sizeof(int));
  
  // Display default claw position
  lcd.setCursor(9,1);
  lcd.print("Closed");
}

void loop() {
  potVal = analogRead(potPin);
  
  // Read memory
  if (potVal < 10) {    
    lcd.clear();
    lcd.setCursor(1,0);
    lcd.print("Reading memory");
    loadingColumn = 0;
    
    while (potVal < 10) {
      potVal = analogRead(potPin);
      lcd.setCursor(loadingColumn++,1);
      lcd.print(char(255));
      delay(loadingTime);
      
      if (loadingColumn == 16) {
        // Set up main LCD screen
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Base:");
        lcd.setCursor(9,0);
        lcd.print("Arm:");
        lcd.setCursor(0,1);
        lcd.print("Lift:");
        
        armPosOld = armPos;
        lcd.setCursor(13,0);
        lcd.print(armPosOld);
        
        liftPosOld = liftPos;
        lcd.setCursor(5,1);
        lcd.print(liftPosOld);
        
        basePosOld = basePos;
        lcd.setCursor(5,0);
        lcd.print(basePosOld);

        if (clawPos == 25) {
          claw.write(clawPos);
          lcd.setCursor(9,1);
          lcd.print("Closed");
        }
          
        else {
          claw.write(clawPos);
          lcd.setCursor(9,1);
          lcd.print("Opened");
        }
        
        readingIndex = 0;
        
        while (readingIndex < maxData) {
          // Check if memory is empty
          if (savingIndex == 0 && savingCounter == maxData/4) {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Memory is empty");
          delay(messageTime);
          lcd.clear();
          break;
          }
          
          memoryPos = storageArray[readingIndex + 1];
          writeSlowly(memoryPos,armPosOld,13,0,arm,lcd);
          armPosOld = memoryPos;
          
          memoryPos = storageArray[readingIndex + 2];
          writeSlowly(memoryPos,liftPosOld,5,1,lift,lcd);
          liftPosOld = memoryPos;
          
          memoryPos = storageArray[readingIndex];
          writeSlowly(memoryPos,basePosOld,5,0,base,lcd);
          basePosOld = memoryPos;
          
          // Claw position in memory
          memoryPos = storageArray[readingIndex + 3];
          
          if (memoryPos == 25) {
            claw.write(memoryPos);
            lcd.setCursor(9,1);
            lcd.print("Closed");
          }

          else {
            claw.write(memoryPos);
            lcd.setCursor(9,1);
            lcd.print("Opened");
          }

          readingIndex += 4;
          
          delay(transitionTime);
          
          // Return to last Anlog input position set 
          if (readingIndex == maxData) {
            
            writeSlowly(armPos,armPosOld,13,0,arm,lcd);
            writeSlowly(liftPos,liftPosOld,5,1,lift,lcd);
            writeSlowly(basePos,basePosOld,5,0,base,lcd);
          
            if (clawPos == 25) {
              claw.write(clawPos);
              lcd.setCursor(9,1);
              lcd.print("Closed");
            }

            else {
              claw.write(clawPos);
              lcd.setCursor(9,1);
              lcd.print("Opened");
            }
          }
        }
        
        lcd.clear();
        
        loadingColumn = 0;
        readingIndex = 0;
        
        break;
      }
    }
    lcd.clear();
  }

  // Erase memory
  if (potVal > 1013) {
    lcd.clear();
    lcd.setCursor(1,0);
    lcd.print("Erasing memory");
    loadingColumn = 0;
    
    while (potVal > 1013) {
      lcd.setCursor(loadingColumn++,1);
      lcd.print(char(255));
      delay(loadingTime);
      potVal = analogRead(potPin);
      
      if (loadingColumn == 16) {
        
        // Check if the memory is already empty
        if (savingIndex == 0 && savingCounter == maxData/4) {
          lcd.clear();
          lcd.setCursor(3,0);
          lcd.print("Memory is");
          lcd.setCursor(2,1);
          lcd.print("already empty");
          delay(messageTime);
          lcd.clear();
      }
      
      else {
        // Erase memory
        memset(storageArray,0,maxData*sizeof(*storageArray));
        free(storageArray);
        savingIndex = 0;
        savingCounter = maxData/4;
               
        lcd.clear();
        lcd.setCursor(2,0);
        lcd.print("Memory erased");
        
        while (potVal > 1013) {
          lcd.setCursor(3,1);
          lcd.print("Slide right");
          potVal = analogRead(potPin);
        }
      }
        lcd.clear();
        loadingColumn = 0;
        break;
      }
    }
    lcd.clear();
  }
  
  basePos = servoPos(basePotPin);
  armPos = servoPos(armPotPin);
  liftPos = servoPos(liftPotPin);
  
  CSWValNew = digitalRead(CSWPin);
  SSWVal = digitalRead(SSWPin);

  // Print to the serial monitor. This is just a way to check if the values showed in the LCD display are accurate.
  Serial.print(basePos);
  Serial.print(" ");
  Serial.print(armPos);
  Serial.print(" ");
  Serial.print(liftPos);
  Serial.print(" ");
  Serial.print(clawState);
  Serial.print(" ");
  Serial.println(SSWVal);

  // Print to LCD display.
  lcd.setCursor(0,0);
  lcd.print("Base:");
  lcd.print(basePos);
  lcd.print(" ");
  lcd.setCursor(9,0);
  lcd.print("Arm:");
  lcd.print(armPos);
  lcd.print(" ");
  lcd.setCursor(0,1);
  lcd.print("Lift:");
  lcd.print(liftPos);
  lcd.print(" ");

  // Write to servomotors.
  base.write(basePos);
  arm.write(armPos);
  lift.write(liftPos);

  if (clawState == 1) {
    clawPos = 65;
    claw.write(clawPos);
    lcd.setCursor(9,1);
    lcd.print("Opened");
  }
  
  else {
    clawPos = 25;
    claw.write(clawPos);
    lcd.setCursor(9,1);
    lcd.print("Closed");
  }

  // Claw toggle switch and state
  if (CSWValOld == 0 && CSWValNew == 1) {
    if (clawState == 0) {
      clawState = 1;
    }
    else {
      clawState = 0;
    }
  }
  CSWValOld = CSWValNew;

  // Store position set
  if (SSWVal == 0) {
    
    // Check if the memory is full
    if (savingIndex == maxData && savingCounter == 0) {
      lcd.clear();
      lcd.setCursor(2,0);
      lcd.print("The memory is");
      lcd.setCursor(6,1);
      lcd.print("full");
      delay(savingTime);
      lcd.clear();
    }
    
    else {
      
      storageArray[savingIndex] = basePos;
      storageArray[savingIndex + 1] = armPos;
      storageArray[savingIndex + 2] = liftPos;
      storageArray[savingIndex + 3] = clawPos;
      
      savingIndex += 4;
    
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("The position set");
      lcd.setCursor(0,1);
      lcd.print("has been stored");
      
      delay(messageTime);
      
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(--savingCounter);

      // Change from "sets" to set
      if (savingCounter < 2) {
        lcd.print(" position set");
      }
      
      else {
        lcd.print(" position sets");
      }
      
      lcd.setCursor(6,1);
      lcd.print("left");
      delay(messageTime);
      lcd.clear(); 
    }
  }

  delay(1);
}
