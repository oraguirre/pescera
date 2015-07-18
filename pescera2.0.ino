// This #include statement was automatically added by the Spark IDE.
#include "TM1637Display/TM1637Display.h"
#include "solarcalc/solarcalc.h"
#include "math.h"
  
  float latitude = -34.9290;
  float longitude = 138.6010;
  float zenith = 90.833;
  float timezone = 9.5;
  //int brightness=0;
 
  const int PinCLK 	    =   D0;
  const int PinDIO 	    =   D1;
  const int PinWhite    =   A7;
  
  int brightnessW = 0;
  int brightnessB = 0;
  
  byte segto;
  byte hour00, hour01;
 
  solarcalc solardata(latitude, longitude, zenith, timezone);
 
  TM1637Display display(PinCLK, PinDIO);
  
  void displaytime() {
	// display current time in TM1637Display
    display.setBrightness(0x0a);
    hour00 = floor(Time.hour() / 10);
    display.showNumberDec(hour00, true, 1, 0);
    hour01 = floor(Time.hour() % 10);
    segto = 0x80  | display.encodeDigit(hour01);
    display.setSegments(&segto, 1, 1);
    display.showNumberDec(Time.minute(), true, 2, 2);  
  }
  
  int fadein(int led, int brightness) {
      int result;
      if (brightness<255) {
          for (brightness = 0; brightness <= 255; brightness += 1) {
          // sets the value (range from 0 to 255):
          analogWrite(led, brightness);
          // wait for 10 milliseconds to see the dimming effect
          delay(10);
          }
      }
      result=brightness;
      return result;
  }    

  int fadeout(int led, int brightness) {    
      int result;
      if (brightness>0) {
          for (brightness = 255; brightness >= 0; brightness -= 1) {
          // sets the value (range from 0 to 255):
          analogWrite(led, brightness);
          // wait for 10 milliseconds to see the dimming effect
              if (brightness>170 && brightness<=255){
                  delay(5);
              }
              else if (brightness>85 && brightness<=170){
                  delay(10);
              }
              else if (brightness<=85){
                  delay(20);
              }
          }
     }
    result=brightness;
    return result;    
  }
 
  void setup() {
		Serial.begin(9600);
		Spark.syncTime();
		Time.zone(timezone);
  }
  void loop() { 
	  solardata.time(Time.year(), Time.month(), Time.day(), Time.hour(), Time.minute(), Time.second());
	  solardata.calculations();
      displaytime();
      brightnessW=fadein(PinWhite, brightnessW);
	  brightnessW=fadeout(PinWhite, brightnessW);
	  delay(1000);
}
