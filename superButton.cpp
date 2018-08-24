/* SuperButton is a code base that uses the following devices
 1.) RGB button
 2.) APDS 9960 proximaty, gesture and light sensor
 3.) PIR sensor
 purpose is to manage my overhad lights and my standing desk,  it is a client to a parcile on a relay board
 Reference
  Button post:  https://community.particle.io/t/photon-wkp-pin-button-library/37166  
  Sparkfun lib (not used) https://github.com/sparkfun/APDS-9960_RGB_and_Gesture_Sensor/
 */ 

// Includes
 #include "application.h"
 #include "lib/streaming/firmware/spark-streaming.h"
 #include "lib/OneWire/OneWire.h"
 #include "lib/Particle-NeoPixel/src/neopixel/neopixel.h"
 #include "Adafruit_APDS9960_Particle.h"
 #include "lib/clickButton/src/clickButton.h"
 #include "superButton.h"

// Objects 
  SYSTEM_MODE(AUTOMATIC);
  Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);
  Adafruit_NeoPixel strip2(60, D4, WS2812B);
  Adafruit_APDS9960 apds;
  ClickButton button1(BUTTON1, LOW, CLICKBTN_PULLUP);
  MyConfig myConfig = { false, false, "Test!"}; // instatinate the config object with default values


//Global variables
  uint16_t r, g, b, c = 0;  // variable to store colororData(&r, &g, &b, &c);
  uint16_t dorainbow = 0;
  uint8_t proximity_data = 0;
  int red, green, blue, clear = 0;  // variable to store color
  int mode, mydelay, distance =  0;
  String buttonTEXT = "Not Set";
  String dragoState = "xx";
  boolean enGesture = true;
  int motionState, oldMotionState;
  int lastMotionTime, secSinceMotion = 0;
  int function = 0;


void setup() {
 
  EEPROM.get(CONFIGADDR,myConfig);  // get config from EEPROM



  Serial.begin(9600);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  strip2.begin();
  strip2.clear();  // dont know why I get one green.
  strip2.show(); // Initialize all pixels to 'off'
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
  Particle.variable("enGesture", enGesture);
  Particle.variable("lastMotion", secSinceMotion);
  Particle.variable("isArmed", myConfig.isArmed);
  Particle.function("rainbow",toogleRainbow);
  Particle.function("getcolor",getColor);
  Particle.function("setMode",setMode);
  Particle.function("setDelay",setDelay);
  Particle.function("togGesture",togGesture);
  Particle.function("togMotArm",toggleMotionArmedFunction);
  Particle.subscribe("log.drago.state", dragoHandler, MY_DEVICES);

  pinMode(INT_PIN, INPUT_PULLUP);
  pinMode(BLINKER, OUTPUT);
  pinMode(BUTTONRED, OUTPUT);
  pinMode(BUTTONGREEN, OUTPUT);
  pinMode(BUTTONBLUE, OUTPUT);
  pinMode(PIR, INPUT_PULLDOWN);


  if(!apds.begin()){
    Serial.println("failed to initialize device! Please check your wiring.");
    Particle.publish("DEBUG","failed to initialize device! Please check your wiring.");
  }
  else {
    Serial.println("Device initialized!");
    Particle.publish("DEBUG","failed to initialize device! Please check your wiring.");
  }

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
     //   apds.enableColor(true);  // lets try both gesture and color;  this only works if I dont fire the gesture



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
  switch(function){
    case 1:
      buttonTEXT = "SINGLE click";
      Particle.publish("buttonTEXT", "SINGLE click");
      if (dragoState == "00") {
        secSinceMotion = 0;  // just in case
        Particle.publish("drago", "10",PRIVATE);
        setButtonColor(9,0,0);
      } else if (dragoState == "10") {
        secSinceMotion = 0;  // just in case
        Particle.publish("drago", "11",PRIVATE);
        setButtonColor(0,9,0);
      } else if (dragoState == "11") {
        secSinceMotion = 0;  // just in case
        Particle.publish("drago", "01",PRIVATE);
        setButtonColor(0,0,9);
      } else {
        Particle.publish("drago", "00",PRIVATE);
        setButtonColor(0,9,9);
      }
      break;
    case 2:
      buttonTEXT = "DOUBLE click";
      Particle.publish("buttonTEXT", "DOUBLE click");
      Particle.publish("drago", "11",PRIVATE);
      break;
    case 3:
      buttonTEXT = "TRIPLE click";
      Particle.publish("buttonTEXT", "TRIPLE click");
      Particle.publish("drago", "01",PRIVATE);
      break;
    case -1:
      buttonTEXT = "SINGLE LONG click";
      Particle.publish("buttonTEXT", "SINGLE LONG click");
      Particle.publish("drago", "00",PRIVATE);
      //setButtonColor(255,255,0);  // try yellow -- does red
      setButtonColor(64,128,0);  // haha yellow works.  green has to be 2x red
      break;
    case -2:
      buttonTEXT = "DOUBLE LONG click";
      Particle.publish("buttonTEXT", "DOUBLE LONG click");
      break;
    case -3:
      buttonTEXT = "TRIPLE LONG click";
      Particle.publish("buttonTEXT", "TRIPLE LONG click");
      break;
  }

  function = 0;  // reset the click type
  //delay(5);
  distance =  digitalRead(INT_PIN);   // THIS DOENSNT WORK

  //PIR sensor
  motionState = digitalRead(PIR);
  if (motionState != oldMotionState ) {  // this should save some time in the loop and not call a show every time
    switch (motionState) {
      case 0:
        //strip.setPixelColor(0, 0,0,0,0 );
        strip.clear();
        break;
      case 1:
        strip.setPixelColor(1,10,0,0,0 );
        if (myConfig.isArmed)  strip.setPixelColor(0,10,0,0,0 );
        if (!myConfig.isArmed) strip.setPixelColor(0,10,10,0,0 );
        lastMotionTime = millis();

        break;
    }
    strip.show();
  }

  //check for away
  if ( dragoState != "00" && secSinceMotion > AWAYHOLDOWNTIMER && myConfig.isArmed == true )  {
          Particle.publish("drago", "00",PRIVATE);
          setButtonColor(255,0,255);  // I wanna turn it yellow.
            // 255,255,0); is red but I have no green

          dragoState = "00";
  }


  secSinceMotion = ( millis() - lastMotionTime )/1000;
  oldMotionState = motionState;

}

