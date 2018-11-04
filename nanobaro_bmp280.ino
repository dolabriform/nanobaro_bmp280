#include <Adafruit_BMP280.h>
#include <Wire.h>

#define NMEA_TALKER_ID "WI" // Weather Instruments
#define NMEA_DELAY 10 // Send data every 10 seconds

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

  if (!pressure.begin() ) {
    nmea_txt("BMP280 init fail");
    exit(1);
  }
  nmea_txt("Nanobaro ready.");
 
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
      s += ",P," + String(press / 1000.0, 5) + ",B,BARO"; // OpenCPN doesn't seem to grok Pascal (unit "P"), only Bar ("B")
      nmea_send("XDR", s);
      // These NMEA 0183 sentences are deprecated:
      //nmea_send("MTA", "," + String(temp) + ",C");
      //nmea_send("MMB", ",0.0,I,1.5,B");
    
  
  delay(NMEA_DELAY * 1000);
}
