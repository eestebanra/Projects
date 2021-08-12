/*  Programmable Alarm Clock
    Developed by Esteban Rosero
    Electrical Engineering Co-op Student at the University of Alberta
*/

/*  IMPORTANT: 
 *  The buzzer always tiks three times in a row after resetting as it is connected to Pin 13, and Pin 13 always 
 *  goes HIGH and LOW three times on each start.
 *  
 *  volatile variables are declared as they are changing continuously within the ISR
 *  
 *  resetEnablePin, 4, needs to be HIGH to reset the Arduino. This is because Pin 4 is attached to the a transistor's 
 *  base which acts as a switch to set the RESET Pin LOW.
 *  
 *  It is extremely important to call the Timer1 library's methods as in the activateAlarm() function's order.
 *  Otherwise, the program won't start as the resetEnablePin, 4, will always be set HIGH.
 */
 
#include "RTClib.h"
#include "Bounce2.h"
#include "TimerOne.h"

RTC_DS1307 rtc;
Bounce bouncerOne = Bounce();
Bounce bouncerTwo = Bounce();

// Shift register parameters
#define dataPin 12
#define latchPin 11
#define clockPin 10

// 7 Segments parameters
#define separatorPin 9
#define digitOnePin 8
#define digitTwoPin 7
#define digitThreePin 6
#define digitFourPin 5

// Controls parameters
#define resetEnablePin 4
#define buttonOnePin 3
#define buttonTwoPin 2
#define buzzerPin 13
#define potPin A0


// Time parameters
int currentHour;
int currentMinute;
int currentSecond;
int pastSecond;

// Controls states
int buttonOneState = HIGH;
int buttonTwoState = HIGH;
volatile int separatorState = LOW;

// Alarm parameters
int alarmHour;
int alarmMinute;
const int alarmWait = 2; // Change this value to modify how much time, in minute, the alarm
                         // should sound for after it has gone off.
volatile int counterSecond = 0;

// Alarm states
int alarmSet = LOW;
int alarmState = LOW;

// Functions prototypes and definitions
void turnOFF() {
  digitalWrite(separatorPin, LOW);
  digitalWrite(digitOnePin, LOW);
  digitalWrite(digitTwoPin, LOW);
  digitalWrite(digitThreePin, LOW);
  digitalWrite(digitFourPin, LOW);
}

void digitState(int digit, int state) {
  switch (digit) {
    case 1:
    digitalWrite(digitOnePin, state);
    break;

    case 2:
    digitalWrite(digitTwoPin, state);
    break;

    case 3:
    digitalWrite(digitThreePin, state);
    break;

    case 4:
    digitalWrite(digitFourPin, state);
    break;
  }
}

void sendData(int dataIndex) {
  const byte LEDs[10] = {0xfc, 0x60, 0xda, 0xf2, 0x66, 0xb6, 0xbe, 0xe0, 0xfe, 0xf6};
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, LEDs[dataIndex]);
  digitalWrite(latchPin, HIGH);
}

void timeToLEDs(int hours, int minutes) {
  const int delayDigit = 2000; // Changing this value will either increase or decrease the 7 Segments's brightness
  // Hours
  sendData(hours / 10);
  digitState(1, 1);
  delayMicroseconds(delayDigit);
  digitState(1,0);

  sendData(hours % 10);
  digitState(2,1);
  delayMicroseconds(delayDigit);
  digitState(2,0);
  
  sendData(minutes / 10);
  digitState(3,1);
  delayMicroseconds(delayDigit);
  digitState(3,0);

  sendData(minutes % 10);
  digitState(4,1);
  delayMicroseconds(delayDigit);
  digitState(4,0);
}

void separatorBlink() {
  if ((currentSecond % 1) == 0 && (currentSecond != pastSecond)) {
    // Changing the number besides % will change how often the separator blinks
    separatorState = !separatorState;
    digitalWrite(separatorPin, separatorState);
    pastSecond = currentSecond;
  }
}

void setAlarm() {
  buttonOneState = !buttonOneState;
  digitalWrite(separatorPin, HIGH); // The fact that the separator LEDs are ON and not flashing 
                                    // anymore tells us that we have got into the set Alarm mode.
  while (buttonOneState == LOW) {
    // Set Alarm hour
    alarmHour = map(analogRead(A0), 10, 1013, 0, 24);
    timeToLEDs(alarmHour, currentMinute);
    if (bouncerTwo.update() && bouncerTwo.read() == LOW) {
      buttonTwoState = !buttonTwoState;
      while(buttonTwoState == LOW) {
        // Set Alarm minute
        alarmMinute = map(analogRead(A0), 10, 1013, 0, 59);
        timeToLEDs(alarmHour, alarmMinute);
        if (bouncerTwo.update() && bouncerTwo.read() == LOW) { // Get out of the set alarm mode
          buttonOneState = !buttonOneState;
          buttonTwoState = !buttonTwoState;
          alarmSet = HIGH; // The alarm has been set succesfully.
          // OPTIONAL
          Serial.println(alarmHour);
          Serial.println(alarmMinute);
          // OPTIONAL
        }
      }
    }
  }
}

void activateAlarm() {
  alarmState = !alarmState;
  turnOFF();
  digitState(4,1);

  // DON'T CHANGE
  Timer1.initialize(1000000);
  Timer1.attachInterrupt(secondCheck);
  Timer1.restart();
  // DON'T CHANGE
  
  while (alarmState == HIGH) {
    playTone();
  }
}

// ISRs

void secondCheck() {
  if (bouncerOne.update() && bouncerOne.read() == LOW) {
    digitalWrite(resetEnablePin, HIGH);
  }
  
  digitalWrite(separatorPin, separatorState);
  counterSecond++;
  
  if (counterSecond < (60 * alarmWait)) {
    sendData(counterSecond / 60);
    separatorState = !separatorState;
  }
  else {
    digitalWrite(resetEnablePin, HIGH);
  }
}

void setup() {
  // OPTIONAL
  Serial.begin(9600);
  // OPTIONAL

  rtc.begin();

  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);

  pinMode(digitOnePin, OUTPUT);
  pinMode(digitTwoPin, OUTPUT);
  pinMode(digitThreePin, OUTPUT);
  pinMode(digitFourPin, OUTPUT);
  
  pinMode(separatorPin, OUTPUT);
  digitalWrite(separatorPin, separatorState);

  pinMode(buttonOnePin, INPUT_PULLUP);
  bouncerOne.attach(buttonOnePin);  

  pinMode(buttonTwoPin, INPUT_PULLUP);
  bouncerTwo.attach(buttonTwoPin);

  pinMode(buzzerPin, OUTPUT);

  pinMode(resetEnablePin, OUTPUT);
  digitalWrite(resetEnablePin, LOW);

  turnOFF();
}

void loop() {

  DateTime now = rtc.now();
  currentHour = now.hour();
  currentMinute = now.minute();
  currentSecond = now.second();

  timeToLEDs(currentHour, currentMinute);

  separatorBlink();

  if (bouncerOne.update() && bouncerOne.read() == LOW) {
    setAlarm();
  }
  
  if ((currentHour == alarmHour && (currentMinute == alarmMinute) && (alarmSet == HIGH))) {
    activateAlarm();
  }
}
