
#include "application.h"
 #include "lib/streaming/firmware/spark-streaming.h"
 #include "lib/OneWire/OneWire.h"
 #include "lib/Particle-NeoPixel/src/neopixel/neopixel.h"
 #include "Adafruit_APDS9960_Particle.h"
 #include "lib/clickButton/src/clickButton.h"
 #include "superButton.h"

 /*
 Button post:  https://community.particle.io/t/photon-wkp-pin-button-library/37166
 */

SYSTEM_MODE(AUTOMATIC);

#define PIXEL_PIN D2 // IMPORTANT: Set pixel COUNT, PIN and TYPE for Neopixel
#define PIXEL_COUNT 2
#define PIXEL_TYPE SK6812RGBW

Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);
Adafruit_APDS9960 apds;

//the BUTTON
//const int buttonPin1 = 4;
ClickButton button1(BUTTON1, LOW, CLICKBTN_PULLUP);

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
  void checkMode(int mode);
  void checkButton();
  void dragoHandler(const char *event, const char *data);
  void setButtonColor(int red, int green, int blue) ;

//Global variables
uint16_t r, g, b, c = 0;  // variable to store colororData(&r, &g, &b, &c);
  uint16_t dorainbow = 0;
  uint8_t proximity_data = 0;
  int red, green, blue, clear = 0;  // variable to store color
  int INT_PIN = D5;  // I think this is the apds coms pin
  int mode, mydelay, distance =  0;
  int blinker = D7;
  String buttonTEXT = "Not Set";
  String dragoState = "xx";

  // Button results
  int function = 0;


void setup() {
  Serial.begin(9600);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  Particle.variable("red", red);
  Particle.variable("green", green);
  Particle.variable("blue", blue);
  Particle.variable("clear", clear);
  Particle.variable("mode", mode);
  Particle.variable("dragoState", dragoState);
  Particle.variable("buttonTEXT", buttonTEXT);
  Particle.variable("version", MYVERSION);
  Particle.variable("fileName", FILENAME);
  Particle.variable("buildDate", BUILD_DATE);
  Particle.variable("doRainbow", dorainbow);
  Particle.variable("distance", distance);
  Particle.function("rainbow",toogleRainbow);
  Particle.function("getcolor",getColor);
  Particle.function("setMode",setMode);
  Particle.function("setDelay",setDelay);
  Particle.subscribe("drago.state", dragoHandler, MY_DEVICES);

  pinMode(INT_PIN, INPUT_PULLUP);
  pinMode(blinker, OUTPUT);
  pinMode(BUTTONRED, OUTPUT);
  pinMode(BUTTONGREEN, OUTPUT);
  pinMode(BUTTONBLUE, OUTPUT);

  //  pinMode(BUTTON1, INPUT_PULLUP);  I think this is done in the Button Library now
    //attachInterrupt(BUTTON1, checkButton, FALLING);
    //pinMode(D7, OUTPUT);
    //digitalWrite(D7, LOW);  // turn off blue led

  if(!apds.begin()){
    Serial.println("failed to initialize device! Please check your wiring.");
  }
  else Serial.println("Device initialized!");


  // ApDS 9960 Modes Pick 1

    //  1. Proximity Mode
    // apds.enableProximity(true); //enable proximity mode
    // apds.setProximityInterruptThreshold(0, 175); //set the interrupt threshold to fire when proximity reading goes above 175
    // apds.enableProximityInterrupt(); //enable the proximity interrupt

  //2.  Gesture mode
    //gesture mode will be entered once proximity mode senses something close
    apds.enableProximity(true);
    apds.enableGesture(true);
  //3.  Color mode
      //apds.enableColor(true);


    // Setup button timers (all in milliseconds / ms)
  // (These are default if not set, but changeable for convenience)
  button1.debounceTime   = 20;   // Debounce timer in ms
  button1.multiclickTime = 250;  // Time limit for multi clicks
  button1.longClickTime  = 1000; // time until "held-down clicks" register (was 100)



  setButtonColor(0,0,0);


}


