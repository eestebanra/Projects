/*  Boeing 737NG MCP
 *  Developed by Esteban Rosero
 *  Electrical Engineering Co-op Student at the University of Alberta
*/
#include <LCD.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Bounce2.h>

LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7);

Bounce pushDetect = Bounce();
Bounce pushRE = Bounce();
Bounce switchCOM = Bounce();
Bounce switchNAV = Bounce();
Bounce switchQNHmode = Bounce();
Bounce switchAutopilot = Bounce();
Bounce switchAutothrottle = Bounce();
Bounce switchFlightDirector = Bounce();
Bounce switchLandingGear = Bounce();
Bounce switchLandingLights = Bounce();

// Shift registers parameters
#define dataSRPin 10
#define latchPin 9
#define clockSRPin 8
#define pushDetectPin 7

int i;
byte Check;
byte CheckSecond;
byte Output = 0x00;
byte OutputSecond = 0xf0;

// Rotary Encoder parameters
#define pushREPin 13
#define dataREPin 12
#define clockREPin 11

// Rotary Encoder previous pins states
int previousData;
int previousClock;

// Encoder Debouncer
long timeOfLastDebounce = 0;
int delayOfDebounce = 0.01;

// MCP variables
// Course
int CRS = 0;

// Indicated airspeed
int IAS = 0;
int IASflag = 0;
int IASmachFlag = 0;
double IASmach = double(IAS) * 0.00149984;
int VNAVflag = 0;

// Heading
int HDG = 0;

// Altitude
long int ALT = 0;

// Vertical velocity
int VSV = 200;

// QNH
double QNH = 29.90;
int QNHmodeFlag = 1;
int QNHstandardFlag = 0;

// Bank angle
int bankAngle = 20;

// COM frequency
long int FQintCOM = 119;
int FQfloCOM = 700;

// NAV frequency
long int FQintNAV= 113;
int FQfloNAV = 35;

// MCP modes
int identifier = 0;
int previousIdentifier;

// Switches parameters
#define switchCOMPin 6
int switchNAVPin = 5;

#define switchQNHmodePin 4
int switchQNHmodeState = LOW;

#define switchAutopilotPin 3
int switchAutopilotState = LOW;

#define switchAutothrottlePin 2
int switchAutothrottleState = LOW;

#define switchFlightDirectorPin A0
int switchFlightDirectorState = LOW;

#define switchLandingGearPin A1
int switchLandingGearState = HIGH;

#define switchLandingLightsPin A2
int switchLandingLightsState = LOW;


void checkRE(int identifierMode) {
  if ((millis() - timeOfLastDebounce) > delayOfDebounce) {
    checkREdir(identifierMode);
    previousClock = digitalRead(clockREPin);
    previousData = digitalRead(dataREPin);
    timeOfLastDebounce = millis();
  }
}

void checkREdir(int identifierMode) {
  // Clockwise
  if ((previousClock == 1) && (previousData == 0)) {
    if ((digitalRead(clockREPin) == 0) && (digitalRead(dataREPin) == 0)) {
      clockwiseMode(identifierMode);
    }
  }
  // Counterclockwise
  if ((previousClock == 1) && (previousData == 1)) {
    if ((digitalRead(clockREPin) == 0) && (digitalRead(dataREPin))) {
      counterclockwiseMode(identifierMode);
    }
  }
}

