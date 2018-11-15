#include <Adafruit_BMP280.h>
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>

#define NMEA_TALKER_ID "WI" // Weather Instruments
#define NMEA_DELAY 10 // Send data every 10 seconds

#define I2C_ADDR          0x3F        //Define I2C Address where the PCF8574A is

#define BACKLIGHT_PIN      3
#define En_pin             2
#define Rw_pin             1
#define Rs_pin             0
#define D4_pin             4
#define D5_pin             5
#define D6_pin             6
#define D7_pin             7

//Initialise the LCD
LiquidCrystal_I2C      lcd(I2C_ADDR, En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

// meters above mean sea level
#define ALTITUDE 1
// How many samples to take per iteration
#define PRESS_OVERSAMPLING 3

//create a new Adafruit_BMP280 class object called pressure.
Adafruit_BMP280 pressure;

const byte buff_len = 90;
char CRCbuffer[buff_len];

byte nmea_crc(String msg) {
  // NMEA CRC: XOR each byte with previous for all chars between '$' and '*'
  char c;
  int i;
  byte crc = 0;
  for (i = 0; i < buff_len; i++) {
    crc ^= msg.charAt(i); // XOR
  }
  return crc;
}

void setup() {
  // put your setup code here, to run once:

  Serial.begin(4800);

  if (!pressure.begin(0x76) ) {
    nmea_txt("BMP280 init fail");
    exit(1);
  }
  nmea_txt("Nanobaro ready.");
    //Define the LCD as 16 column by 2 rows 
    lcd.begin (16,2);
    
    //Switch on the backlight
    lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
    lcd.setBacklight(HIGH);
 
}

void nmea_send(String sentence, String params) {
  String msg = String(NMEA_TALKER_ID) + sentence + params;

  msg.toCharArray(CRCbuffer, sizeof(CRCbuffer)); // put complete string into CRCbuffer
  int crc = nmea_crc(CRCbuffer);

  if (crc < 16) msg += "0"; // pad with leading 0
  String hexcrc = String(crc, HEX);
  hexcrc.toUpperCase();
  Serial.println("$" + msg + "*" + hexcrc);
}

void nmea_txt(String text) {
  nmea_send("TXT", ",01,01,01," + text);
}


void loop() {
  String s;
  float temp, press;
    
  temp = pressure.readTemperature();
  press = pressure.readPressure();
  
      s = "";
      s = ",C," + String(temp) + ",C,TEMP";
      s += ",P," + String(press / 100000.0, 5) + ",B,BARO"; // OpenCPN doesn't seem to grok Pascal (unit "P"), only Bar ("B")
      nmea_send("XDR", s);
      // These NMEA 0183 sentences are deprecated:
      //nmea_send("MTA", "," + String(temp) + ",C");
      //nmea_send("MMB", ",0.0,I,1.5,B");
      int mb = round(press /100);
      String outStr = String(mb) + "mb " + String(temp) +"C";
      lcd.setCursor(0,0);
    lcd.print(outStr);
  
  delay(NMEA_DELAY * 1000);
}
