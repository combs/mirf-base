#include <SPI.h>
#include <Mirf.h>
#include <Wire.h>


// #include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>

//#define LCD_I2C
//#include "LiquidCrystal_I2C.h"

#define LCD_BACKLIGHT_PIN 10

#include "LiquidCrystal.h"

#ifdef LCD_I2C

// #include "LiquidCrystal_I2C.h"
#else
 // #include "LiquidCrystal.h"

#endif


String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
volatile byte updateRequested = false;
volatile byte paintRequested = false;
volatile long msTurnedOn = 0;
const long msOnTime=30*1000;
const long msTimeout=5000;
const long msMaxDataAge=30L*60L*1000;
const long msScreenUpdateInterval=1000;

long msScreenUpdated=0;

volatile long msGotForecast = 0;
volatile long msRequestedForecast = 0;
const byte Payload = 32;
char stringForecast0[Payload];
char stringForecast1[Payload];
char stringForecastNow[Payload];
char stringForecastLater[Payload];
char stringForecastConditions[Payload];
char stringBuffer[Payload];

#ifdef LCD_I2C

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); 
#else
LiquidCrystal lcd(7, 6, A3, A2, A1, A0);

#endif
//LiquidCrystal_I2C_simple lcd(0x27,16,2); 


void setup()
{

  pinMode(2,INPUT);

  //  Serial.begin(115200);
  inputString.reserve(200);
  lcd.begin(16, 2);
  #ifdef LCD_I2C
    lcd.on();
  #endif

  delay(2);
  lcd.clear();
  lcd.print("Starting radio..");
  Mirf.spi = &MirfHardwareSpi;
  Mirf.init();

  // name the receiving channel - must match tranmitter setting!
  Mirf.setRADDR((byte *)"WCLKK");


  Mirf.payload=Payload;

  Mirf.channel = 10;
  

  // configure 15 retries, 500us between attempts
  Mirf.configRegister(SETUP_RETR,(B0001<<ARD ) | (B1111<<ARC));

  // now config the device.... 
  Mirf.config();  

  // Set 1MHz data rate - this increases the range slightly
  Mirf.configRegister(RF_SETUP,0x06);

  Mirf.setTADDR((byte *)"BASES");
  Mirf.send((byte *)"WCLKKStarted");
  //  Serial.print("booted with ");
  //  Serial.println(Mirf.getStatus());

  blockForSend();


  byte status=Mirf.getStatus();
  if (status==14) {
    //    lcd.clear();
    //   lcd.println("");
    //  lcd.println("");

    myLCDClear();
    delay(5);
    lcd.print("Radio started.");
    lcd.setCursor(0,1); 
    lcd.print("Contacting base"); 
  } 
  else {
    lcd.clear();

    lcd.print("Radio error: ");
    lcd.setCursor(0,1);
    lcd.print(status);
  }

  attachInterrupt(0,flagUpdate,RISING);
  flagUpdate();
  //updateRequested=true;


}

void myLCDClear() {
  lcd.home();
  lcd.write("                ");
  lcd.setCursor(0,1);
  lcd.write("                ");
  lcd.home();
}
void flagUpdate() {
  updateRequested=true;
  msTurnedOn=millis();
  //  lcd.on();
  //  paintScreen();
  paintRequested=true;

}