//----Functions ---------------
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
      // dorainbow = command.toInt();
      // if (dorainbow == 0 ) {
      //     strip.setPixelColor(0, 0,0,0,0 );
      //     strip.setPixelColor(1, 0,0,0,0 );
      //     return 0;
      // }
      if ( command.toInt() == 1 ) rainbow(10);
      if ( command.toInt() == 0 ) {
        strip2.clear();
      }
  }
  int  getColor(String command) {
      int myreturn;
      apds.enableProximity(false);
      apds.enableGesture(false);
      apds.enableColor(true);
      apds.getColorData(&r, &g, &b, &c);
      if (command == "r" ) myreturn = r;
      if (command == "g" ) myreturn = g;
      if (command == "b" ) myreturn = b;
      if (command == "c" ) myreturn = c;
      // Now reenable gesture
      apds.enableProximity(true);
      apds.enableGesture(true);
      return myreturn;
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
    switch (apds.readGesture() ) {
      case APDS9960_DOWN:
        Serial.println("v");
        Particle.publish("gesture", "Down");
        Particle.publish("drago", "00",PRIVATE);
        setButtonColor(255,255,255);
        break;
      case APDS9960_UP:
          Serial.println("^");
          Particle.publish("gesture", "UP");
          Particle.publish("drago", "11",PRIVATE);
          setButtonColor(0,0,0);
          break;
      case APDS9960_LEFT:
          Serial.println("<");
          Particle.publish("gesture", "left");
          Particle.publish("drago", "10",PRIVATE);
          setButtonColor(56,56,56);
          break;
      case APDS9960_RIGHT:
          Serial.println(">");
          Particle.publish("gesture", "right");
          Particle.publish("drago", "01",PRIVATE);
          setButtonColor(128,128,128);
          break;
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
      apds.enableProximity(true);
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
      case 5:
        //turn on bar
        #define mydelay2 30
        for (int n=0; n < 60; n++) {
        //strip2.setPixelColor(n, 255,255,255 );
        strip2.setColorDimmed(n,255,255,240,28+4*n);
        delay(mydelay2);
        strip2.show();
        }
        delay(500);
        for (int n=60; n > 0; n--) {
          strip2.setColorDimmed(n,255,255,240,0);
          delay(mydelay2);
          strip2.show();
        }

       // strip2.show();

       
        break;
      case 6:
        strip2.clear();
        strip2.show();
        break;
      }
       mode = c;
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
  void setButtonColor(int myred, int mygreen, int myblue) {
    red = myred;
    green = mygreen;
    blue = myblue;
    analogWrite(BUTTONRED, 255-red);
    analogWrite(BUTTONBLUE, 255-blue);
    analogWrite(BUTTONGREEN, 255-green);

  }
  int togGesture(String command){
    if( command.toInt() == 1)  { enGesture = true; }
    if( command.toInt() == 0)  { enGesture = false; }
  }
  int toggleMotionArmedFunction(String command) {
    if ( command.toInt() == 1) {
      //holdDownArmed = true;
      myConfig.isArmed = true;
      EEPROM.put(CONFIGADDR,myConfig);
    } else  if ( command.toInt() == 0) {
      // holdDownArmed = false;
      myConfig.isArmed = false;
      EEPROM.put(CONFIGADDR,myConfig);
    } else return -1;

    return myConfig.isArmed;
  }