void clockwiseMode(int mode) {
  switch (mode) {
    case 0:
    if ((CRS >= 0) && (CRS <= 360)) {
      CRS++;
      if (CRS == 361) {
        CRS = 360;
      }
    }
    Serial.print("E0");
    Serial.println(CRS);
    lcd.setCursor(17,0);
    lcd.print("   ");
    lcd.setCursor(17,0);
    lcd.print(CRS);
    break;
    
    case 1: // IAS
    if (IASflag == 0) {
      if ((IAS >= 0) && (IAS <= 392)) {
        IAS++;
        if (IAS == 393) {
          IAS = 392; 
        }
      }
    }
    else {
      if ((IAS >= 114) && (IAS <= 392)) {
        IAS++;
        if (IAS == 393) {
          IAS = 392;
        }
      }
    }
    lcd.setCursor(5,1);
    lcd.print("    ");
    lcd.setCursor(5,1);
    if (VNAVflag == 0) {
      if (IASmachFlag == 0) {
        Serial.print("E1");
        Serial.println(IAS);
        lcd.print(IAS);
      }
      else {
        IASmach = double(IAS) * 0.00149984;
        Serial.print("E2");
        Serial.println(IASmach);
        lcd.print(IASmach);
      }
    }
    else {
      Serial.print("E1");
      Serial.println(IAS);
    }
    break;
    
    case 2: // HDG
    if ((HDG >= 0) && (HDG <= 360)) {
      HDG++;
      if (HDG == 361) {
        HDG = 0;
        HDG++;
      }
    }
    Serial.print("E3");
    Serial.println(HDG);
    lcd.setCursor(5,2);
    lcd.print("   ");
    lcd.setCursor(5,2);
    lcd.print(HDG);
    break;

    case 3:
    if ((ALT >= -1000) && (ALT <= 99900)) {
      ALT += 100;
      if (ALT == 100000) {
        ALT = 99900;
      }
    }
    Serial.print("E4");
    Serial.println(ALT);
    lcd.setCursor(15,1);
    lcd.print("     ");
    lcd.setCursor(15,1);
    lcd.print(ALT);
    break;
    
    case 4: // V/S
    if ((VSV >= -10000) && (VSV <= 10000)) {
      VSV += 100;
      if (VSV == 5100) {
        VSV = 5000;
      }
    }
    Serial.print("E5");
    Serial.println(VSV);
    lcd.setCursor(15,2);
    lcd.print("     ");
    if (VSV == 0);
    else {
      lcd.setCursor(15,2);
      lcd.print(VSV);  
    }
    break;

    case 5: // QNH
    if ((QNH >= 0.00) && (QNH <= 40.00)) {
      QNH += 0.01;
      if ((QNH + 1) >= 40.5) {
        QNH = 40.00;
      }
    }
    Serial.print("E6");
    Serial.println(QNH);
    QNHstandardFlag = 0;
    lcd.setCursor(6,0);
    lcd.print("     ");
    lcd.setCursor(6,0);
    if (QNH == 29.92) {
      lcd.print("STD");
    }
    else {
      if (QNHmodeFlag == 1) {
        lcd.print(int(round((QNH * (33.863886666667)))));
      }
      else {
        lcd.print(QNH);
      }
    }
    
    break;

    case 6: // Bank Angle
    if ((bankAngle >= 10) && (bankAngle <= 30)) {
      bankAngle += 5;
      if (bankAngle == 35) {
        bankAngle = 30;
      }
    }
    Serial.println("B12");
    lcd.setCursor(9,3);
    lcd.print("   ");
    lcd.setCursor(9,3);
    lcd.print(bankAngle);
    break;
    
    case 7: // Frequency COM integer
    if ((FQintCOM >= 118) && (FQintCOM<= 136)) {
      FQintCOM++;
      if (FQintCOM == 137) {
        FQintCOM = 118;
      }
    }
    lcd.setCursor(0,3);
    lcd.print("   ");
    lcd.setCursor(0,3);    
    lcd.print(FQintCOM);
    break;

    case 8: // Frequency COM float
    if ((FQfloCOM >= 0) && (FQfloCOM <= 995)) {
      FQfloCOM += 5;
      if (FQfloCOM == 1000) {
        FQfloCOM = 0;
      }
    }
    lcd.setCursor(4,3);
    lcd.print("   ");
    lcd.setCursor(4,3);
    lcd.print(FQfloCOM);
    break;
    
    case 9: // Frequency NAV integer
    if ((FQintNAV >= 108) && (FQintNAV <= 117)) {
      FQintNAV++ ;
      if (FQintNAV == 118) {
        FQintNAV = 108;
      }
    }
    lcd.setCursor(13,3);
    lcd.print("   ");
    lcd.setCursor(13,3);    
    lcd.print(FQintNAV);
    break;

    case 10: // Frequency NAV float
    if ((FQfloNAV >= 0) && (FQfloNAV <= 95)) {
      FQfloNAV += 5;
      if (FQfloNAV == 100) {
        FQfloNAV = 0;
      }
    }
    lcd.setCursor(17,3);
    lcd.print("   ");
    lcd.setCursor(17,3);
    lcd.print(FQfloNAV);
    break;
  }
}

