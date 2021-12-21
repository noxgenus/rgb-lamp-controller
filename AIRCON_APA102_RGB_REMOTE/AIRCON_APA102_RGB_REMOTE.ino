#include <Adafruit_DotStar.h>
#include <SoftwareSerial.h>
#include <DualFunctionButton.h>
#include <Encoder.h>
#include <EEPROM.h>
#include <eepromtheshit.h>
#include <YesNoIO_rgbl.h>

#define NUMPIXELSRECEIVER 21
#define DEBOUNCE_INTERVAL 500L // Milliseconds
#define BUTTONLED_DELAY 1000L
#define DIAL_DELAY 2000L

#define NUMPIXELS 1
#define DATAPIN    8
#define CLOCKPIN   7
Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);

// COLOR SETS

uint32_t colorwhite = 0x111111;
uint32_t colorred = 0xb40101;
uint32_t colorredoff = 0x300000;
uint32_t colorblue = 0x0000FF;
uint32_t colorblueoff = 0x000039;
uint32_t coloryellow = 0xFFFF00;
uint32_t coloryellowoff = 0x302d00;
uint32_t colorgreen = 0x00dc1f;
uint32_t colorgreenoff = 0x038715;
uint32_t coloroff = 0x000000;
uint32_t off = 0x000000;

int bright = 200;
int ledNumbersDialIndicator;

SoftwareSerial SSerial(4, 5); // RX, TX HC12
Encoder knob(2, 3);

const int pinButton = 6; 
long positionEnc  = -999;
int buttonState1 = 0;
int buttonState2 = 0;
long newEnc;
int modeState = 0;
int dialState = 0;
int saveLampEeprom = 0;
int battLow = 0;
static int battPin = 9;

unsigned long lastStartStopTime = 0;

unsigned long buttonLedDelayStart1 = 0;
unsigned long buttonLedDelayStart2 = 0;
unsigned long remoteSaveDelay = 0;
unsigned long dialDelayStart = 0;
unsigned long now2 = millis();  // NOW2 FOR BUTTONS DEBOUNCE


// DATATYPES TO INCLUDE IN EEPROM
struct config_t{
  int r = 250;
  int g = 150;
  int b = 80;
  int a = 250;
  int ledNumbersDial = 0;
  int modeState = 0;
} int2eeprom;

DualFunctionButton button(pinButton, 2000, INPUT_PULLUP);


void setup() {
  
  Serial.begin(9600);
  SSerial.begin(9600);
  
  strip.setBrightness(bright);
  strip.begin();
  strip.show();

  strip.setPixelColor(0, coloryellow);
  strip.show();

  pinMode(battPin, INPUT_PULLUP);
  
  
  Serial.println("Aircon SENDER Loaded!");

  load(); // LOAD DATATYPE VALUES FROM EEPROM
  delay(500);
}

