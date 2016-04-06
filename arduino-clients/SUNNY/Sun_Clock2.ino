
#include <SPI.h>
#include <Mirf.h> 
#include <MirfHardwareSpiDriver.h>
#include <avr/power.h>
#include <avr/sleep.h>

// #define MIRF_PINS_STANDARD
// #define MIRF_PINS_PCB
#define MIRF_PINS_SUNCLOCK

#include <EEPROM.h>

#if defined( __AVR_ATtiny84__ )

// #include "TinyWireM.h"
// #include "TinyRTClib.h"
// #include <SoftwareSerial.h>


#else

#include <Wire.h>
#include "RTClib.h"

#endif

String inputString = "";         // a string to hold incoming data
const byte Payload = 32;
volatile byte updateRequested = true;
const long msTimeout=5000;
volatile long msGotUpdate = 0;
volatile long msRequestedUpdate = 0;
const long msMaxDataAge=60L*60L*1000;
volatile uint8_t mirfEnabled=false;


char nameBase[6] = "BASES";
char nameClient[6] = "SUNNY";

const int debugEnabled=0; // dimming is crappy with debug enabled

#define USE_PULLUPS


#if defined USE_PULLUPS
const int LOGICAL_ON = LOW;
const int LOGICAL_OFF = HIGH;


#else
const int LOGICAL_ON  = HIGH;
const int LOGICAL_OFF  = LOW;



#endif


RTC_DS1307 RTC;
DateTime timePreviousTick;
DateTime timeNow;

// const int lightSensorPin = 0; // the analog input pin for the photoresistor (used to keep from firing when it's very bright, maybe?)
// const int upButtonPin = 1; // the "up" button
// #define downButtonPin 0; // the "down" button

#if defined( __AVR_ATmega328P__ )
const int ledArrayPin = 9; // the main LED PWM pin with the TIP-120 on it
// const int ledArrayPin = 13; // the main LED PWM pin with the TIP-120 on it
const int setTimeButtonPin = 3; // the "set time" button
const int changeModeButtonPin = 2; // the "full/off" button
const int alarmEnabledSwitchPin = 10; // the "alarm enabled" switch
const int light0600Pin = 8; // 6:00
const int light0630Pin = 7; // 6:30
const int light0700Pin = 6; // 7:00
const int light0730Pin = 5; // 7:30
const int light0800Pin = 4; // 8:00
const int debugPin = 13;



#elif defined( __AVR_ATtiny84__ )

const int ledArrayPin = 6; // the main LED PWM pin with the TIP-120 on it
const int setTimeButtonPin = 7; // the "set time" button
const int changeModeButtonPin = 8; // the "full/off" button
const int alarmEnabledSwitchPin = 9; // the "alarm enabled" switch
const int light0600Pin = 0; // 6:00
const int light0630Pin = 1; // 6:30
const int light0700Pin = 2; // 7:00
const int light0730Pin = 3; // 7:30
const int light0800Pin = 4; // 8:00
const int debugPin = 10; 

SoftwareSerial Serial(-1, 11); // RX, TX

#else

const int ledArrayPin = 9; // the main LED PWM pin with the TIP-120 on it
const int setTimeButtonPin = 3; // the "set time" button
const int changeModeButtonPin = 2; // the "full/off" button
const int alarmEnabledSwitchPin = 10; // the "alarm enabled" switch
const int light0600Pin = 8; // 6:00
const int light0630Pin = 7; // 6:30
const int light0700Pin = 6; // 7:00
const int light0730Pin = 5; // 7:30
const int light0800Pin = 4; // 8:00
const int debugPin = 13;


// SoftwareSerial Serial(-1, 11); // RX, TX

#endif




const byte ACTION_FULLBRIGHT=5;
const byte ACTION_OFF=1;
const byte ACTION_BRIGHTEN=2;
const byte ACTION_NOCHANGE=0;
const byte ACTION_PREBRIGHTEN=3;
const byte ACTION_WAITFORSET=4;
const byte ACTION_PULSING=6;

// uint8_t nextAction __asm__("r12") = ACTION_NOCHANGE;

volatile byte nextAction=ACTION_NOCHANGE;
// volatile int nextAction=ACTION_PREBRIGHTEN;
volatile byte delayedAction=ACTION_NOCHANGE;


const byte EEPROM_ALARM_LOCATION = 200;
// const byte EEPROM_ENABLED_LOCATION = 201;
// replaced by switch




