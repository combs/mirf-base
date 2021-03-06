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

const byte NUM_LEDS = 128;
#define CYCLE_MULTIPLIER 1 // make this larger for short strips

volatile long secondsSinceStartup = 0;
volatile long secondsGotUpdate = 0;
volatile long secondsRequestedUpdate = 0;
volatile long secondsTurnedOn = 0;
volatile long secondsIdleTime = 300;
volatile long millisFadeStarted=0;
volatile long millisFadeEnds=0; 

volatile byte toRed=0;
volatile byte toGreen=0;
volatile byte toBlue=0;
volatile byte fromRed=0;
volatile byte fromGreen=0;
volatile byte fromBlue=0;
volatile byte currentRed=0;
volatile byte currentGreen=0;
volatile byte currentBlue=0; // TODO move these to CRGB arrays?

volatile byte currentCycleSpeed=1; // interval of updates -- high is faster
volatile byte currentCycleColors=2; // number of received colors

byte color0[3]={
  0,0,0};
byte color1[3]={
  0,0,0};
byte color2[3]={
  0,0,0};
byte color3[3]={
  0,0,0};
byte color4[3]={
  0,0,0};
byte color5[3]={
  0,0,0};
byte color6[3]={
  0,0,0};
byte color7[3]={
  0,0,0};

byte *colors[8]={
  color0,color1,color2,color3,color4,color5,color6,color7}; 


#define MODE_OFF 0
#define MODE_ONE_COLOR 1
#define MODE_WIPE 2
#define MODE_CYCLE_FAST 3
#define MODE_CYCLE 4
#define MODE_FADE_INTO_CYCLE 5
#define MODE_CYCLE_SLOW 6
#define MODE_CHASE_SOFT 7
#define MODE_CHASE_HARD 8
#define MODE_SCAN 9
#define MODE_BLINK 10
#define MODE_PULSE 11
#define MODE_FADE 12
#define MODE_SET_ARRAY 13
#define MODE_FADE_INTO_SET_ARRAY 14
#define MODE_SET_ARRAY_PIXELS 15
#define MODE_FADE_INTO_SET_ARRAY_PIXELS 16


volatile byte currentMode=MODE_OFF;

CRGB currentColors[NUM_LEDS];
CRGB fromColors[NUM_LEDS];


