// This #include statement was automatically added by the Spark IDE.
#include "Adafruit_DHT/Adafruit_DHT.h"
#include "TM1637Display/TM1637Display.h"
#include "solarcalc/solarcalc.h"
#include "math.h"

  #define DHTTYPE DHT22
  
  float latitude = -34.9290;
  float longitude = 138.6010;
  float zenith = 90.833;
  float timezone = 9.5;
  float humidity;
  float temperature;
  
  String myCharhumid;
  String myChartemp;
  //int brightness=0;

  const int PinAutoLed 	    =   D0;
  const int PinSensor	    =   D1;
  const int PinDIO 	        =   D2;
  const int PinCLK 	        =   D3;
  const int PinDHT          =   D4;
  const int PinSwitch 	    =   D6; 
  const int PinSensorLed    =   D7;

  const int PinWhiteLed 	=   A0;
  const int PinBlueLed 	    =   A1;
  
  const int delayTime       =   2000;
  
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
  
  int DisplayScroll;
  int publishcount;
  
  byte segto;
  byte Digit0, Digit1, Digit2, Digit3;
 
  solarcalc solardata(latitude, longitude, zenith, timezone);
 
  TM1637Display display(PinCLK, PinDIO);
  
  DHT dht(PinDHT, DHTTYPE);
  
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
//*****************************************
//  Display
//*****************************************
  
  void displaysegments() {
	// Set Brightness
    display.setBrightness(0x0a);

    // XGFEDCBA
    //0b00111111,    // 0
    
    //      A
    //     ---
    //  F |   | B
    //     -G-
    //  E |   | C
    //     ---
    //      D

    switch (DisplayScroll) {
      case 0:    
        // Get Time
        Digit0 = floor(Time.hour() / 10);
        Digit1 = floor(Time.hour() % 10);
        Digit2 = floor(Time.minute() / 10);
        Digit3 = floor(Time.minute() % 10);
        break;
      case 1:
        // Get Temperature
        Digit0 = floor(int(temperature) / 10);
        Digit1 = floor(int(temperature) % 10);
        Digit2 = 0b01100011; //degrees
        Digit3 = 0b00111001; // C
        break;
      case 2:
        // Get Humidity
        Digit0 = floor(int(humidity) / 10);
        Digit1 = floor(int(humidity) % 10);
        Digit2 = 0b01100011; //degrees
        Digit3 = 0b01011100; // o
        break;
      default:
        Digit0 = 0b01000000; // -
        Digit1 = 0b01000000; // -
        Digit2 = 0b01000000; // -
        Digit3 = 0b01000000; // -
        break;
    }
    
    // Set digit 0 
    display.showNumberDec(Digit0, true, 1, 0);    

    if (DisplayScroll>0) {
      display.showNumberDec(Digit1, true, 1, 1);
      //Set Digit 2
      display.setSegments(&Digit2, 1, 2);
      //Set Digit 3
      display.setSegments(&Digit3, 1, 3);        
    }
    else {
      //Set digit 1
      segto = 0x80  | display.encodeDigit(Digit1);
      display.setSegments(&segto, 1, 1);
      //Set Digit 2
      display.showNumberDec(Digit2, true, 1, 2);
      //Set Digit 3
      display.showNumberDec(Digit3, true, 1, 3);
    }
    // Go to next case
    DisplayScroll++;
    if (DisplayScroll>2) {
      DisplayScroll=0;
    }
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

int lightcmd(String command) {
    
    if (command=="0") {
        buttonFunction=0;
        return 0;
    }
    else if (command=="1") {
        buttonFunction=1;
        return 1;
    }
    else if (command=="2") {
        buttonFunction=2;
        return 2;
    }
    else if (command=="3") {
        buttonFunction=3;
        return 3;
    }
    else {
        return -1;
    }
}

//*****************************************
// SETUP
//*****************************************
 
  void setup() {

        Spark.variable("lightstate",&buttonFunction,INT);
        Spark.function("lightcmd",lightcmd);

        pinMode(PinWhiteLed, OUTPUT);
        pinMode(PinBlueLed, OUTPUT);
        pinMode(PinAutoLed, OUTPUT);
        pinMode(PinSwitch, INPUT);
        pinMode(PinSensor, INPUT);
        pinMode(PinSensorLed, OUTPUT);
        
		Serial.begin(9600);
        dht.begin();
        
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
  
//*****************************************
// LOOP
//*****************************************  
  
  void loop() { 

	solardata.time(Time.year(), Time.month(), Time.day(), Time.hour(), Time.minute(), Time.second());
	solardata.calculations();

 	humidity = dht.getHumidity();
	temperature = dht.getTempCelcius();

    //publish every 10 seconds
    if (publishcount > 30) {
 	  Spark.publish("humidity",String(int(humidity)),60,PRIVATE); 
	  Spark.publish("temperature",String(int(temperature)),60,PRIVATE);
	  publishcount=0;
    }
    publishcount++;
    
    delay(delayTime);
    
    displaysegments();
    
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
          Spark.publish("Sensor","ON",60,PRIVATE);
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
}
