#include <SPI.h>
#include <Mirf.h>
#include <Wire.h>
#include <MirfHardwareSpiDriver.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <EEPROM.h>

void setup();

#include "mirfscreenconfig.h"
#include "watchdog.h"
#include "MirfClient.h"


#define MIRF_PINS_STANDARD
// #define MIRF_PINS_PCB

volatile byte updateRequested = true;
volatile byte paintRequested = true;
volatile byte sleepRequested = false;

volatile long secondsSinceStartup = 0;
volatile long secondsGotUpdate = 0;
volatile long secondsRequestedUpdate = 0;
volatile long secondsTurnedOn = 0;
volatile long secondsNapStarted=0;


#define PIN_METER_ONE 3
#define PIN_METER_TWO 5
#define PIN_METER_THREE 6


volatile int valueOne=0;
volatile int valueTwo=0;
volatile int valueThree=0;

void setup()
{
  //  Serial.begin(115200);
  //  Serial.println("hello");

  wdt_disable();
  power_adc_disable();
  power_usart0_disable();

  getNameClient();
  getNameBase();

  pinMode(2,INPUT);

  pinMode(PIN_METER_ONE,OUTPUT);
  pinMode(PIN_METER_TWO,OUTPUT);
  pinMode(PIN_METER_THREE,OUTPUT);

  SetupMirfClient();

  SendToBase("Starting");

  byte status=Mirf.getStatus(); 

  if (status==14) { 

  } 
  else { 
    pinMode(13,OUTPUT);
    while (true) {
      digitalWrite(13,HIGH);
      delay(1000);
      digitalWrite(13,LOW);
      delay(500);
      wanderMeters(5);
    }
  }


 // attachInterrupt(0,flagUpdate,RISING);
  flagUpdate();

  SendToBase("Started");
  delay(100); // Avoid message-spamming race condition at startup

  setup_watchdog(WDTO_1S);
  wanderMeters(1);



}


void flagUpdate() {
  updateRequested=true;
  secondsTurnedOn=secondsSinceStartup;

}


void loop()
{


  //  waking();

  if (paintRequested==true) {
    display();
    backlight();
    paintScreen();
    paintRequested=false;
  }


  if ( secondsSinceStartup - secondsTurnedOn > secondsScreenAwakeTime ) {
    
    noBacklight();
    noDisplay(); 
    sleeping();
    
  } else if ((( secondsSinceStartup - secondsNapStarted < secondsSleep )) && (secondsNapStarted != 0)) 
    {
    Mirf.powerDown();
    noBacklight();
    noDisplay();
    napping();

  } 

  else {

    Mirf.powerUpRx();

    if ( ( ( secondsSinceStartup - secondsGotUpdate ) > secondsMaxDataAge ) && 
      ( ( secondsSinceStartup - secondsRequestedUpdate ) > secondsTimeout ) ) {
      secondsRequestedUpdate=secondsSinceStartup;

      requestUpdate();
      // screen is on, data is old, but let's not flagUpdate because that will keep screen on longer

    }

    display();
    backlight();

    //    paintScreen();

    byte data[Payload];

    if (updateRequested==true) {

      updateRequested=false;

      if  ( // more than double timeout since last asked OR just started
      ( ( secondsSinceStartup - secondsRequestedUpdate > 
        secondsTimeout+secondsTimeout ) ||  
        (secondsGotUpdate==0 && secondsRequestedUpdate==0) )  && 
        //  AND
      // it's been longer than maxDataAge since we got info OR we've never gotten info

      ( ( secondsSinceStartup - secondsGotUpdate > secondsMaxDataAge ) || 
        secondsGotUpdate==0 ) 
        // OR we just woke up from sleep
      ||
        ( ( (secondsSinceStartup - secondsNapStarted) > (secondsSleep - 1) ) && 
        ( (secondsSinceStartup - secondsNapStarted) < (secondsSleep + 5) ) && 
        ( (secondsSinceStartup - secondsRequestedUpdate) > (secondsSleep) )  )

      ) {

        secondsRequestedUpdate=secondsSinceStartup;
        requestUpdate();

      } 

    }
    if ((secondsSinceStartup - secondsRequestedUpdate > secondsTimeout ) && 
      (secondsSinceStartup - secondsRequestedUpdate < ( secondsTimeout+secondsTimeout ) ) && 
      (secondsSinceStartup - secondsGotUpdate > secondsTimeout+secondsTimeout ) ) {

      SendToBase("Timeout");
      secondsRequestedUpdate=secondsSinceStartup;
      requestUpdate();

    }

    // is there any data pending? 
    if( !Mirf.isSending() && Mirf.dataReady() )
    {

#ifdef LCD_UPDATES_EXTEND_ON_TIME
      secondsTurnedOn=secondsSinceStartup;
#endif

      Mirf.getData((byte *) &data);
      String thePayload=String((char *)data);
      char theCommand=thePayload[5];
      char theMessage[Payload];
      thePayload.substring(6).toCharArray(theMessage,Payload);

      for (byte a=0;a<Payload;a++) {
        if (theMessage[a]=='\r' || theMessage[a]=='\n'){
          theMessage[a]='\0';
        }
      }

      byte length=strlen(theMessage);
      switch (theCommand) {

      case '$':
        wanderMeters(5);
        HandleBuiltinMessage(theMessage);  

        break;

      case 'S':
        // Sleep
        sleepRequested=true;
        updateRequested=true;
        break;


      case 'A':  // Ack request; don't print anything
        secondsRequestedUpdate=secondsSinceStartup;
        // reset the timeout.
        break;

      case 'V':  // Values
        if (length>8) {
          char stringOne[4]="000";
          char stringTwo[4]="000";
          char stringThree[4]="000";

          secondsGotUpdate=secondsSinceStartup;
          thePayload.substring(6,9).toCharArray(stringOne,4);
          thePayload.substring(9,12).toCharArray(stringTwo,4);

          thePayload.substring(12,15).toCharArray(stringThree,4);

          /*     Serial.println(thePayload);
           
           Serial.println(valueOne);
           Serial.println(valueTwo);
           Serial.println(valueThree);
           */
          valueOne=atoi(stringOne);
          valueTwo=atoi(stringTwo);
          valueThree=atoi(stringThree);

          analogWrite(PIN_METER_ONE,atoi(stringOne));
          analogWrite(PIN_METER_TWO,valueTwo);
          analogWrite(PIN_METER_THREE,valueThree);

        }

      default:
        break;
      }


    }
  }

  if ( (secondsSinceStartup - secondsRequestedUpdate <  
    secondsTimeout+secondsTimeout ) && ( sleepRequested==false) ) {
    delay(50);
  } 
  else {
    if (sleepRequested==true) {
      secondsNapStarted=secondsSinceStartup;
      sleepRequested=false;
    }

    napping();

  }

}