void setup()
{ 

  wdt_disable(); // disable watchdog timer

  power_adc_disable();
  power_usart0_disable();

  getNameClient();
  getNameBase();

  pinMode(PIN_OUTPUT,OUTPUT); 
  pinMode(PIN_GROUND,OUTPUT);
  digitalWrite(PIN_GROUND,LOW);
  FastLED.addLeds<NEOPIXEL, PIN_OUTPUT>(currentColors, NUM_LEDS).setCorrection(CRGB(255,255,150));

  wakeRadio();
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

      delay(500);
      digitalWrite(13,LOW);
      FastLED.showColor(CRGB(0,0,0));

      delay(200); 
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

    byte theLength=strlen(theMessage);
    
    switch (theCommand) {

    case '$':
      HandleBuiltinMessage(theMessage);
      delay(500); 


      break;
    case 'c':

      if (theLength % 3 != 0 ) {
        break;
      }
      currentRed=fromRed=toRed=unpack(theMessage[0]);
      currentGreen=fromGreen=toGreen=unpack(theMessage[1]); 
      currentBlue=fromBlue=toBlue=unpack(theMessage[2]);

      currentMode=MODE_ONE_COLOR;
      secondsGotUpdate=secondsSinceStartup;

      break;
    case 'p':
      if (theLength % 3 != 0 ) {
        break;
      }

      toRed=unpack(theMessage[0]);
      toGreen=unpack(theMessage[1]); 
      toBlue=unpack(theMessage[2]);

      fromRed=unpack(theMessage[3]);
      fromGreen=unpack(theMessage[4]);
      fromBlue=unpack(theMessage[5]);
      secondsGotUpdate=secondsSinceStartup;


      currentMode=MODE_PULSE;

      break;
    case 'f':
      
      if (theLength % 3 != 0 ) {
        break;
      }

      toRed=unpack(theMessage[0]);
      toGreen=unpack(theMessage[1]); 
      toBlue=unpack(theMessage[2]);
      memmove(  &fromColors[0], &currentColors[0], NUM_LEDS * sizeof( CRGB) );
      currentMode=MODE_FADE;
      secondsGotUpdate=secondsSinceStartup;
      
      millisFadeStarted=millis();
      millisFadeEnds=millisFadeStarted+5000;

      break;
    case 'F':

      if (theLength % 3 != 0 ) {
        break;
      }

      toRed=unpack(theMessage[0]);
      toGreen=unpack(theMessage[1]); 
      toBlue=unpack(theMessage[2]);
      
      memmove(  &fromColors[0], &currentColors[0], NUM_LEDS * sizeof( CRGB) );

      currentMode=MODE_FADE;
      secondsGotUpdate=secondsSinceStartup;


      millisFadeStarted=millis();
      millisFadeEnds=millisFadeStarted+30000;

      break;


    case 'm':
      {
        char theDebug[8]="000";
        itoa(currentMode,theDebug,10);
        SendToBase(theDebug);
        
      }
      break;
    
    case 'i': // Idle color/animation
      if (secondsSinceStartup - secondsGotUpdate <= secondsIdleTime) {
        break;
      }

      // and continue...

    
    case 'a': // Setting the whole array
    case 'A': // Fade into setting the whole array
    case 'x': // Set the whole array with hard pixel boundaries
    case 'X': // Fade the whole array with hard pixel boundaries
    case 'C': // Color cycling
      if ((theLength - 1) % 3 != 0 ) {
        break;
      }


      if(theCommand!='i') {
        secondsGotUpdate=secondsSinceStartup;
      }


      {
        char theSpeed=theMessage[0];
        currentCycleSpeed=theSpeed - '0';
        if (currentCycleSpeed < 1 || currentCycleSpeed > 9) {
          currentCycleSpeed=1;
        }
        for (byte a=0;a*3<Payload-5;a++) {
          if ((theMessage[a*3+1]!='\0' && theMessage[a*3+1]!='\n' && theMessage[a*3+1]!='\r') &&
            (theMessage[a*3+2]!='\0' && theMessage[a*3+2]!='\n' && theMessage[a*3+2]!='\r') &&
            (theMessage[a*3+3]!='\0' && theMessage[a*3+3]!='\n' && theMessage[a*3+3]!='\r')
            ){
            colors[a][0]=unpack(theMessage[a*3+1]);
            colors[a][1]=unpack(theMessage[a*3+2]);
            colors[a][2]=unpack(theMessage[a*3+3]);
            currentCycleColors=a+1;
          } 
          else {
            break;
          }
        }
        toRed=colors[0][0];
        toGreen=colors[0][1];
        toBlue=colors[0][2];
        memmove(  &fromColors[0], &currentColors[0], NUM_LEDS * sizeof( CRGB) );

        millisFadeStarted=millis();
        millisFadeEnds=millisFadeStarted+(10-currentCycleSpeed)*250;

        if (theCommand=='C' || theCommand=='i') {

          currentMode=MODE_FADE_INTO_CYCLE;

        } 
        else if (theCommand=='A') {

          currentMode=MODE_FADE_INTO_SET_ARRAY;

        }

        else if (theCommand=='a') {

          currentMode=MODE_SET_ARRAY;

        }

        else if (theCommand=='X') {

          currentMode=MODE_FADE_INTO_SET_ARRAY_PIXELS;

        }

        else if (theCommand=='x') {

          currentMode=MODE_SET_ARRAY_PIXELS;

        }


      }

      break;
    }



  }

  switch(currentMode) {

  case MODE_OFF:
    currentRed=0;
    currentGreen=0;
    currentBlue=0;

    for (byte b=1;b<=NUM_LEDS;b++) {
      currentColors[b-1]=CRGB(currentRed,currentGreen,currentBlue);
    }
    FastLED.show();


    break;
  case MODE_ONE_COLOR:

    for (byte b=1;b<=NUM_LEDS;b++) {
      currentColors[b-1]=CRGB(currentRed,currentGreen,currentBlue);
    }
    FastLED.show();

    break;
  case MODE_PULSE:
    { 
      int position=millis() % 2000;
      if (position>=1000) {
        position=2000-position;
      }

      currentRed=map(position,0,1000,fromRed,toRed);
      currentGreen=map(position,0,1000,fromGreen,toGreen);
      currentBlue=map(position,0,1000,fromBlue,toBlue);

      for (byte b=1;b<=NUM_LEDS;b++) {
        currentColors[b-1]=CRGB(currentRed,currentGreen,currentBlue);
      }
      FastLED.show();



    }

    break;
  case MODE_FADE_INTO_SET_ARRAY_PIXELS:
  case MODE_FADE_INTO_SET_ARRAY:
  case MODE_FADE_INTO_CYCLE:
    {
      long thisMillis=millis(); 
      if (thisMillis < millisFadeEnds ) {

        for (byte b=0;b<NUM_LEDS;b++) {

          uint16_t thePosition;
          if (currentMode==MODE_FADE_INTO_CYCLE) {

            thePosition=( ( b * CYCLE_MULTIPLIER) % ( (currentCycleColors)*256 ) );
          } 
          else if (currentMode==MODE_FADE_INTO_SET_ARRAY) {
            
            thePosition=map(b,(NUM_LEDS-1)*0,(NUM_LEDS - 1)*1,0,(currentCycleColors+1)*256) ;
            thePosition=constrain(thePosition,256,currentCycleColors*256);
            thePosition-=256;
            
          } 
          else if (currentMode==MODE_FADE_INTO_SET_ARRAY_PIXELS) {

          

          }

          CRGB scratch;
          
          if (currentMode==MODE_FADE_INTO_SET_ARRAY_PIXELS) {
          
            scratch=getPixelAbsolute(b);
            
          } else {
            
            scratch=getCyclePosition(thePosition);
          
          }
          
          currentColors[b]=CRGB(map(thisMillis,millisFadeStarted,millisFadeEnds,fromColors[b].red,scratch.red),
          map(thisMillis,millisFadeStarted,millisFadeEnds,fromColors[b].green,scratch.green),
          map(thisMillis,millisFadeStarted,millisFadeEnds,fromColors[b].blue,scratch.blue));
        }


        FastLED.show();

      }      
      else {
        if (currentMode==MODE_FADE_INTO_CYCLE) {

          currentMode=MODE_CYCLE;
        } 
        else if (currentMode==MODE_FADE_INTO_SET_ARRAY) {

          currentMode=MODE_SET_ARRAY;

        }

        else if (currentMode==MODE_FADE_INTO_SET_ARRAY_PIXELS) {

          currentMode=MODE_SET_ARRAY_PIXELS;

        }
      }
    }
    break;

  case MODE_SET_ARRAY:

    {
      
      for (byte b=0;b<NUM_LEDS;b++) { 
        uint16_t thePosition=map(b,(NUM_LEDS-1)*0,(NUM_LEDS - 1)*1,0,(currentCycleColors+1)*256) ;
        thePosition=constrain(thePosition,256,currentCycleColors*256);
        thePosition-=256;
        currentColors[b]=getCyclePosition(thePosition);
      }
      FastLED.show(); 

    }
    break;

  case MODE_SET_ARRAY_PIXELS:

    { 

      for (byte c=0;c<NUM_LEDS;c++) {
        
        currentColors[c]=getPixelAbsolute(c);
        
      }
      
      FastLED.show();  

    }
    break;


  case MODE_FADE:
    {
      long thisMillis=millis(); 
      if (thisMillis < millisFadeEnds ) {

        for (byte b=0;b<NUM_LEDS;b++) {
          currentColors[b]=CRGB(map(thisMillis,millisFadeStarted,millisFadeEnds,fromColors[b].red,toRed),
          map(thisMillis,millisFadeStarted,millisFadeEnds,fromColors[b].green,toGreen),
          map(thisMillis,millisFadeStarted,millisFadeEnds,fromColors[b].blue,toBlue));


        }
        FastLED.show();



      } 
      else {
        for (byte b=0;b<NUM_LEDS;b++) {
          currentColors[b]=CRGB(toRed,toGreen,toBlue);
        }

        fromRed=currentRed=toRed;
        fromGreen=currentGreen=toGreen;
        fromBlue=currentBlue=toBlue;  
        currentMode=MODE_ONE_COLOR; 

      }

    }

    break;
  case MODE_CYCLE:


    {
      for (byte b=0;b<NUM_LEDS;b++) {
        currentColors[b]=CRGB(colors[b][0],colors[b][1],colors[b][2]);

        uint16_t thePosition=( ( millis() - millisFadeEnds ) / (20-currentCycleSpeed*2) + b*CYCLE_MULTIPLIER) % ( (currentCycleColors)*256 );

        currentColors[b]=getCyclePosition(thePosition);
        // currentColors[b]=getCyclePosition(b*256);

      }
      FastLED.show();


    }



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


CRGB getCyclePosition(uint16_t theCount) {
  byte theIndex=theCount/256;
  byte thePosition=theCount % 256;
  byte theSecondIndex=theIndex+1;
  if (theSecondIndex>=currentCycleColors-1) {
    theSecondIndex=theSecondIndex % (currentCycleColors);
  }

  return CRGB(
  superlerp8by8(colors[theIndex][0],colors[theSecondIndex][0],thePosition),
  superlerp8by8(colors[theIndex][1],colors[theSecondIndex][1],thePosition),
  superlerp8by8(colors[theIndex][2],colors[theSecondIndex][2],thePosition)
    );

}

CRGB getPixelPosition(uint16_t theCount) {
  
  byte theIndex=theCount/256;
  byte thePosition=theCount % 256;
  byte theSecondIndex=theIndex+1;
  if (theSecondIndex>=currentCycleColors-1) {
    theSecondIndex=theSecondIndex % (currentCycleColors);
  }
  return CRGB(colors[theIndex][0],colors[theIndex][1],colors[theIndex][2]);
}


CRGB getPixelAbsolute(byte thePixel) {
   
  long theIndex=map(thePixel,0,NUM_LEDS,0,currentCycleColors);
 
 /* char theDebug[8]="000";
  itoa(colors[theIndex][0],theDebug,10);
  SendToBase(theDebug);
  */
  
  // return CRGB(thePixel,thePixel,thePixel);
  //  return CRGB(theIndex,theIndex,theIndex);
  return CRGB(colors[theIndex][0],colors[theIndex][1],colors[theIndex][2]);
  
}


byte unpack(byte theByte) {
  theByte -= 32;
  theByte = theByte << 2;
  return theByte;  
}





// bringing this in because the current version in FastLED croaks when b > a


// A note on the structure of the lerp functions:
// The cases for b>a and b<=a are handled separately for
// speed: without knowing the relative order of a and b,
// the value (a-b) might be overflow the width of a or b,
// and have to be promoted to a wider, slower type.
// To avoid that, we separate the two cases, and are able
// to do all the math in the same width as the arguments,
// which is much faster and smaller on AVR.

// linear interpolation between two unsigned 8-bit values,
// with 8-bit fraction
LIB8STATIC uint8_t superlerp8by8( uint8_t a, uint8_t b, fract8 frac)
{
  uint8_t result;
  if( b > a) {
    uint8_t delta = b - a;
    uint8_t scaled = scale8( delta, frac);
    result = a + scaled;
  } 
  else {
    uint8_t delta = a - b;
    uint8_t scaled = scale8( delta, frac);
    result = a - scaled;
  }
  return result;
}