const byte TIME_0600 = 1;
const byte TIME_0630 = 2;
const byte TIME_0700 = 3;
const byte TIME_0730 = 255; // the default for an unread EEPROM location.
const byte TIME_0800 = 5;

byte alarmTime=TIME_0730;


int intervalTimer=0;
int dimmingTimer=0;
unsigned long numReps=1;
unsigned long interval=255000;
unsigned long setModeOffTime=0;
unsigned long ledArrayOffTime=0;
unsigned long nextDimIncreaseTime=millis();

const unsigned long DIMMING_FINISH = 16384575;
const unsigned int INTERVAL_FINISH=0;

byte snoozeMode = 0;

const int maxDim=1001; // microseconds off time between 1us on times for custom dimming
const int minDim=1; // microseconds off time between 1us on times for custom dimming 



const unsigned int numSeconds=60;
unsigned int numSteps=numSeconds/2;

const unsigned long numUseconds=numSeconds*1000000;  
const unsigned long numUsecondsPerRun=numUseconds/numSteps;
//const unsigned long numMS=numSeconds*1000;  
//const unsigned long numMSPerRun=numMS/numSteps;


const long debounceDelay = 200;    // the debounce time; increase if buttons oscillate

long lastDebounceTime = millis()-debounceDelay-50;  // the last time the output pin was toggled




// int ledBrightness=0;
// const int maxLedBrightness=10000;

// const int ledBrightnessSwitchover=5000; // this is the point past which it switches to timing by on-delay instead of off-delay


int upButtonState = LOW;
int snoozeButtonState = LOW;
int changeModeButtonState = LOW;
int alarmSetSwitchState = LOW; 
int lastAlarmSetSwitchState = LOW; 

volatile byte setModeOn=0;


void setup() {

  // clear LCD or blink setting LEDs
  // pinMode(upButtonPin, INPUT);

#if defined USE_PULLUPS

  pinMode(setTimeButtonPin, INPUT_PULLUP);
  pinMode(changeModeButtonPin, INPUT_PULLUP);
  pinMode(alarmEnabledSwitchPin, INPUT_PULLUP);
#else


  pinMode(setTimeButtonPin, INPUT);
  pinMode(changeModeButtonPin, INPUT);
  pinMode(alarmEnabledSwitchPin, INPUT);
#endif

  pinMode(ledArrayPin, OUTPUT);
  pinMode(light0600Pin, OUTPUT);
  pinMode(light0630Pin, OUTPUT);
  pinMode(light0700Pin, OUTPUT);
  pinMode(light0730Pin, OUTPUT);
  pinMode(light0800Pin, OUTPUT);

  //  digitalWrite(setTimeButtonPin, HIGH); // enable pullup resistor
  //  digitalWrite(alarmEnabledSwitchPin, HIGH); // enable pullup resistor
  //  digitalWrite(changeModeButtonPin, HIGH); // enable pullup resistor
  digitalWrite(light0600Pin,HIGH);  

  power_adc_disable();

  uint8_t mcucr1, mcucr2;
  mcucr1 = MCUCR | _BV(BODS) | _BV(BODSE);  //turn off the brown-out detector
  mcucr2 = mcucr1 & ~_BV(BODSE);
  MCUCR = mcucr1;
  MCUCR = mcucr2;


  if (debugEnabled==1) {
    Serial.begin(115200);                   // TODO do we really need this? saves 1k to omit
  }


#if defined( __AVR_ATtiny84__ )
  TinyWireM.begin();

#else
  Wire.begin(); // required before RTC
#endif
  digitalWrite(light0630Pin,HIGH);  

  RTC.begin();
  digitalWrite(light0700Pin,HIGH);  

  //  pinMode(debugPin, OUTPUT);
  // pinMode(2, INPUT);

  if ( digitalRead(setTimeButtonPin)==LOGICAL_ON) {
    setLedsOn();
    ledArrayDimBlink();


    delay(1000);
    debug();
    delay(1000);
  }



#if defined USE_PULLUPS

  attachInterrupt(0, snoozeButton, FALLING);
  attachInterrupt(1, setButton, FALLING);
#else

  attachInterrupt(0, snoozeButton, RISING);
  attachInterrupt(1, setButton, RISING);

#endif

  // EEPROM.write(EEPROM_ALARM_LOCATION, alarmTime);

  alarmTime = EEPROM.read(EEPROM_ALARM_LOCATION);
  //   EEPROM.write(EEPROM_ALARM_LOCATION, alarmTime);


  if (numSteps > (maxDim-minDim) ) {
    numSteps = maxDim-minDim;
  }



  byte dirty=0;

  if ( RTC.isrunning() ) {
    timeNow = RTC.now(); 
    timePreviousTick=timeNow;

    if (timeNow.year() < 2013 || timeNow.year() > 2050 ) {

      dirty=1;
    }

  } 
  else {
    dirty=1;

  }

  if (debugEnabled==0) {
    if(dirty==1) {
      //     setLedsOn();
      //   delay(1000);
      //  setLedsOff();
      //   delay(1000);
      setLedsSweep();


    }

  } 
  else {
    if (dirty==1){
      Serial.println("RTC not responding");
    } 
  }

  digitalWrite(light0730Pin,HIGH);  

  Mirf.spi = &MirfHardwareSpi;
#ifdef MIRF_PINS_PCB
  Mirf.cePin=8;
  Mirf.csnPin=9;

#elif defined  MIRF_PINS_SUNCLOCK 
  Mirf.cePin=A3;
  Mirf.csnPin=A2;

#endif
  Mirf.init();

  // name the receiving channel - must match tranmitter setting!
  Mirf.setRADDR((byte *)nameClient);

  Mirf.setTADDR((byte *)nameBase);


  Mirf.payload=Payload;

  Mirf.channel = 10;


  // configure 15 retries, 500us between attempts
  Mirf.configRegister(SETUP_RETR,(B0001<<ARD ) | (B1111<<ARC));

  // Set 1MHz data rate - this increases the range slightly
  Mirf.configRegister(RF_SETUP,0x06);

  // now config the device.... 
  Mirf.config();  

  byte status=Mirf.getStatus();
  if (status==14) { //good startup
    SendToBase("Started");
    mirfEnabled=true;
    requestUpdate();
  } 
  else {
    writeBinary(status);

    ledArrayPulse(1);

  }



  digitalWrite(light0800Pin,HIGH);  
  setButton();
  //   setLedsOff();
  // ledArrayPulse(1);


};