void loop() {

  printGesture();

  button1.Update();
  // Save click codes in LEDfunction, as click codes are reset at next Update()
  if(button1.clicks != 0) function = button1.clicks;
  if(function == 1) {
    buttonTEXT = "SINGLE click";
    Particle.publish("buttonTEXT", "SINGLE click");
    if (dragoState == "00") {
      Particle.publish("drago", "10",PRIVATE);
      setButtonColor(255,0,0);
    } else if (dragoState == "10") {
      Particle.publish("drago", "11",PRIVATE);
      setButtonColor(0,255,0);
    } else if (dragoState == "11") {
      Particle.publish("drago", "01",PRIVATE);
      setButtonColor(0,0,255);
    } else {
      Particle.publish("drago", "00",PRIVATE);
      setButtonColor(0,255,255);
    }
  }
  if(function == 2) {
    buttonTEXT = "DOUBLE click";
    Particle.publish("buttonTEXT", "DOUBLE click");
    Particle.publish("drago", "11",PRIVATE);
  }
  if(function == 3) {
    buttonTEXT = "TRIPLE click";
    Particle.publish("buttonTEXT", "TRIPLE click");
    Particle.publish("drago", "01",PRIVATE);
  }
  if(function == -1) {
    buttonTEXT = "SINGLE LONG click";
    Particle.publish("buttonTEXT", "SINGLE LONG click");
    Particle.publish("drago", "00",PRIVATE);
    setButtonColor(0,0,0);
  }
  if(function == -2) {
    buttonTEXT = "DOUBLE LONG click";
    Particle.publish("buttonTEXT", "DOUBLE LONG click");
  }
  if(function == -3) {
    buttonTEXT = "TRIPLE LONG click";
    Particle.publish("buttonTEXT", "TRIPLE LONG click");
  }
  function = 0;
  //delay(5);
  distance =  digitalRead(INT_PIN);

  //checkMode(mode);
  //digitalWrite(blinker,!digitalRead(blinker));
  //checkButton();
  //delay(mydelay);




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

  Serial << " D5 " << digitalRead(INT_PIN) << endl ;

  Serial.println();



}
void printGesture() {
  digitalWrite(blinker,!digitalRead(blinker));
  uint8_t gesture = apds.readGesture();
    if(gesture == APDS9960_DOWN) {
      Serial.println("v");
      Particle.publish("gesture", "Down");
      Particle.publish("drago", "00",PRIVATE);
      setButtonColor(255,255,255);
    }
    if(gesture == APDS9960_UP) {
      Serial.println("^");
      Particle.publish("gesture", "UP");
      Particle.publish("drago", "11",PRIVATE);
      setButtonColor(0,0,0);
    }
    if(gesture == APDS9960_LEFT) {
      Serial.println("<");
      Particle.publish("gesture", "left");
      Particle.publish("drago", "10",PRIVATE);
      setButtonColor(56,56,56);
    }
    if(gesture == APDS9960_RIGHT) {
      Serial.println(">");
      Particle.publish("gesture", "right");
      Particle.publish("drago", "01",PRIVATE);
      setButtonColor(128,128,128);
    }
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
      mode = c;
      break;
    case 4:
      lightsOut();
      mode = c;

      break;
    }
    Particle.publish("setMode",String(c));
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
void checkMode(int mode){
  if (mode == 0 && millis() % 500 == 0 ) assignColors();
  if( mode == 1 ) printGesture();
  if( mode == 2 ) rainbow(20);
  if( mode == 3 ) setColor();
  if( mode == 4 ) lightsOut();
  //if( mode > 4 ) mode = 0;  //TODO should this be here or on the button,
}
void checkButton(){
  if( digitalRead(D6)==LOW ) { // button pushed turn on blue led
    digitalWrite(D7, HIGH);
    mode++;
    if( mode > 4 ) mode = 0;
      Particle.publish("setMode",String(mode));
    delay(250);
  } else  {
  digitalWrite(D7, LOW);  // turn off blue led
  }

}
void dragoHandler(const char *event, const char *data) {
   dragoState = data;

}
void setButtonColor(int red, int green, int blue) {
  analogWrite(BUTTONRED, 255-red);
  analogWrite(BUTTONBLUE, 255-blue);
  analogWrite(BUTTONGREEN, 255-green);

}