void counterclockwiseMode(int Mode) {
  switch (Mode) {
    case 0: // CRS
    if ((CRS >= 0) && (CRS <= 360)) {
      CRS--;
      if (CRS == -1) {
        CRS = 0;
      }
    }
    Serial.print("E0");
    Serial.println(CRS);
    lcd.setCursor(17,0);
    lcd.print("   ");
    lcd.setCursor(17,0);
    lcd.print(CRS); 
    break;
    
    case 1: // IAS
    if (IASflag == 0) {
      if ((IAS >= 0) && (IAS <= 392)) {
        IAS--;
        if (IAS == -1) {
          IAS = 0; 
        }
      }
    }
    else {
      if ((IAS >= 114) && (IAS <= 392)) {
        IAS--;
        if (IAS == 113) {
          IAS = 114;
        }
      }
    }
    lcd.setCursor(5,1);
    lcd.print("    ");
    lcd.setCursor(5,1);
    if (VNAVflag == 0) {
      if (IASmachFlag == 0) {
        Serial.print("E1");
        Serial.println(IAS);
        lcd.print(IAS);
      }
      else {
        IASmach = double(IAS) * 0.00149984;
        Serial.print("E2");
        Serial.println(IASmach);
        lcd.print(IASmach);
      }
    }
    else {
      Serial.print("E1");
      Serial.println(IAS);
    }
    break;

    case 2: // HDG
    if ((HDG >= 0) && (HDG <= 360)) {
      HDG--;
      if (HDG == -1) {
        HDG = 360;
        HDG--;
      }
    }
    Serial.print("E3");
    Serial.println(HDG);
    lcd.setCursor(5,2);
    lcd.print("   ");
    lcd.setCursor(5,2);
    lcd.print(HDG);
    break;

    case 3: // ALT
    if ((ALT >= -1000) && (ALT <= 99900)) {
      ALT -= 100;
      if (ALT == -100) {
        ALT = 0;
      }
    }
    Serial.print("E4");
    Serial.println(ALT);
    lcd.setCursor(15,1);
    lcd.print("     ");
    lcd.setCursor(15,1);
    lcd.print(ALT);
    break;
    
    case 4: // V/S
    if ((VSV >= -10000) && (VSV <= 10000)) {
      VSV -= 100;
      if (VSV == -5100) {
        VSV = -5000;
      }
    }
    Serial.print("E5");
    Serial.println(VSV);
    lcd.setCursor(15,2);
    lcd.print("     ");
    if (VSV == 0);
    else {
      lcd.setCursor(15,2);
      lcd.print(VSV);
    }
    break;

    case 5: // QNH
    if ((QNH >= 0.00) && (QNH <= 40.00)) {
      QNH -= 0.01;
      if ((QNH + 1) <= 1) {
        QNH = 0;
      }
    }
    Serial.print("E6");
    Serial.println(QNH);
    QNHstandardFlag = 0;
    lcd.setCursor(6,0);
    lcd.print("     ");
    lcd.setCursor(6,0);
    if (QNH == 29.92) {
      lcd.print("STD");
    }
    else {
      if (QNHmodeFlag == 1) {
        lcd.print(int(round((QNH * (33.863886666667)))));
      }
      else {
        lcd.print(QNH);
      }
    }
    break;

    case 6: // Banking Angle
    if ((bankAngle >= 10) && (bankAngle <= 30)) {
      bankAngle -= 5;
      if (bankAngle == 5) {
        bankAngle = 10;
      }
    }
    Serial.println("B13");
    lcd.setCursor(9,3);
    lcd.print("   ");
    lcd.setCursor(9,3);
    lcd.print(bankAngle);
    break;
    
    
    case 7: // Frequency COM integer
    if ((FQintCOM >= 118) && (FQintCOM <= 136)) {
      FQintCOM--;
      if (FQintCOM == 117) {
        FQintCOM = 136;
      }
    }
    lcd.setCursor(0,3);
    lcd.print("   ");
    lcd.setCursor(0,3);
    lcd.print(FQintCOM);
    break;

    case 8: // Frequency COM float
    if ((FQfloCOM >= 0) && (FQfloCOM <= 995)) {
      FQfloCOM -= 5;
      if (FQfloCOM == -5) {
        FQfloCOM = 995;
      }
    }
    lcd.setCursor(4,3);
    lcd.print("   ");
    lcd.setCursor(4,3);
    lcd.print(FQfloCOM);
    break;

    case 9: // Frequency NAV integer
    if ((FQintNAV >= 108) && (FQintNAV <= 117)) {
      FQintNAV--;
      if (FQintNAV == 107) {
        FQintNAV = 117;
      }
    }
    lcd.setCursor(13,3);
    lcd.print("   ");
    lcd.setCursor(13,3);
    lcd.print(FQintNAV);
    break;

    case 10: // Frequency NAV float
    if ((FQfloNAV >= 0) && (FQfloNAV <= 95)) {
      FQfloNAV -= 5;
      if (FQfloNAV == -5) {
        FQfloNAV = 95;
      }
    }
    lcd.setCursor(17,3);
    lcd.print("   ");
    lcd.setCursor(17,3);
    lcd.print(FQfloNAV);
    break;
  }
}