void ledArrayPulse(int reps) {
  delayedAction=nextAction;
  nextAction=ACTION_PULSING;

  for (int a=0;a<reps; a++){
    for (int b=255; b>1; b--) {
      analogWrite(ledArrayPin,b);
      delay(5);
    }
    if (nextAction != ACTION_PULSING) {
      break;
    }

    for (int b=0; b<255; b++) {
      analogWrite(ledArrayPin,b);
      delay(5);
    }
    if (nextAction != ACTION_PULSING) {
      break;
    }

  }
  digitalWrite(ledArrayPin,HIGH);
}









void writeBinary(int theNum) {
  setLedsOff();
  delay(500); 

  theNum=theNum%10;

  digitalWrite(light0800Pin,theNum & B00000001);
  digitalWrite(light0730Pin,((theNum & B00000010) >> 1));
  digitalWrite(light0700Pin,((theNum & B00000100) >> 2));
  digitalWrite(light0630Pin,((theNum & B00001000) >> 3));

  if (theNum==0) {
    setLedsSweep();
  }

  delay(3000);
  setLedsOff();

}

void setLedsSweep(void){
  setLedsOff();
  digitalWrite(light0600Pin,1);
  delay(50);

  digitalWrite(light0600Pin,0);

  digitalWrite(light0630Pin,1);
  delay(50);
  digitalWrite(light0630Pin,0);

  digitalWrite(light0700Pin,1);
  delay(50);
  digitalWrite(light0700Pin,0);

  digitalWrite(light0730Pin,1);
  delay(50);
  digitalWrite(light0730Pin,0);

  digitalWrite(light0800Pin,1);
  delay(50);
  digitalWrite(light0800Pin,0);

  setLedsOff();


}

void debug(void) {

  timeNow = RTC.now();
  unsigned int year=timeNow.year();
  int month=timeNow.month();
  int day=timeNow.day();

  int hour=timeNow.hour();
  int min=timeNow.minute();

  ledArrayDimBlink();

  /*
for (int a=0; a<12; a++){
   writeBinary(a);
   }*/

  writeBinary(int(hour/10));
  writeBinary(int(hour%10));
  ledArrayDimBlink();

  writeBinary(int(min/10));
  writeBinary(int(min%10));
  ledArrayDimBlink();

  writeBinary(int(month/10));
  writeBinary(int(month%10));
  ledArrayDimBlink();

  writeBinary(int(day/10));
  writeBinary(int(day%10));
  ledArrayDimBlink();

  writeBinary(int(year/1000));
  writeBinary(int(year/100));
  writeBinary(int(year/10));
  writeBinary(int(year%10));
  ledArrayDimBlink();





}

