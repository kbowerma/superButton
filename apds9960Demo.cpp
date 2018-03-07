
#include "application.h"
 #include "lib/streaming/firmware/spark-streaming.h"
 #include "lib/OneWire/OneWire.h"
 #include "lib/Particle-NeoPixel/src/neopixel/neopixel.h"
 #include "Adafruit_APDS9960_Particle.h"
 #include "apds9960Demo.h"

SYSTEM_MODE(AUTOMATIC);

// IMPORTANT: Set pixel COUNT, PIN and TYPE for Neopixel
#define PIXEL_PIN D2
#define PIXEL_COUNT 2
#define PIXEL_TYPE SK6812RGBW


Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);
Adafruit_APDS9960 apds;

// Prototypes for local build, ok to leave in for Build IDE
void rainbow(uint8_t wait);
  void assignColors();
  int getColor(String command);
  int setMode(String command);
  int toogleRainbow(String command);
  int setDelay(String command);
  void printGesture();
  uint32_t Wheel(byte WheelPos);
  void lightsOut();
  void setColor();

//Global variables
uint16_t r, g, b, c = 0;  // variable to store colororData(&r, &g, &b, &c);
  uint16_t dorainbow = 0;
  uint8_t proximity_data = 0;
  int red, green, blue, clear = 0;  // variable to store color
  int pin1 = D5;
  int mode, mydelay =  0;
  int blinker = D7;


void setup() {
  Serial.begin(9600);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  Particle.variable("red", red);
  Particle.variable("green", green);
  Particle.variable("blue", blue);
  Particle.variable("clear", clear);
  Particle.variable("version", MYVERSION);
  Particle.variable("fileName", FILENAME);
  Particle.variable("buildDate", BUILD_DATE);
  Particle.variable("doRainbow", dorainbow);
  Particle.function("rainbow",toogleRainbow);
  Particle.function("getcolor",getColor);
  Particle.function("setMode",setMode);
  Particle.function("setDelay",setDelay);

  pinMode(pin1, INPUT_PULLUP);
  pinMode(blinker, OUTPUT);

  if(!apds.begin()){
    Serial.println("failed to initialize device! Please check your wiring.");
  }
  else Serial.println("Device initialized!");


  apds.enableColor(true); //enable color sensign mode
  apds.enableProximity(true);
  //apds.enableGesture(true);  //this breaks the Proximity,and color
  apds.setProximityInterruptThreshold(0, 175); //set the interrupt threshold to fire when proximity reading goes above 175
  apds.enableProximityInterrupt();  //enable the proximity interrupt

}

void loop() {

  if (mode == 0 && millis() % 500 == 0 ) assignColors();
  if( mode == 1 ) printGesture();
  if( mode == 2 ) rainbow(20);
  if( mode == 3 ) setColor();
  if( mode == 4 ) lightsOut();
  //strip.show();
  digitalWrite(blinker,!digitalRead(blinker));
  delay(mydelay);
  for (int i = 0 ; i < 1000; i++) {
    Serial << " this is a dumb loop" << endl ;
  }


}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
int  toogleRainbow(String command) {
    dorainbow = command.toInt();
    if (dorainbow == 0 ) {
        strip.setPixelColor(0, 0,0,0,0 );
        strip.setPixelColor(1, 0,0,0,0 );
        return 0;
    }
}
int  getColor(String command) {
    apds.getColorData(&r, &g, &b, &c);
    return r;
}
void assignColors() {
  apds.getColorData(&r, &g, &b, &c);
  Serial.print("red: ");
  red = r;
  Serial.print(r);

  Serial.print(" green: ");
  green =g;
  Serial.print(g);

  Serial.print(" blue: ");
  blue = b;
  Serial.print(b);

  Serial.print(" clear: ");
  clear =c ;
  Serial.print(c);
  Serial.print(" ");


  Serial.print(" proximity ");
  Serial.print(apds.readProximity());
  apds.clearInterrupt();  //clear the interrupt

  Serial << " D5 " << digitalRead(pin1) << endl ;

  Serial.println();



}
void printGesture() {
  uint8_t gesture = apds.readGesture();
  if(gesture == APDS9960_DOWN) Serial.println("v");
  if(gesture == APDS9960_UP) Serial.println("^");
  if(gesture == APDS9960_LEFT) Serial.println("<");
  if(gesture == APDS9960_RIGHT) Serial.println(">");
}
int  setMode(String command ) {
  int c = command.toInt();
  switch (c) {
    case 0: // color & proximity
      apds.enableColor(true);
      apds.enableGesture(false);
      //lightsOut();
      mode = c;
      break;
    case 1:  // gesture
      apds.enableColor(false);
      apds.enableGesture(true);
      lightsOut();
      mode = c;
      break;
    case 2:  // rainbow
      apds.enableColor(false);
      apds.enableGesture(false);
      mode = c;
      break;
    case 3:
      setColor();
      mode = 0;
      break;
    }

  return mode;
}
void lightsOut() {
  strip.setPixelColor(0, 0,0,0,0 );
  strip.setPixelColor(1, 0,0,0,0 );
  strip.show();

}
void setColor() {
  strip.setPixelColor(0, red,green,blue,clear );
  strip.setPixelColor(1, red,green,blue,clear );
  strip.show();
}
int setDelay(String command) {
  mydelay = command.toInt();
  return mydelay;
}
