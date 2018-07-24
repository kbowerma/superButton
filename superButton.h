#define FILENAME "superButton"
#define MYVERSION "0.2.4_PIR"
#define BUILD_DATE "7/23/2018"
#define BUTTON1 D6
#define BUTTONRED A4
#define BUTTONGREEN A5
#define BUTTONBLUE A7
#define PIR A0
#define BLINKER D7
#define INT_PIN D5

// Prototypes for local build, ok to leave in for Build IDE
void rainbow(uint8_t wait);
  void assignColors();
  int getColor(String command);
  int setMode(String command);
  int toogleRainbow(String command);
  int setDelay(String command);
  int togGesture(String command);
  void printGesture();
  uint32_t Wheel(byte WheelPos);
  void lightsOut();
  void setColor();
  void checkMode(int mode);
  void checkButton();
  void dragoHandler(const char *event, const char *data);
  void setButtonColor(int red, int green, int blue) ;