void displayMode(int selectedMode) {
  switch (selectedMode) {
    case 0:
    lcd.setCursor(2,0);
    lcd.print("CRS");
    break;

    case 1:
    lcd.setCursor(2,0);
    lcd.print("IAS");
    break;

    case 2:
    lcd.setCursor(2,0);
    lcd.print("HDG");
    break;
    
    case 3:
    lcd.setCursor(2,0);
    lcd.print("ALT");
    break;

    case 4:
    lcd.setCursor(2,0);
    lcd.print("V/S");
    break;

    case 5:
    lcd.setCursor(2,0);
    lcd.print("QNH");
    break;

    case 6:
    lcd.setCursor(2,0);
    lcd.print("BAn");
    break;
  }
}

void latch() {
  digitalWrite(latchPin, LOW);
  digitalWrite(latchPin, HIGH);
}

void checkSR() {
  Check = 0x10;
  CheckSecond = 0x01;
  for (i = 0; i < 12; i++) {
    
    if (i < 4) {
      shiftOut(dataSRPin, clockSRPin, MSBFIRST, 0x00);
      shiftOut(dataSRPin, clockSRPin, MSBFIRST, Check);
      shiftOut(dataSRPin, clockSRPin, MSBFIRST, 0x00);
      latch();
      if (digitalRead(pushDetectPin) == HIGH) {
        pushButton(i);
      }
      Check = Check << 1;
    }

    if (i >= 4) {
      shiftOut(dataSRPin, clockSRPin, MSBFIRST, CheckSecond);
      shiftOut(dataSRPin, clockSRPin, MSBFIRST, 0x00);
      shiftOut(dataSRPin, clockSRPin, MSBFIRST, 0x00);
      latch();
      if (digitalRead(pushDetectPin) == HIGH) {
        pushButton(i);
      }
      CheckSecond = CheckSecond << 1;
    }
    shiftOut(dataSRPin, clockSRPin, MSBFIRST, 0xff);
    shiftOut(dataSRPin, clockSRPin, MSBFIRST, OutputSecond);
    shiftOut(dataSRPin, clockSRPin, MSBFIRST, Output);
    latch();
  }
}

void pushToLED (int indexLED) {
  indexLED += 1;
  
  if (indexLED == 1 || indexLED == 12) {
    return;
  }
  
  if (indexLED < 5) {
    if ((bitRead(Output, indexLED) == 1)) {
      bitWrite(Output, indexLED, 0);
    }
    else {
      bitWrite(Output, indexLED, 1);
    }
  }
  if (indexLED >= 5 && indexLED < 8) {
    if ((bitRead(Output, indexLED) == 1)) {
      bitWrite(Output, indexLED, 0);
    }
    else {
      bitWrite(Output, indexLED, 1);
    }
  }

  if (indexLED >= 8) {
    if ((bitRead(OutputSecond, (indexLED - 8)) == 1)) {
      bitWrite(OutputSecond, (indexLED - 8), 0);
    }
    else {
      bitWrite(OutputSecond, (indexLED - 8), 1);
    }
  }
}