void loop(){
  
now2 = millis();


  if (button.shortPress()) { // change mode (0=#ofleds 1=R 2=G 3=B)
      modeState ++;
      buttonState1 = 1;
        if (modeState > 3){
          modeState = 0;
        }
        Serial.print("Mode: ");
        Serial.println(modeState);
        buttonLedDelayStart1 = now2;
  }
  if (button.longPress()) { // save to eeprom
      buttonState2 = 1;
      save();
      saveLampEeprom = 1;
      Serial.println("Saved settings to eeprom!");
      buttonLedDelayStart2 = now2;
      remoteSaveDelay = now2;
  }

  // reset remote save state
  if ((remoteSaveDelay + (DEBOUNCE_INTERVAL)) < now2) {
      saveLampEeprom = 0;
  }
  
  // INDICATION LEDS OFF DELAY
  if ((buttonLedDelayStart1 + (BUTTONLED_DELAY)) < now2) {
      buttonState1 = 0;
  }

  if ((buttonLedDelayStart2 + (BUTTONLED_DELAY)) < now2) {
      buttonState2 = 0;
  }


  newEnc = getEncPosition();

  if (newEnc != positionEnc) {
    // -------------- ADD -------------------
      if (newEnc > positionEnc){
        // add mode and value here ++
        if (modeState == 0){
          int2eeprom.ledNumbersDial++;
          if (int2eeprom.ledNumbersDial > NUMPIXELSRECEIVER){
            int2eeprom.ledNumbersDial = NUMPIXELSRECEIVER;
          }
        } else if (modeState == 1){
          int2eeprom.r ++;
          if (int2eeprom.r > 255){
            int2eeprom.r = 255;
          }
        } else if (modeState == 2){
          int2eeprom.g ++;
          if (int2eeprom.g > 255){
            int2eeprom.g = 255;
          }
        } else if (modeState == 3){
          int2eeprom.b ++;
          if (int2eeprom.b > 255){
            int2eeprom.b = 255;
          }
        }
     // ------------- SUBTRACT ---------------
      } else if (newEnc < positionEnc){
        // add mode and value here --
          if (modeState == 0){
          int2eeprom.ledNumbersDial--;
          if (int2eeprom.ledNumbersDial < 1){
            int2eeprom.ledNumbersDial = 1;
          }
        } else if (modeState == 1){
          int2eeprom.r --;
          if (int2eeprom.r < 1){
            int2eeprom.r = 1;
          }
        } else if (modeState == 2){
          int2eeprom.g --;
          if (int2eeprom.g < 1){
            int2eeprom.g = 1;
          }
        } else if (modeState == 3){
          int2eeprom.b --;
          if (int2eeprom.b < 1){
            int2eeprom.b = 1;
          }
        }
      }
      
    positionEnc = newEnc;
    dialState = 1;
    dialDelayStart = now2;
    
//    Serial.print("NUM: ");
//    Serial.print(int2eeprom.ledNumbersDial);
//    Serial.print(" R: ");
//    Serial.print(int2eeprom.r);
//    Serial.print(" G: ");
//    Serial.print(int2eeprom.g);
//    Serial.print(" B: ");
//    Serial.print(int2eeprom.b);
//    Serial.print(" ENC: ");
//    Serial.println(positionEnc);
    
  }


  if ((dialDelayStart + (DIAL_DELAY)) < now2) {
      dialState = 0;
  }

  if (modeState == 0){
        ledNumbersDialIndicator = map(int2eeprom.ledNumbersDial, 0, 21, 20, 250);
        strip.setPixelColor(0, rgbl(int2eeprom.r,int2eeprom.g,int2eeprom.b,ledNumbersDialIndicator));
  } else if (modeState == 1){
        strip.setPixelColor(0, rgbl(int2eeprom.r,int2eeprom.g,int2eeprom.b,ledNumbersDialIndicator));
  } else if (modeState == 2){
        strip.setPixelColor(0, rgbl(int2eeprom.r,int2eeprom.g,int2eeprom.b,ledNumbersDialIndicator));
  } else if (modeState == 3){
        strip.setPixelColor(0, rgbl(int2eeprom.r,int2eeprom.g,int2eeprom.b,ledNumbersDialIndicator));
  }


 // indicator off
  if (dialState == 0){
      if (modeState == 0){
            strip.setPixelColor(0, colorwhite);
      } else if (modeState == 1){
            strip.setPixelColor(0, colorredoff);
      } else if (modeState == 2){
            strip.setPixelColor(0, colorgreenoff);
      } else if (modeState == 3){
            strip.setPixelColor(0, colorblueoff);
      }
  }

// BUTTON PRESS LED INDICATORS

// change mode
  if (buttonState1 == 1){
      if (modeState == 1){
            strip.setPixelColor(0, colorred);
      } else if (modeState == 2){
            strip.setPixelColor(0, colorgreen);
      } else if (modeState == 3){
            strip.setPixelColor(0, colorblue);
      }
  }
  
 // save
  if (buttonState2 == 1){
       strip.setPixelColor(0, coloryellow);
  }

// Read low batt
   battLow = digitalRead(battPin);
   if (battLow == LOW) {
     strip.setPixelColor(0, colorred);
   } 
  
//SEND RGB VALS TO SERIAL
  
  sendColor(1, int2eeprom.r);
  sendColor(2, int2eeprom.g);
  sendColor(3, int2eeprom.b);
  sendColor(4, int2eeprom.a);
  sendColor(5, int2eeprom.ledNumbersDial);
  sendColor(6, saveLampEeprom);

  strip.show();
  delay(40);

}



// SPLIT ENCODER VALS (RISE/FALL shit encoder)
long getEncPosition(){
    return knob.read()/2;
}

  
//SEND TO TX
void sendColor(int colortype, int colorval) {
       SSerial.write(char(255));
       SSerial.write(char(colortype));
       SSerial.write(char(colorval));
}

// EEPROM THE SHIT (DATATYPES)----------------------------------------------------------------------

void load() {
  EEPROM_readAnything(0,int2eeprom);
}

void save(){
  EEPROM_writeAnything(0,int2eeprom);
}
