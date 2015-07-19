// This #include statement was automatically added by the Spark IDE.
#include "TM1637Display/TM1637Display.h"
#include "solarcalc/solarcalc.h"
#include "math.h"
  
  float latitude = -34.9290;
  float longitude = 138.6010;
  float zenith = 90.833;
  float timezone = 9.5;
  //int brightness=0;

  const int PinAutoLed 	    =   D0;
  const int PinSensor	    =   D1;
  const int PinDIO 	        =   D2;
  const int PinCLK 	        =   D3;
  const int PinSwitch 	    =   D6; 
  const int PinSensorLed    =   D7; 

  const int PinWhiteLed 	=   A0;
  const int PinBlueLed 	    =   A1;
  
  int brightnessW;
  int brightnessB;

  int buttonState = 0;        // current state of the button
  int lastButtonState = 0;
  int buttonFunction;
  int AutoLedState;  
  boolean wLed, bLed;
  byte midnight = 22;
  
  int sensorState = 0;
  bool sensor_triggered = false;
  int count = 0;
  int sensor_timeoff = 60;
  
  byte segto;
  byte hour00, hour01;
 
  solarcalc solardata(latitude, longitude, zenith, timezone);
 
  TM1637Display display(PinCLK, PinDIO);
  
  // Turn on or off led using time limits
  boolean LedControl(float time, float time_on, float time_off) {
  boolean result;
  if (time_off > time_on) {
    if (time >= time_on && time <= time_off) {
      result = true;
    }
    else {
      result = false;
    }
  }
  else {
    if (time >= time_on || time <= time_off) {
      result = true;
    }
    else {
      result = false;
    }
  }
  return result;
}
  
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
  
void FadeInWhite() {
  if (brightnessW < 255) {// fade in from min to max in increments of 5 points:
    for (brightnessW = 0; brightnessW <= 255; brightnessW += 1) {
      // sets the value (range from 0 to 255):
      analogWrite(PinWhiteLed, brightnessW);
      // wait for 10 milliseconds to see the dimming effect
      delay(10);
    }
  }
}

void FadeInBlue() {
  if (brightnessB < 255) {// fade in from min to max in increments of 5 points:
    for (brightnessB = 0; brightnessB <= 255; brightnessB += 1) {
      // sets the value (range from 0 to 255):
      analogWrite(PinBlueLed, brightnessB);
      // wait for 10 milliseconds to see the dimming effect
      delay(10);
    }
  }
}

void FadeOutWhite() {
  if (brightnessW > 0) { // fade out from max to min in increments of 5 points:
    for (brightnessW = 255; brightnessW >= 0; brightnessW -= 1) {
        // sets the value (range from 0 to 255):
        analogWrite(PinWhiteLed, brightnessW);
        if (brightnessW>170 && brightnessW<=255){
          delay(5);
        }
        else if (brightnessW>85 && brightnessW<=170){
          delay(10);
        }
        else if (brightnessW<=85){
          delay(20);
        }
    }
  }
}

void FadeOutBlue() {
  if (brightnessB > 0) { // fade out from max to min in increments of 5 points:
    for (brightnessB = 255; brightnessB >= 0; brightnessB -= 1) {
      // sets the value (range from 0 to 255):
      analogWrite(PinBlueLed, brightnessB);
            if (brightnessB>170 && brightnessB<=255){
              delay(5);
            }
            else if (brightnessB>85 && brightnessB<=170){
              delay(10);
            }
            else if (brightnessB<=85){
            delay(20);
            }
    }
  }
}
 
  void setup() {

        pinMode(PinWhiteLed, OUTPUT);
        pinMode(PinBlueLed, OUTPUT);
        pinMode(PinAutoLed, OUTPUT);
        pinMode(PinSwitch, INPUT);
        pinMode(PinSensor, INPUT);
        pinMode(PinSensorLed, OUTPUT);
		Serial.begin(9600);
		Spark.syncTime();
		Time.zone(timezone);
		
		brightnessW = 0;
        brightnessB = 0;
        FadeInWhite();
        FadeOutWhite();
        delay(500);
        FadeInBlue();
        FadeOutBlue();
        delay(500);
		brightnessW = 0;
        brightnessB = 0;        
        
  }
  void loop() { 
	  solardata.time(Time.year(), Time.month(), Time.day(), Time.hour(), Time.minute(), Time.second());
	  solardata.calculations();
      displaytime();
  buttonState = digitalRead(PinSwitch);
  if (buttonState != lastButtonState) {
    if (buttonState == HIGH) {
      buttonFunction++;
      if (buttonFunction > 3) {
        buttonFunction = 0;
      }
    }
  }
  lastButtonState = buttonState;

  sensorState = digitalRead(PinSensor);
  digitalWrite(PinSensorLed, sensorState);

  switch (buttonFunction) {
    case 0:
      wLed = LedControl(solardata.current_time(), solardata.sunset_time(), midnight);
      bLed = LedControl(solardata.current_time(), midnight, solardata.sunrise_time());

      // turn on white led and turn off blue led
      if ((wLed == true) && (bLed == false)) {
        FadeInWhite();
        FadeOutBlue();
      }

      // turn on blue led and turn off white led (motion sensor dependant)
      if ((wLed == false) && (bLed == true)) {
        if (sensorState == HIGH) {
          sensor_triggered = HIGH;
          count = 0;
        }
        if (sensor_triggered == HIGH) {
          FadeInWhite();
          FadeOutBlue();
          count++;
          if (count > sensor_timeoff) {
            count = 0;
            sensor_triggered = LOW;
          }
        }
        else {
          FadeInBlue();
          FadeOutWhite();
        }
      }

      // Turn off all leds
      if ((wLed == false) && (bLed == false)) {
        FadeOutBlue();
        FadeOutWhite();
      }
      digitalWrite(PinAutoLed, HIGH);
      break;
    case 1:
      // White Light on
      FadeInWhite();
      FadeOutBlue();
      digitalWrite(PinAutoLed, LOW);
      break;
    case 2:
      // Blue Light on
      FadeInBlue();
      FadeOutWhite();
      digitalWrite(PinAutoLed, LOW);
      break;
    case 3:
      // Non state selected
      FadeOutBlue();
      FadeOutWhite();
      AutoLedState = !AutoLedState;
      digitalWrite(PinAutoLed, AutoLedState);
      count = 0;
      sensor_triggered = LOW;
      break;
    default:
      // default off
      analogWrite(PinWhiteLed, 0);
      analogWrite(PinBlueLed, 0);
  }
  
  delay(1000);
}
