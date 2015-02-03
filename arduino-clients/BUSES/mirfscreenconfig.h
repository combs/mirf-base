
// These are default values that will be overruled by EEPROM.

char nameBase[6] = "BASES";

char nameClient[6] = "BUSES";

#define LCD_I2C
// #define LCD_PCB

#define LCD_BACKLIGHT_PIN 10

#define MIRF_PINS_STANDARD
//#define MIRF_PINS_PCB

#define LCD_CLEAR_ON_SLEEP
//#define LCD_UPDATES_EXTEND_ON_TIME


const long secondsScreenAwakeTime=118;
const long secondsTimeout=10;
const long secondsMaxDataAge=30L;

// obsolete:

const long msOnTime=118L*1000;
const long msTimeout=5000;
const long msMaxDataAge=secondsMaxDataAge*1000L;