void ledArrayDimBlink(void){
  analogWrite(ledArrayPin,1);
  delay(1000);
  analogWrite(ledArrayPin,0);

}



void setModeDisable(void) {
  setLedsOff();
  setModeOn=0;
  // not sure if we should move the nextAction over here
  alarmSetSwitchState = digitalRead(alarmEnabledSwitchPin);

  // blink current selection as confirmation
  if (alarmSetSwitchState==LOGICAL_ON) {
    for (int a=0;a<4;a++) {
      delay (100);
      setLedsCurrentAlarm();
      delay (100);
      setLedsOff();
    }
  }

}

void setLedsCurrentAlarm(void) {

  setLedsOff();

  switch (alarmTime) {
    // light up the led for the currently selected alarm time

    case TIME_0600:
    digitalWrite(light0600Pin,HIGH);
    break;

  case TIME_0630:
    digitalWrite(light0630Pin,HIGH);
    break;

  case TIME_0700:
    digitalWrite(light0700Pin,HIGH);
    break;

  case TIME_0730:
    digitalWrite(light0730Pin,HIGH);
    break;

  case TIME_0800:
    digitalWrite(light0800Pin,HIGH);
    break;


  }
}

void setLedsOff(void) { 
  digitalWrite(light0600Pin,LOW);  
  digitalWrite(light0630Pin, LOW); 
  digitalWrite(light0700Pin, LOW); 
  digitalWrite(light0730Pin,LOW);
  digitalWrite(light0800Pin, LOW);
  // we could replace this with a bitwise operator but what if the pins change? maybe once out of breadboard
  // TODO do we need this function?



  // setModeOn=0; ugly hack

};


void setLedsOn(void) { 
  digitalWrite(light0600Pin,HIGH);  
  digitalWrite(light0630Pin, HIGH); 
  digitalWrite(light0700Pin, HIGH); 
  digitalWrite(light0730Pin,HIGH);
  digitalWrite(light0800Pin, HIGH);
  // we could replace this with a bitwise operator but what if the pins change? maybe once out of breadboard
  // TODO do we need this function?



  // setModeOn=0; ugly hack

};
void ledArrayOn(void) {

  dimmingTimer=DIMMING_FINISH;
  intervalTimer=INTERVAL_FINISH;
  //  analogWrite(ledArrayPin,255);
  digitalWrite(ledArrayPin,HIGH);

}
void ledArrayOff(void) {

  dimmingTimer=DIMMING_FINISH;
  intervalTimer=INTERVAL_FINISH;
  //  analogWrite(ledArrayPin,255);
  digitalWrite(ledArrayPin,LOW);

}


void setButton(void) {
  if( (millis() - lastDebounceTime) < debounceDelay) {
    return;
  }
  lastDebounceTime=millis();


  if (debugEnabled==1) {
    Serial.println("Set!");

    digitalWrite(debugPin,1);


    // TODO this is for debug
    digitalWrite(light0730Pin,HIGH);
  }


  switch (nextAction) {
  case ACTION_WAITFORSET:
  case ACTION_NOCHANGE:
  case ACTION_OFF:
    break;
  case ACTION_FULLBRIGHT:
  case ACTION_BRIGHTEN: 
  case ACTION_PULSING:
  case ACTION_PREBRIGHTEN:
    ledArrayOff();
    nextAction=ACTION_OFF;
    return;
    break;

  }




  setModeOffTime=millis() + 2000;

  if (setModeOn==0) {
    setModeOn=1;

    delayedAction=nextAction;
    nextAction=ACTION_WAITFORSET;
    // if we're not in set mode, let's not change the alarm time



  } 
  else {
    // we are already in set mode, so let's increment by one for each set button press

    switch (alarmTime) {

    case TIME_0600:
      alarmTime=TIME_0630;

      break;

    case TIME_0630:
      alarmTime=TIME_0700;
      break;

    case TIME_0700:
      alarmTime=TIME_0730;
      break;

    case TIME_0730:
      alarmTime=TIME_0800;
      break;

    case TIME_0800:
      alarmTime=TIME_0600;
      break;
    default:
      alarmTime=TIME_0730;



    }
    setLedsCurrentAlarm();

    EEPROM.write(EEPROM_ALARM_LOCATION, alarmTime);

  }




  if ( nextAction != ACTION_WAITFORSET) {
    delayedAction=nextAction;
  }
  nextAction=ACTION_WAITFORSET;

  setLedsCurrentAlarm();





}




