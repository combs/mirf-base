#include <Arduino.h>
#include <SPI.h>
#include <Mirf.h>
#include <Wire.h>
#include <MirfHardwareSpiDriver.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <EEPROM.h>
#include "FastLED.h"

void setup();

#include "mirfscreenconfig.h"
#include "watchdog.h"
#include "MirfClient.h"

#define MIRF_PINS_STANDARD
#define PIN_OUTPUT 5
#define PIN_GROUND 4
#define PIN_NRF A3

#define NUM_LEDS 7

volatile long secondsSinceStartup = 0;
volatile long secondsGotUpdate = 0;
volatile long secondsRequestedUpdate = 0;
volatile long secondsTurnedOn = 0;

volatile byte desiredRed=0;
volatile byte desiredGreen=0;
volatile byte desiredBlue=0;
volatile byte currentRed=0;
volatile byte currentGreen=0;
volatile byte currentBlue=0;

#define MODE_OFF 0
#define MODE_ONE_COLOR 1
#define MODE_WIPE 2
#define MODE_CYCLE_FAST 3
#define MODE_CYCLE 4
#define MODE_CYCLE_SLOW 5
#define MODE_CHASE_SOFT 6
#define MODE_CHASE_HARD 7
#define MODE_SCAN 8
#define MODE_BLINK 9

volatile byte currentMode=MODE_OFF;

CRGB leds[NUM_LEDS];


void setup()
{
  //  Serial.begin(115200);
  //  Serial.println("hello");

  wdt_disable();

  power_adc_disable();
  //  power_usart0_disable();

  getNameClient();
  getNameBase();

  pinMode(PIN_OUTPUT,OUTPUT); 
  pinMode(PIN_GROUND,OUTPUT);
  digitalWrite(PIN_GROUND,LOW);

  wakeRadio();

  pinMode(13,OUTPUT);
  digitalWrite(13,HIGH);
  delay(1000);
  digitalWrite(13,LOW);
  delay(500);
 

  FastLED.addLeds<NEOPIXEL, PIN_OUTPUT>(leds, NUM_LEDS);

  SendToBase("Starting");

  byte status=Mirf.getStatus(); 

  if (status==14) { 
    FastLED.showColor(CRGB(0,16,0));

    delay(500);
    FastLED.showColor(CRGB(0,0,0));
  } 
  else {  
    while (true) {
      pinMode(13,OUTPUT);
      digitalWrite(13,HIGH);
      FastLED.showColor(CRGB(16,0,0));

      delay(1000);
      digitalWrite(13,LOW);
      FastLED.showColor(CRGB(0,0,0));

      delay(500); 
    }
  }

  setup_watchdog(WDTO_1S);

  SendToBase("Started"); 
}



void loop()
{

  byte data[Payload];
  if( !Mirf.isSending() && Mirf.dataReady() )
  {
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

    switch (theCommand) {

    case '$':
      HandleBuiltinMessage(theMessage);
      delay(500); 


      break;
    case 'c':
      desiredRed=theMessage[0] - 32;
      currentRed=desiredRed=desiredRed<<2;
      desiredGreen=theMessage[1] - 32;
      currentGreen=desiredGreen=desiredGreen<<2;
      desiredBlue=theMessage[2] - 32;
      currentBlue=desiredBlue=desiredBlue<<2;
      currentMode=MODE_ONE_COLOR;
      
    }


  }

  switch(currentMode) {

  case MODE_OFF:
    FastLED.showColor(CRGB(0,0,0));
    break;
  case MODE_ONE_COLOR:
    FastLED.showColor(CRGB(currentRed,currentGreen,currentBlue));
    break;

  }



}


void wakeRadio() {

  pinMode(PIN_NRF,OUTPUT);  
  digitalWrite(PIN_NRF,HIGH);
  delay(200); // Power back up those bypass caps
  SetupMirfClient();

}


void sleepRadio() {

  Mirf.powerDown(); 
  digitalWrite(PIN_NRF,LOW);
  //  pinMode(PIN_NRF,INPUT);  

}



void waking() {


}

void sleeping() {

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  //pinMode(LCD_BACKLIGHT_PIN,INPUT); 

  sleepRadio();

  sleep_enable();
  sei();
  sleep_cpu();
  sleep_disable();
  waking();


}



void napping() {

  set_sleep_mode(SLEEP_MODE_PWR_SAVE);
  //pinMode(LCD_BACKLIGHT_PIN,INPUT); 

  sleep_enable();
  sei();
  sleep_cpu();
  sleep_disable();
  waking();

}

ISR(WDT_vect) {

  secondsSinceStartup++; 

}