void loop()
{

  if (paintRequested==true) {
    #ifdef LCD_I2C
    lcd.on();
    #endif
    paintScreen();
    paintRequested=false;  
  }

  //  digitalWrite(13,updateRequested);
  /*
  Serial.println("Req, Got, On:");
   Serial.println(msRequestedForecast);
   Serial.println(msGotForecast);
   
   Serial.println(msTurnedOn);
   */

  byte data[Payload];


  if (updateRequested==true) {

    updateRequested=false;
    if (millis() - msGotForecast > msMaxDataAge || msGotForecast==0) {
      // myLCDClear(); 
      // lcd.print("Requesting...");
      lcd.setCursor(strlen(stringForecastNow),0);
      //lcd.cursor();
      lcd.blink();
      msRequestedForecast=millis();
      inputString="BASESWCLKKupdate";
      send2824Message(inputString);
      blockForSend();

    } 
    else {

      // paintScreen();

    }

  }
  if ((millis() - msRequestedForecast > msTimeout ) && (millis() - msRequestedForecast < msTimeout+msTimeout) && (millis() - msGotForecast > msTimeout )) {
    updateRequested=true;
    //timeout
  }

  // is there any data pending? 
  if( Mirf.dataReady() )
  {
    if (millis() - msRequestedForecast < msTimeout) {
      msGotForecast = millis();
      // Let's lock out unrelated status updates from the base station.

    }

    msTurnedOn=millis();
    // lcd.noCursor();
    lcd.noBlink();
    Mirf.getData((byte *) &data);
    String thePayload=String((char *)data);
    char theLine=thePayload[5];
    char theMessage[Payload];
    thePayload.substring(6).toCharArray(theMessage,Payload);
    for (byte a=0;a<Payload;a++) {
      if (theMessage[a]=='\r' || theMessage[a]=='\n'){
        theMessage[a]='\0';
      }
    }
    byte x=0;
    byte y=0;
    byte length=strlen(theMessage);
    switch (theLine) {
    case '0':
      strcpy(stringForecast0,theMessage);
      x=0;
      y=0;
      length=16;
      break;

    case '1':
      strcpy(stringForecast1,theMessage);
      x=0;
      y=1;
      length=16;
      break;
    case 'C':
      strcpy(stringForecastConditions,theMessage);
      x=0;
      y=1;
      break;
    case 'N':
      strcpy(stringForecastNow,theMessage);
      x=0;
      y=0;
      break;
    case 'L':
      strcpy(stringForecastLater,theMessage);
      x=16-strlen(stringForecastLater);
      y=0;
      break;
    case 'A':
      x=30;
      y=4;
      length=0;
      theMessage[0]='\0';
      msRequestedForecast=millis();
      
    }

    lcd.setCursor(x,y);


    for (int a=0;a<length;a++){
      lcd.write(" ");
    }

    //    lcd.write("                ");

    lcd.setCursor(x,y);
    //    lcd.write(theLine);
    lcd.print( theMessage);


    //        thePayload.substring(5,1).toCharArray(theFrom,6);

    // ... and write it out to the PC
    //   Serial.println((char *) data);
  }




  delay(250);
  if (millis() - msTurnedOn > msOnTime ) {
    #ifdef LCD_I2C
    lcd.off();
#endif

  } 
  else {
        #ifdef LCD_I2C

    lcd.on();
    #endif
    
    //    paintScreen();

  }



}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n' || inChar == '\r') {
      stringComplete = true;
    } 
  }
}

void send2824Message(String theMessage) {
  char theTarget[6];
  char thePayload[Payload];
  char theSource[6];
  theMessage.substring(0,5).toCharArray(theTarget,6);
  theMessage.substring(5).toCharArray(thePayload,Payload);
  //   Serial.println(inputString);
  theMessage.substring(5,5).toCharArray(theSource,6);


  Mirf.setTADDR((byte *)theTarget);

  //    Mirf.setRADDR((byte *)theSource);
  // do we want this? malformed message could make this node unreachable 

  Mirf.config();
  Mirf.send((byte *)thePayload);

}

void blockForSend() {

  while( Mirf.isSending() )
  {
    delay(1);
  }
}


void paintScreen() {

  lcd.setCursor(0,0);
  if (strlen(stringForecastNow) > 1) {
    myLCDClear();
    lcd.print( stringForecastNow);
    //    lcd.print(strlen(stringForecastNow));
    lcd.setCursor(16-(strlen(stringForecastLater)),0);
    lcd.print( stringForecastLater);
  } 
  else if (strlen(stringForecast0) > 1 ) {
    myLCDClear();
    lcd.print(stringForecast0);
  }
  lcd.setCursor(0,1);  

  if (strlen(stringForecastConditions) > 1) {

    lcd.print( stringForecastConditions); 
  }
  else if (strlen(stringForecast0) > 1 ) {

    lcd.print(stringForecast1);
  }

}