void pushButton (int indexPushButton) {
  if (i != 1 && i != 2 && i != 11) {
    IASflag = 1;
    if (IAS < 114) {
      IAS = 114;
      lcd.setCursor(5,1);
      lcd.print(IAS);
    }
  }
  switch (indexPushButton) {
    case 0:
    Serial.println("B0");
    if (IASmachFlag == 0) {
      IASmach = double(IAS) * 0.00149984;
      Serial.print("E2");
      Serial.println(IASmach);
      IASmachFlag = 1;
      lcd.setCursor(5,1);
      lcd.print("     ");
      lcd.setCursor(5,1);
      lcd.print(IASmach);
    }
    else {
      Serial.print("E2");
      Serial.println(IAS);
      IASmachFlag = 0;
      lcd.setCursor(5,1);
      lcd.print("     ");
      lcd.setCursor(5,1);
      lcd.print(IAS);
    }
    break;

    case 1: // N1
    if (switchAutothrottleState == HIGH) {
      Serial.println("B1");
      if (!((bitRead(OutputSecond, 2) == 1 || bitRead(OutputSecond, 3) == 1) && (bitRead(Output, 3) == 1))) {
        bitWrite(Output, 3, 0); // SPEED
        if (bitRead(OutputSecond, 2) == 1 || bitRead(OutputSecond, 3) == 1) {
          bitWrite(Output, 3, 1);
          shiftOut(dataSRPin, clockSRPin, MSBFIRST, 0xff);
          shiftOut(dataSRPin, clockSRPin, MSBFIRST, OutputSecond);
          shiftOut(dataSRPin, clockSRPin, MSBFIRST, Output);
          latch();
        }
        else {
          pushToLED(indexPushButton); // N1  
        }
      }
    }
    break;

    case 2: // SPEED
    if (switchAutothrottleState == HIGH) {
      Serial.println("B2");
      bitWrite(Output, 2, 0); // N1
      bitWrite(Output, 5, 0); // LVL CHG
      pushToLED(indexPushButton);
    }
    break;

    case 3: // VNAV
    Serial.println("B3");
    pushToLED(indexPushButton);
    if (VNAVflag == 0) {
      VNAVflag = 1;
      lcd.setCursor(5,1);
      lcd.print("    ");
    }
    else {
      VNAVflag = 0;
      lcd.setCursor(5,1);
      if (IASmachFlag == 1) {
        IASmach = double(IAS) * 0.00149984;
        lcd.print(IASmach);        
      }
      else {
        lcd.print(IAS);
      }
    }
    break;
    
    case 4: // LVL CHG
    Serial.println("B4");
    bitWrite(OutputSecond, 2, 0); // ALT HLD
    bitWrite(OutputSecond, 3, 0); // V/S
    if (bitRead(Output, 3) == 1) {
      bitWrite(Output, 2, 1);
      bitWrite(Output, 3, 0);
    }
    pushToLED(indexPushButton);
    break;

    case 5: // HDG SEL
    Serial.println("B5");
    pushToLED(indexPushButton);
    break;

    case 6: // APP
    Serial.println("B6");
    bitWrite(OutputSecond, 0, 0); // VOR LOC
    pushToLED(indexPushButton);
    break;

    case 7: // VOR LOC
    Serial.println("B7");
    if (bitRead(Output, 7) == 1) {
      bitWrite(Output, 7, 0);
    }
    else {
      pushToLED(indexPushButton);
    }
    break;

    case 8: // LNAV
    Serial.println("B8");
    break;

    case 9: // ALT HLD
    Serial.println("B9");
    bitWrite(Output, 5, 0); // LVL CHG
    bitWrite(OutputSecond, 3, 0); // V/S 
    if (bitRead(Output, 2) == 1) {
      bitWrite(Output, 2, 0); // N1
      bitWrite(Output, 3, 1); // SPEED
    }
    pushToLED(indexPushButton);
    break;

    case 10: // V/S
    Serial.println("B10");
    bitWrite(Output, 5, 0); // LVL CHG
    bitWrite(OutputSecond, 2, 0); // ALT HLD
    if (bitRead(Output, 2) == 1) {
      bitWrite(Output, 2, 0); // N1
      bitWrite(Output, 3, 1); // SPEED
    }
    pushToLED(indexPushButton);
    break;
    
    case 11: // QNH
    Serial.println("B11");
    // Set QNH to standard mode
    lcd.setCursor(6, 0);
    lcd.print("     ");
    lcd.setCursor(6,0);
    lcd.print("STD");
    QNHstandardFlag = 1;
    QNH = 29.92;
    break;
  }
}