void snoozeButton(void) {
  if( (millis() - lastDebounceTime) < debounceDelay){
    return;
  }
  lastDebounceTime=millis();


  if (debugEnabled==1) {

    Serial.println("Snooze!");
    digitalWrite(debugPin,1);
  }

  switch (nextAction) {
  case ACTION_BRIGHTEN: 
    ledArrayOff();
    nextAction=ACTION_OFF;
    break;
  case ACTION_FULLBRIGHT:
    ledArrayOff();
    nextAction=ACTION_OFF;
    break;
  case ACTION_OFF:
    //ledArrayOn();
    analogWrite(ledArrayPin,1);
    nextAction=ACTION_BRIGHTEN;    
    // nextAction=ACTION_FULLBRIGHT;
    break;
  case ACTION_PULSING:
    ledArrayOff();
    nextAction=ACTION_OFF;
    break;
  case ACTION_PREBRIGHTEN:
    ledArrayOff();
    nextAction=ACTION_OFF;
    break;
  case ACTION_NOCHANGE:
    ledArrayOn();
    nextAction=ACTION_FULLBRIGHT;
    ledArrayOffTime=millis()+1800000;
    break;
  case ACTION_WAITFORSET:
    // snooze button pressed while in set mode--turn off set
    nextAction=delayedAction;
    delayedAction=ACTION_NOCHANGE;

    setModeDisable();


    break;

  }
  if (digitalRead(setTimeButtonPin)==LOGICAL_ON) {

    setModeDisable();
    nextAction=ACTION_PREBRIGHTEN;
    lastDebounceTime = millis()+5000;

  }


}



