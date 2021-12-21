
#include <Adafruit_DotStar.h>
#include <SoftwareSerial.h>
#include <YesNoIO_rgbl.h>
#include <EEPROM.h>
#include <eepromtheshit.h>

#define NUMPIXELS 21
#define DATAPIN    5
#define CLOCKPIN   4
Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);

#define DEBOUNCE_INTERVAL 500L // Milliseconds

// DATATYPES TO INCLUDE IN EEPROM
struct config_t{
  int r = 250;
  int g = 150;
  int b = 80;
  int a = 250;
  int ledNumbersDial = 21;
} int2eeprom;

int z = 0;

int remoteEepromSave = 0;
int saveState = 0;
int bright = 250;
unsigned long remoteSaveDelay = 0;
unsigned long now2 = millis();  // NOW2 FOR BUTTONS DEBOUNCE

// Serial input
int userInput[3];   
int startbyte;       
int colortype;         
int colorval;         
int i;     



SoftwareSerial SSerial(2, 3); // RX/TX HC12

void setup() {
  
    SSerial.begin(9600);
    Serial.begin(9600);
  
    strip.setBrightness(bright);
    strip.begin();
    strip.show();
    
    load(); // LOAD DATATYPE VALUES FROM EEPROM
    Serial.println("Aircon RECEIVER Loaded!");
    
    delay(1000);
}

void loop() {

  
now2 = millis();

  strip.setPixelColor(0, rgbl(z, z, z, z));
  strip.setPixelColor(1, rgbl(z, z, z, z));
  strip.setPixelColor(2, rgbl(z, z, z, z));
  strip.setPixelColor(3, rgbl(z, z, z, z));
  strip.setPixelColor(4, rgbl(z, z, z, z));
  strip.setPixelColor(5, rgbl(z, z, z, z));
  strip.setPixelColor(6, rgbl(z, z, z, z));
  strip.setPixelColor(7, rgbl(z, z, z, z));
  strip.setPixelColor(8, rgbl(z, z, z, z));
  strip.setPixelColor(9, rgbl(z, z, z, z));
  strip.setPixelColor(10, rgbl(z, z, z, z));
  strip.setPixelColor(11, rgbl(z, z, z, z));
  strip.setPixelColor(12, rgbl(z, z, z, z));
  strip.setPixelColor(13, rgbl(z, z, z, z));
  strip.setPixelColor(14, rgbl(z, z, z, z));
  strip.setPixelColor(15, rgbl(z, z, z, z));
  strip.setPixelColor(16, rgbl(z, z, z, z));
  strip.setPixelColor(17, rgbl(z, z, z, z));
  strip.setPixelColor(18, rgbl(z, z, z, z));
  strip.setPixelColor(19, rgbl(z, z, z, z));
  strip.setPixelColor(20, rgbl(z, z, z, z));
//  strip.setPixelColor(21, rgbl(z, z, z, z));

  
 // Receive colorpicker serial through 443MHz

   if (SSerial.available() > 2) {

      startbyte = SSerial.read();

         if (startbyte == 255) {
           for (i=0;i<2;i++) {
             userInput[i] = SSerial.read();
           }
           colortype = userInput[0];
           colorval = userInput[1];
           if (colorval == 255) { 
            colortype = 255; 
            }
           
           switch (colortype) {
             case 1:
              int2eeprom.r = colorval;
               break;
             case 2:
              int2eeprom.g = colorval;      
               break;
             case 3:
              int2eeprom.b = colorval;
               break;
             case 4:
              int2eeprom.a = colorval;
               break;
             case 5:
              int2eeprom.ledNumbersDial = colorval;
               break;
             case 6:
              remoteEepromSave = colorval;
               break;
           }
         }
   
 }

  if ((remoteEepromSave == 1) && (saveState == 0)) { // save to eeprom
      saveState = 1;
      save();
      Serial.println("Saved settings to eeprom!");
      remoteSaveDelay = now2;
  }
  if ((remoteSaveDelay + (DEBOUNCE_INTERVAL)) < now2) {
      saveState = 0;
  }
  
//Serial.print("R:");
//Serial.print(r);
//Serial.print(" G:");
//Serial.print(g);
//Serial.print(" B:");
//Serial.print(b);
//Serial.print(" LedNum:");
//Serial.println(ledNumbersDial);

if (saveState == 1){
  for(byte x=0;x<int2eeprom.ledNumbersDial;x++) {
    strip.setPixelColor(x, rgbl(20, 100, 20, int2eeprom.a));
  }
}else{
  for(byte x=0;x<int2eeprom.ledNumbersDial;x++) {
    strip.setPixelColor(x, rgbl(int2eeprom.r, int2eeprom.g, int2eeprom.b, int2eeprom.a));
  }
}


  
  strip.show(); 
  delay(2);

}


// EEPROM THE SHIT (DATATYPES)----------------------------------------------------------------------

void load() {
  EEPROM_readAnything(0,int2eeprom);
}

void save(){
  EEPROM_writeAnything(0,int2eeprom);
}