void setup() {
  Serial.begin(115200);

  // Set up shift registers
  pinMode(clockSRPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(dataSRPin, OUTPUT);
  pinMode(pushDetectPin, INPUT);
  pushDetect.attach(pushDetectPin);

  // Setup
  shiftOut(dataSRPin, clockSRPin, MSBFIRST, 0xff);
  shiftOut(dataSRPin, clockSRPin, MSBFIRST, 0xf0);
  shiftOut(dataSRPin, clockSRPin, MSBFIRST, 0x00);
  latch();
  
  // Read current pins states
  previousData = digitalRead(dataREPin);
  previousClock = digitalRead(clockREPin);

  // Set up push button (Rotary Encoder)
  pinMode(pushREPin, INPUT_PULLUP);
  pushRE.attach(pushREPin);

  // Set up switches
  pinMode(switchCOMPin, INPUT_PULLUP);
  switchCOM.attach(switchCOMPin);

  pinMode(switchNAVPin, INPUT_PULLUP);
  switchNAV.attach(switchNAVPin);

  pinMode(switchQNHmodePin, INPUT_PULLUP);
  switchQNHmode.attach(switchQNHmodePin);

  pinMode(switchAutopilotPin, INPUT_PULLUP);
  switchAutopilot.attach(switchAutopilotPin);

  pinMode(switchFlightDirectorPin, INPUT_PULLUP);
  switchFlightDirector.attach(switchFlightDirectorPin);

  pinMode(switchAutothrottlePin, INPUT_PULLUP);
  switchAutothrottle.attach(switchAutothrottlePin);

  pinMode(switchLandingGearPin, INPUT_PULLUP);
  switchLandingGear.attach(switchLandingGearPin);

  pinMode(switchLandingLightsPin, INPUT_PULLUP);
  switchLandingLights.attach(switchLandingLightsPin);
  
  // Set up LCD display
  lcd.begin(20,4); 
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);

  lcd.setCursor(0,0);
  lcd.print("=>");
  
  displayMode(identifier);

  // QNH
  Serial.print("E6");
  Serial.println(QNH);
  lcd.setCursor(6,0);
  lcd.print(QNH);

  lcd.setCursor(12,0);
  lcd.print("CRS:");
  Serial.print("E0");
  Serial.println(CRS);
  lcd.setCursor(17, 0);
  lcd.print(CRS);
  
  lcd.setCursor(0,1);
  lcd.print("IAS:");
  Serial.print("E1");
  Serial.println(IAS);
  lcd.setCursor(5, 1);
  lcd.print(IAS);
    
  lcd.setCursor(0,2);
  lcd.print("HDG:");
  Serial.print("E3");
  Serial.println(HDG);
  lcd.setCursor(5, 2);
  lcd.print(HDG);

  lcd.setCursor(10,1);
  lcd.print("ALT:");
  Serial.print("E4");
  Serial.println(ALT);
  lcd.setCursor(15,1);
  lcd.print(ALT);

  lcd.setCursor(10,2);
  lcd.print("V/S:");
  Serial.print("E5");
  Serial.println(VSV);
  lcd.setCursor(15,2);
  lcd.print(VSV);

  // COM
  Serial.print("E7");
  Serial.println((FQintCOM*1000) + FQfloCOM);
  lcd.setCursor(3,3);
  lcd.print(".");
  lcd.setCursor(0,3);
  lcd.print(FQintCOM);
  lcd.setCursor(4,3);
  lcd.print(FQfloCOM);

  // NAV
  Serial.print("E8");
  Serial.println((FQintNAV*100) + FQfloNAV);
  lcd.setCursor(13,3);
  lcd.print(FQintNAV);
  lcd.setCursor(16,3);
  lcd.print(".");
  lcd.setCursor(17,3);
  lcd.print(FQfloNAV);

  // Bank Angle
  lcd.setCursor(9,3);
  lcd.print(bankAngle);
}