void requestUpdate() {
  SendToBase("poll");
}



void waking() {


}

void sleeping() {

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  //pinMode(LCD_BACKLIGHT_PIN,INPUT); 

  Mirf.powerDown(); 
  sleep_enable();
  sei();
  sleep_cpu();
  sleep_disable();
  waking();

}



void napping() {

  set_sleep_mode(SLEEP_MODE_IDLE);
  //pinMode(LCD_BACKLIGHT_PIN,INPUT); 

  sleep_enable();
  sei();
  sleep_cpu();
  sleep_disable();
  waking();

}

ISR(WDT_vect) {

  if (secondsSinceStartup + 1 < secondsSinceStartup) {
    secondsTurnedOn = 0; 
    secondsGotUpdate = 0;
    secondsRequestedUpdate = 0;
  }

  secondsSinceStartup++;

}






void wanderMeters(int seconds) {

  long msQuitTime=millis() + (seconds * 1000) ;

  byte one=random(255);
  byte two=random(255);
  byte three=random(255);
  byte speed_one=random(5);
  byte speed_two=random(5);
  byte speed_three=random(5);

  analogWrite(PIN_METER_ONE,random(255));
  analogWrite(PIN_METER_TWO,random(255));
  analogWrite(PIN_METER_THREE,random(255));

  while (millis() < msQuitTime) {
    one+=speed_one;
    two+=speed_two;
    three+=speed_three;
    analogWrite(PIN_METER_ONE,one);
    analogWrite(PIN_METER_TWO,two);
    analogWrite(PIN_METER_THREE,three);

    /*    OCR0B++;
     OCR0A++;
     OCR2B++;
     */
    delay(20);
  }




}



void paintScreen() {

  analogWrite(PIN_METER_ONE,valueOne);
  analogWrite(PIN_METER_TWO,valueTwo);
  analogWrite(PIN_METER_THREE,valueThree);
}


void display() {

  pinMode(PIN_METER_ONE,OUTPUT);
  pinMode(PIN_METER_TWO,OUTPUT);
  pinMode(PIN_METER_THREE,OUTPUT); 


}

void noDisplay() {
  pinMode(PIN_METER_ONE,INPUT);
  pinMode(PIN_METER_TWO,INPUT);
  pinMode(PIN_METER_THREE,INPUT);
  digitalWrite(PIN_METER_ONE,LOW);
  digitalWrite(PIN_METER_TWO,LOW);
  digitalWrite(PIN_METER_THREE,LOW);



}



void backlight() {

}
void noBacklight() {
}
