void loop(void) {


  byte data[Payload];

  if (updateRequested==true && mirfEnabled==true) {

    updateRequested=false;
    msRequestedUpdate=millis();
    requestUpdate();


  }
  if (mirfEnabled==true && (millis() - msRequestedUpdate > msTimeout ) && (millis() - msRequestedUpdate < ( msTimeout+msTimeout ) ) && (millis() - msGotUpdate > msTimeout+msTimeout ) ) {
    SendToBase("Timeout");
    msRequestedUpdate=millis();
    requestUpdate();


  }

  // is there any data pending? 
  if( mirfEnabled==true && Mirf.dataReady() )
  {

    // if (millis() - msRequestedUpdate < msTimeout) {
    // msGotUpdate = millis();
    // We don't want to cache unrelated status updates from the base station
    // - if a message is received that was not recently requested, do not update receive time

    //}
    msGotUpdate=millis();

    Mirf.getData((byte *) &data);
    String thePayload=String((char *)data);

    char theMessage[Payload];
    thePayload.substring(6).toCharArray(theMessage,Payload);
    
    //    SendToBase("Ack");
    for (byte a=0;a<Payload;a++) {
      if (theMessage[a]=='\r' || theMessage[a]=='\n'){
        theMessage[a]='\0';
        // Look for linebreaks and swap them out for end-of-string
      }
    }
    byte x=0;
    byte y=0;
    char theFirst[11];
    char theSecond[11];
    byte updateDirty=0;

    byte length=strlen(theMessage);
    if (length != 22) {
      updateDirty=1;
    }
    byte theVar=1;
    theFirst[10]='\0';
    theSecond[10]='\0';
    if (theMessage[0] != 't') {
      updateDirty=1;
    }
    for (byte a=1;a<22;a++) {
      if (theMessage[a]=='|') {
        theVar=2; 
      } 
      else {
        if (theVar==1) {
          theFirst[a]=theMessage[a];
        } 
        else if (theVar==2) {
          theSecond[a-11]=theMessage[a];
          if (theMessage != '\0' && theMessage[a-11] != theMessage[a]) {
            updateDirty=1;
          }

        }

      }

    }


    long theUnixTime = atol(theFirst);
    // SendToBase(theMessage);
    if (updateDirty==0) {
      RTC.adjust(DateTime(theUnixTime));

      DateTime now = RTC.now();
      // char theAck[Payload]="Updated 0000-00-00-00-00";
      //                            8    13 16 19 22
      char theAck[Payload]="Updated-";
      char scratch[5];
      itoa(now.year(),scratch,10);
      strcat(theAck,scratch);
      strcat(theAck,"-");
      itoa(now.month(),scratch,10);
      strcat(theAck,scratch);
      strcat(theAck,"-");
      itoa(now.day(),scratch,10);
      strcat(theAck,scratch);
      strcat(theAck,"-");
      itoa(now.hour(),scratch,10);
      strcat(theAck,scratch);
      strcat(theAck,"-");
      itoa(now.minute(),scratch,10);
      strcat(theAck,scratch);
      strcat(theAck,"-");
      itoa(now.second(),scratch,10);
      strcat(theAck,scratch); 
      SendToBase(theAck);
      Mirf.powerDown(); 

    } 
    else {

      char theAck[Payload]="no-";
      strcat(theAck,theFirst);
      strcat(theAck,theSecond);
      SendToBase(theAck);
      Mirf.powerDown(); 


    }


  }

  if (mirfEnabled==true &&  ( ( millis() - msGotUpdate ) > msMaxDataAge ) && ( ( millis() - msRequestedUpdate ) > msTimeout ) ) {
    msRequestedUpdate=millis();
    requestUpdate();
  }



  //  upButtonState = digitalRead(upButtonPin);
  //   snoozeButtonState = digitalRead(setTimeButtonPin);
  changeModeButtonState = digitalRead(changeModeButtonPin);
  lastAlarmSetSwitchState = alarmSetSwitchState;
  alarmSetSwitchState = digitalRead(alarmEnabledSwitchPin);



  if (alarmSetSwitchState == LOGICAL_ON &&  lastAlarmSetSwitchState == LOGICAL_OFF && setModeOn==0) {
    setButton();

  }  
  else if (alarmSetSwitchState == LOGICAL_OFF &&  lastAlarmSetSwitchState == LOGICAL_ON) {
    nextAction=ACTION_OFF;
    ledArrayOff();
    setLedsOff();
    setModeOn=0;
    setModeOffTime=millis(); 
  }



  if (debugEnabled==1) {
    digitalWrite(debugPin,LOW);
    Serial.print("nextAction: " );
    Serial.println(nextAction);
  }

  unsigned int dimInterval;
  dimInterval=(maxDim-minDim)/numSteps;
  if (dimInterval==0) {
    dimInterval=1;
  }


  byte dirty=0;

  switch (nextAction) {
  case ACTION_PULSING:

    nextAction=ACTION_FULLBRIGHT;
    ledArrayOffTime=millis()+1800000;


  case ACTION_PREBRIGHTEN:

    nextDimIncreaseTime=millis()+1000L;
    //    interval=255000;
    interval=16383L;


    while (interval > 1) {
      if (debugEnabled==1) {
        Serial.print(" interval: ");
        Serial.println(interval); 
      }



      while (millis() < nextDimIncreaseTime) {

        digitalWrite(ledArrayPin,HIGH); 
        digitalWrite(ledArrayPin,LOW);
        delayMicroseconds(interval);

        digitalWrite(ledArrayPin,HIGH); 
        digitalWrite(ledArrayPin,LOW);
        delayMicroseconds(interval);
        digitalWrite(ledArrayPin,HIGH); 
        digitalWrite(ledArrayPin,LOW);
        delayMicroseconds(interval);
        digitalWrite(ledArrayPin,HIGH); 
        digitalWrite(ledArrayPin,LOW);
        delayMicroseconds(interval);
        digitalWrite(ledArrayPin,HIGH); 
        digitalWrite(ledArrayPin,LOW);
        delayMicroseconds(interval);
        digitalWrite(ledArrayPin,HIGH); 
        digitalWrite(ledArrayPin,LOW);
        delayMicroseconds(interval);
        digitalWrite(ledArrayPin,HIGH); 
        digitalWrite(ledArrayPin,LOW);
        delayMicroseconds(interval);
        digitalWrite(ledArrayPin,HIGH); 
        digitalWrite(ledArrayPin,LOW);
        delayMicroseconds(interval);
        digitalWrite(ledArrayPin,HIGH); 
        digitalWrite(ledArrayPin,LOW);
        delayMicroseconds(interval);
        digitalWrite(ledArrayPin,HIGH); 
        digitalWrite(ledArrayPin,LOW);
        delayMicroseconds(interval);

      }


      //      interval=long(interval*0.992L);
      interval=long(interval*1015 >> 10) ;

      nextDimIncreaseTime=nextDimIncreaseTime+1000L;


      if (nextAction != ACTION_PREBRIGHTEN) {
        interval=0;
      }



    }
    analogWrite(ledArrayPin,128);

    nextAction=ACTION_BRIGHTEN;


  case ACTION_BRIGHTEN:
    if (debugEnabled==1) {
      digitalWrite(debugPin,HIGH); 
    }

    for (int a=128; a<=255; a++) {
      if (nextAction != ACTION_BRIGHTEN) {
        dirty=1;
        break;
      }


      if (debugEnabled==1) {
        Serial.print("analog value: ");
        Serial.println(a);
      }
      analogWrite(ledArrayPin, a);
      delay(3000);
    }

    if (dirty==0) {

      //      ledArrayPulse(116);
      ledArrayPulse(232);

      nextAction=ACTION_FULLBRIGHT;
      ledArrayOffTime=millis()+1800000;
    }


    if (debugEnabled==1) {    
      Serial.println("-------------");
    }

    break; 
  case ACTION_FULLBRIGHT: 
    ledArrayOn();

    if (millis() > ledArrayOffTime) {
      nextAction=ACTION_OFF;
    } 
    else {
      break;
    }



    // digitalWrite(ledArrayPin,HIGH);

    //     nextAction=ACTION_NOCHANGE; 
  case ACTION_OFF:
    digitalWrite(ledArrayPin, LOW);
    nextAction=ACTION_NOCHANGE;

    break;
  case ACTION_NOCHANGE:
    if (alarmSetSwitchState == LOGICAL_ON ) { // on switch is set to on

      byte theHour=7;
      byte theMinute=0; //default to 07:00 start time for 07:30 alarm
      switch (alarmTime) {

      case TIME_0600:
        theHour=5;
        theMinute=30;


        break;

      case TIME_0630:
        theHour=6;
        theMinute=0;


        break;

      case TIME_0700:
        theHour=6;
        theMinute=30;

        break;

      case TIME_0730:
        theHour=7;
        theMinute=0;

        break;

      case TIME_0800:
        theHour=7;
        theMinute=30;

        break;


      }



      if ( RTC.isrunning() ) {
        DateTime timeNow=RTC.now();

        if (debugEnabled==1) {    
          Serial.println(timeNow.minute());
        }

        /*        if (timeNow.unixtime() < timePreviousTick.unixtime()) {
         
         RTC.adjust(timePreviousTick); // time slowing down, use previous time
         // 
         timeNow=timePreviousTick;
         
         }
         */

        timePreviousTick=timeNow;

        if (timeNow.hour()==theHour && timeNow.minute()==theMinute ) { // should be precise enough
          nextAction=ACTION_PREBRIGHTEN;
          SendToBase("Awakening");

        }

      } 
      else {
        // setLedsSweep();

        //        setLedsOn();
        //      delay(200);
        //    setLedsOff();
        SendToBase("RTC is not running");

        if (debugEnabled==1) {    
          Serial.println("RTC is not running");
        }

      }

    }
    break;




  case ACTION_WAITFORSET:
    if ( setModeOffTime < millis()) { // ten-second delay over 

      nextAction=delayedAction;
      delayedAction=ACTION_NOCHANGE;

      setModeDisable();

    } 
    else {
      delay (300); // get some rest
    };

    break;

  }

  delay (500); // this should be okay or even longer if the interrupts are working properly 
}







void requestUpdate() {

  SendToBase("update");


}


void SendToBase(String theMessage) {
  char thePayload[Payload];
  char theMessageChar[Payload];

  Mirf.config();
  // wake up from sleep

  Mirf.setTADDR((byte *)nameBase);
  Mirf.setRADDR((byte *)nameClient);
  Mirf.config();
  // initial set addresses

  strcpy(thePayload,nameClient);
  theMessage.toCharArray(theMessageChar,Payload);
  strcat(thePayload,theMessageChar);

  Mirf.send((byte *)thePayload);
  blockForSend();

  // something changes the RADDR to nameBase... auto ack? let's change it back

  Mirf.setRADDR((byte *)nameClient);
  Mirf.config();

}


void blockForSend() {

  while( Mirf.isSending() )
  {
    delay(1);
  }
}