void loop() {  
  // Check if the Rotary Encoder has been slid
  checkRE(identifier);
  
  // Check if any push button has been pressed
  if (pushDetect.update() && pushDetect.read() == HIGH) {
    checkSR();
  }
  
  // Check if the Rotary Encoder push button has been pressed and set the corresponding mode
  if (pushRE.update() && pushRE.read() == LOW) {
    identifier++;
    displayMode(identifier);
    if (identifier == 7) {
      identifier = 0;
    }
    lcd.setCursor(2,0);
    lcd.print("    ");
    displayMode(identifier);
  }
  
  // Switch to COM mode
  if (switchCOM.update() && switchCOM.read() == LOW) {
    lcd.setCursor(2,0);
    lcd.print("   ");
    lcd.setCursor(2,0);
    lcd.print("COM");
    previousIdentifier = identifier;
    identifier = 7;
    while (digitalRead(switchCOMPin) == LOW) {
      checkRE(identifier);
      if (pushRE.update() && pushRE.read() == LOW) {
        identifier++;
        if (identifier == 9) {
          identifier = 7;
        }
      }
    }
    Serial.print("E7");
    Serial.println((FQintCOM*1000) + FQfloCOM);
    identifier = previousIdentifier;
    lcd.setCursor(2,0);
    lcd.print("    ");
    displayMode(identifier);
  }
  
  // Switch to NAV mode
  if (switchNAV.update() && switchNAV.read() == LOW) {
    lcd.setCursor(2,0);
    lcd.print("   ");
    lcd.setCursor(2,0);
    lcd.print("NAV");
    previousIdentifier = identifier;
    identifier = 9;
    while (digitalRead(switchNAVPin) == LOW) {
      checkRE(identifier);
      if (pushRE.update() && pushRE.read() == LOW) {
        identifier++;
        if (identifier == 11) {
          identifier = 9;
        }
      }
    }
    Serial.print("E8");
    Serial.println((FQintNAV*100) + FQfloNAV);
    identifier = previousIdentifier;
    lcd.setCursor(2,0);
    lcd.print("    ");
    displayMode(identifier);
  }

  // Switch between QNH units. Either HPA or inHg.
  if (switchQNHmode.update() && switchQNHmode.read() == switchQNHmodeState) {
    
    if (switchQNHmodeState == HIGH) {
      // HPA
      Serial.println("S1");
      QNHmodeFlag = 0;
      if (QNHstandardFlag == 0) {
        lcd.setCursor(6,0);
        lcd.print("     ");
        lcd.setCursor(6,0);
        if (QNH == 29.92) {
          lcd.print("STD");
        }
        else {
          lcd.print(QNH);          
        }
      }
    }
    if (switchQNHmodeState == LOW) {
      // inHg
      Serial.println("S0");
      QNHmodeFlag = 1;
      if (QNHstandardFlag == 0) {
        lcd.setCursor(6,0);
        lcd.print("     ");
        lcd.setCursor(6,0);
        if (QNH == 29.92) {
          lcd.print("STD");
        }
        else {
          lcd.print(int(round((QNH * 33.863886666667)))); 
        }
      }
    }
    switchQNHmodeState = !switchQNHmodeState;
  }

  // Autopilot
  if (switchAutopilot.update() && switchAutopilot.read() == switchAutopilotState) {
    if (switchAutopilotState == LOW) {
      // Autopilot ON
      Serial.println("S2");
      IASflag = 1;
      if (IAS < 114) {
        IAS = 114;
        lcd.setCursor(5,1);
        lcd.print(IAS);
      }
    }
    
    if (switchAutopilotState == HIGH) {
      // Autopilot OFF
      Serial.println("S3"); 
      IASflag = 0;
      if (switchFlightDirectorState == LOW) {
        bitWrite(Output, 5, 0); // LVLCHANGE
        bitWrite(Output, 6, 0); // HEADING
        bitWrite(Output, 7, 0); // APP
        bitWrite(OutputSecond, 0, 0); // VOR LOC
        bitWrite(OutputSecond, 2, 0); // ALT HLD
        bitWrite(OutputSecond, 3, 0); // V/S
      
        shiftOut(dataSRPin, clockSRPin, MSBFIRST, 0xff);
        shiftOut(dataSRPin, clockSRPin, MSBFIRST, OutputSecond);
        shiftOut(dataSRPin, clockSRPin, MSBFIRST, Output);
        latch(); 
      }
    }
    switchAutopilotState = !switchAutopilotState;
  }

  // Autothrottle
  if (switchAutothrottle.update() && switchAutothrottle.read() == switchAutothrottleState) {
    if (switchAutothrottleState == HIGH) {
      // Autothrottle OFF
      Serial.println("S4");
      bitWrite(Output, 2, 0); // N1
      bitWrite(Output, 3, 0); // SPEED
      shiftOut(dataSRPin, clockSRPin, MSBFIRST, 0xf0);
      shiftOut(dataSRPin, clockSRPin, MSBFIRST, OutputSecond);
      shiftOut(dataSRPin, clockSRPin, MSBFIRST, Output);
      latch();
    }
    
    if (switchAutothrottleState == LOW) {
      // Autothrottle ON
      if (switchFlightDirectorState == HIGH || switchAutopilotState == HIGH) {
        Serial.println("S4");
      }
      else {
        Serial.println("S4");
        switchAutothrottleState = HIGH;
      }
    }
    switchAutothrottleState = !switchAutothrottleState;
  }
  
  // Flight director
  if (switchFlightDirector.update() && switchFlightDirector.read() == switchFlightDirectorState) {
    if (switchFlightDirectorState == LOW) {
      // Flight director ON
      Serial.println("S5");
      IASflag = 1;
      if (IAS < 114) {
        IAS = 114;
        lcd.setCursor(5,1);
        lcd.print(IAS);
      }
    }
    
    if (switchFlightDirectorState == HIGH) {
      // Flight director OFF
      Serial.println("S5");
      IASflag = 0;
      if (switchAutopilotState == LOW) {
        
        // VNAV
        if (bitRead(Output, 4) == 1) {
          bitWrite(Output, 4, 0);
          VNAVflag = 0;
          if (IASmachFlag == 1) {
            lcd.setCursor(5,1);
            lcd.print(double(IAS) * 0.00149984);
          }
          else {
            lcd.setCursor(5,1);
            lcd.print(IAS);
          }
        }
        // VNAV
        
        bitWrite(Output, 5, 0); // LVL CHG
        bitWrite(Output, 6, 0); // HDG SEL
        bitWrite(Output, 7, 0); // APP
        bitWrite(OutputSecond, 0, 0); // VOR LOC
        bitWrite(OutputSecond, 2, 0); // ALT HLD
        bitWrite(OutputSecond, 3, 0); // V/S
        
        shiftOut(dataSRPin, clockSRPin, MSBFIRST, 0xff);
        shiftOut(dataSRPin, clockSRPin, MSBFIRST, OutputSecond);
        shiftOut(dataSRPin, clockSRPin, MSBFIRST, Output);
        latch();
      }
    }
    switchFlightDirectorState = !switchFlightDirectorState;
  }

  // Landing gear
  if (switchLandingGear.update() && switchLandingGear.read() == switchLandingGearState) {
    Serial.println("S6");
    switchLandingGearState = !switchLandingGearState;
  }

  // Landing lights
  if (switchLandingLights.update() && switchLandingLights.read() == switchLandingLightsState) {
    Serial.println("S7");
    switchLandingLightsState = !switchLandingLightsState;
  }
  
  // Check if there has been any change in the vertical velocity
  if (Serial.available() > 0) {
    VSV = Serial.readString().toInt();
    lcd.setCursor(15,2);
    lcd.print("     "); 
    if (VSV == 0) {
    }
    else {
      lcd.setCursor(15,2);
      lcd.print(VSV);
    }
  }
}
