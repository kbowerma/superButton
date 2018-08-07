#define FILENAME "superButton"
#define MYVERSION "0.3.4"
#define BUILD_DATE "8/6/2018"
#define BUTTON1 D6
#define BUTTONRED A4
#define BUTTONGREEN A5
#define BUTTONBLUE A7
#define PIR A0
#define BLINKER D7
#define INT_PIN D5
#define AWAYHOLDOWNTIMER 300
#define CONFIGADDR 10
#define PIXEL_PIN D2 // IMPORTANT: Set pixel COUNT, PIN and TYPE for Neopixel
#define PIXEL_COUNT 2
#define PIXEL_TYPE SK6812RGBW

// Prototypes for local build, ok to leave in for Build IDE
void rainbow(uint8_t wait);
  void assignColors();
  int getColor(String command);
  int setMode(String command);
  int toogleRainbow(String command);
  int setDelay(String command);
  int togGesture(String command);
  int toggleMotionArmedFunction(String command);
  void printGesture();
  uint32_t Wheel(byte WheelPos);
  void lightsOut();
  void setColor();
  void checkMode(int mode);
  void checkButton();
  void dragoHandler(const char *event, const char *data);
  void setButtonColor(int red, int green, int blue) ;

  struct MyConfig {
    bool isArmed;
    bool enGesture;
    char version[16];
  };
