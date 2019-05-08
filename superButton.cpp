/* SuperButton is a code base that uses the following devices
 1.) RGB button
 2.) APDS 9960 proximaty, gesture and light sensor
 3.) PIR sensor
 purpose is to manage my overhad lights and my standing desk,  it is a client to a parcile on a relay board
 Reference
  Button post:  https://community.particle.io/t/photon-wkp-pin-button-library/37166  
  Sparkfun lib (not used) https://github.com/sparkfun/APDS-9960_RGB_and_Gesture_Sensor/
 * 10/25/18:  1.0.2  moved button handler to a function
 * 11/28/18:   Just got Mesh, going to flash this on the old firmare  release/v0.5.2   - then going to move to a new directory and try it again.
 * 3/4/2019:  trying to figure out #6 button or gesture this looks promising:
 *   https://community.particle.io/t/apds-9960-and-photon/42722/16
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
  Adafruit_NeoPixel strip2(60, NEOSTRIP, SK6812RGBW);
  Adafruit_APDS9960 apds;
  ClickButton button1(BUTTON1, LOW, CLICKBTN_PULLUP);
  MyConfig myConfig = { false, false, "Test!"}; // instatinate the config object with default values


//Global variables
  uint16_t r, g, b, w = 0;  // variable to store colororData(&r, &g, &b, &w);
  uint8_t proximity_data = 0;
  int red, green, blue, white = 0;  // variable to store color
  int mode, distance =  0;
  String buttonTEXT = "Not Set";
  String dragoState = "xx";
  int motionState, oldMotionState;
  int lastMotionTime, secSinceMotion = 0;
  int buttonState = 0;
  uint32_t ticksperloop, start;
  unsigned long thistime, lasttime = 0;
  double myperiod = 0;
  double debugcounter1 = 0;
  int isr_flag = 0;


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
  Particle.variable("white", white);
  Particle.variable("mode", mode);
  Particle.variable("dragoState", dragoState);
  Particle.variable("buttonTEXT", buttonTEXT);
  Particle.variable("version", MYVERSION);
  Particle.variable("fileName", FILENAME);
  Particle.variable("buildDate", BUILD_DATE);
  Particle.variable("gestureArmed", myConfig.gestureArmed);
  Particle.variable("lastMotion", secSinceMotion);
  Particle.variable("motionArmed", myConfig.motionArmed);
  Particle.variable("period", myperiod);
  Particle.variable("counter1", debugcounter1);
  Particle.function("getcolor",getColor);
  Particle.function("setMode",setMode);
  Particle.function("setConfig", setConfig);
  Particle.function("freq", freqChecker);
  Particle.subscribe("log.drago.state", dragoHandler, MY_DEVICES);
  pinMode(INT_PIN, INPUT_PULLUP);
  pinMode(D7, OUTPUT);
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
    Particle.publish("DEBUG","Device initialized!");
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
  juiceLeds(0,0,0,0);  // tries to fixe that damn first green light


}

void loop() {
  
 // read Button
  if( digitalRead(BUTTON1) == HIGH ) { 
    digitalWrite(D7, HIGH );   // set the blue led when I push the button.
    } else  digitalWrite(D7, LOW ); 
  
 // read gesture
  if (myConfig.gestureArmed == true ) {
  doGesture();
  }  else setButtonColor(32,64,0);  // this yellow

 

  buttonHandler();  // function to read button and do action 

  //distance =  digitalRead(INT_PIN);   // THIS DOENSNT WORK

  //PIR sensor
  motionState = digitalRead(PIR);
  if (motionState != oldMotionState ) {  // this should save some time in the loop and not call a show every time
    switch (motionState) {
      case 0:
        strip.clear();
        break;
      case 1:
        strip.setPixelColor(1,10,0,0,0 );
        if (myConfig.motionArmed)  strip.setPixelColor(0,10,0,0,0 );
        if (!myConfig.motionArmed) strip.setPixelColor(0,10,10,0,0 );
        lastMotionTime = millis();

        break;
    }
    strip.show();
  }

  //check for away 
   if ( dragoState != "00" && secSinceMotion > myConfig.awayHoldTMR && myConfig.motionArmed == true )  { 
 // if ( dragoState != "00" && secSinceMotion > 15 && myConfig.motionArmed == true )  {
  //  if ( dragoState != "00" && secSinceMotion > 15  )  {   // I am debugging here dont know why I am not turing the lights off when the timer expires
          Particle.publish("drago", "00",PRIVATE);
          setButtonColor(255,0,255);  // I wanna turn it yellow.
          juiceLeds(0,0,0,0);
            // 255,255,0); is red but I have no green

          dragoState = "00";
  }
  if ( dragoState == "00" && red + green + blue + white != 0 && secSinceMotion > myConfig.awayHoldTMR && myConfig.motionArmed == true  ) {  // if the big lights are off and the hold down timer is meet.
    red = green = blue = white = 0;
    juiceLeds(0,0,0,0);
  }


  secSinceMotion = ( millis() - lastMotionTime )/1000;
  oldMotionState = motionState;

    // always last
   myperiod = (thistime - lasttime);
  lasttime = thistime;  // for frequency checker
  //delay(12);
  thistime = millis();
 
  

}

//----Functions ---------------


  int  getColor(String command) {
      int myreturn = 0;
      apds.enableProximity(false);
      apds.enableGesture(false);
      apds.enableColor(true);
      apds.getColorData(&r, &g, &b, &w);
      if (command == "r" ) myreturn = r;
      if (command == "g" ) myreturn = g;
      if (command == "b" ) myreturn = b;
      if (command == "w" ) myreturn = w;
      // Now reenable gesture
      apds.enableProximity(true);
      apds.enableGesture(true);
      return myreturn;
  }
  void assignColors() {
    apds.getColorData(&r, &g, &b, &w);
    Serial.print("red: ");
    red = r;
    Serial.print(r);

    Serial.print(" green: ");
    green =g;
    Serial.print(g);

    Serial.print(" blue: ");
    blue = b;
    Serial.print(b);

    Serial.print(" white: ");
    white =w ;
    Serial.print(w);
    Serial.print(" ");


    Serial.print(" proximity ");
    Serial.print(apds.readProximity());
    apds.clearInterrupt();  //clear the interrupt

    Serial << " D5 " << digitalRead(INT_PIN) << endl ;

    Serial.println();



  }
  void doGesture() {
    debugcounter1++;
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
            setButtonColor(56,0,56);
            break;
        case APDS9960_RIGHT:
            Serial.println(">");
            Particle.publish("gesture", "right");
            Particle.publish("drago", "01",PRIVATE);
            setButtonColor(0,56,56);
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
      case 2:  // rainbow what?
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
       juiceLeds(red, green, blue, white);
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
    // this is the 2 rgb leds used for notifications
    strip.setPixelColor(0, red,green,blue,white );
    strip.setPixelColor(1, red,green,blue,white );
    strip.show();
  }
  void checkMode(int mode){  // never gets called
    if (mode == 0 && millis() % 500 == 0 ) assignColors();
    if( mode == 1 ) doGesture();
    if( mode == 3 ) setColor();
    if( mode == 4 ) lightsOut();
    //if( mode > 4 ) mode = 0;  //TODO should this be here or on the button,
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
  int setConfig(String command) {
    int seperator = command.indexOf("=");
    String key = command.substring(0,seperator);
    String value = command.substring(seperator+1);
    Particle.publish(key, value);
    /// My setters
    if ( key == "myConfig.motionArmed") {
      myConfig.motionArmed = value.toInt(); 
      EEPROM.put(CONFIGADDR,myConfig); 
      return 1;
    }
    if ( key == "myConfig.gestureArmed") {
      myConfig.gestureArmed = value.toInt(); 
      EEPROM.put(CONFIGADDR,myConfig); 
      if ( value.toInt() == 1 ) { 
         setButtonColor(0,64,0);
      } else setButtonColor(32,64,0);
      return 2;
    }
    if ( key == "myConfig.awayHoldTMR" ) {
      myConfig.awayHoldTMR = value.toInt(); 
      EEPROM.put(CONFIGADDR,myConfig); 
      return 3;
    }
    if ( key == "red") { red = value.toInt(); juiceLeds(red,green,blue,white); return 5; }
    if ( key == "green") { green = value.toInt(); juiceLeds(red,green,blue,white); return 7; }
    if ( key == "blue") { blue = value.toInt(); juiceLeds(red,green,blue,white); return 8; }
    if ( key == "white") { white = value.toInt(); juiceLeds(red,green,blue,white); return 9; }
    if ( key == "all") { white = red = green = blue = value.toInt(); juiceLeds(red,green,blue,white); return 9; }
    if ( key == "reset" ) {System.reset(); return 99;}
     else
    return 0;
  }
  void juiceLeds(int ured, int ugreen,int ublue, int uwhite) {
        
        #define mydelay2 20
        for (int n=0; n < 60; n++) {
        strip2.setPixelColor(n,ugreen,ured,ublue,uwhite );  // not sure why red and green is swapped
        delay(mydelay2);
        strip2.show();
        }

  }
  void buttonHandler() {
     
     button1.Update();
   // Save click codes in LEDfunction, as click codes are reset at next Update()
   if(button1.clicks != 0) buttonState = button1.clicks;
   //secSinceMotion = 0;  // need to reset the motions incase the PIR is stuck.
   switch(buttonState){
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
      //Particle.publish("drago", "11",PRIVATE);
      juiceLeds(0,0,0,32);
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
   buttonState = 0;  // reset the click type
  }
  double freqChecker(String command) {
    //double myperiod = (thistime - lasttime);
    double myfrequency = ( 1 / myperiod ) * 1000;
    return myfrequency;
}

