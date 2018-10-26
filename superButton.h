#define FILENAME "superButton"
#define MYVERSION "1.0.1"
#define BUILD_DATE "10/25/2018"
#define BUTTON1 D6
#define BUTTONRED A4
#define BUTTONGREEN A5
#define BUTTONBLUE A7
#define PIR A0
#define BLINKER D7
#define INT_PIN D5
#define NEOSTRIP D4
#define CONFIGADDR 10
#define PIXEL_PIN D2 // IMPORTANT: Set pixel COUNT, PIN and TYPE for Neopixel
#define PIXEL_COUNT 2
#define PIXEL_TYPE SK6812RGBW

// Prototypes for local build, ok to leave in for Build IDE
  void assignColors();
  int getColor(String command);
  int setMode(String command);
  int setConfig(String command);
  void doGesture();
  void lightsOut();
  void setColor();
  void checkMode(int mode);
  void checkButton();
  void dragoHandler(const char *event, const char *data);
  void setButtonColor(int red, int green, int blue) ;
  void juiceLeds(int ured, int ugreen,int ublue, int uwhite); 
  struct MyConfig {
    bool motionArmed;
    bool gestureArmed;
    char version[16];
    int awayHoldTMR;
  };
