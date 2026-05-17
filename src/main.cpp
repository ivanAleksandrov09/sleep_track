#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <sstream>
#include <string>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// The internal OLED is connected to I2C pins 21 and 22
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// // setup variables for wifi connection
// const char *ssid = "";
// const char *password = "";

// String serverName = "http://192.168.0.174:8000/";

void setup()
{
  Serial.begin(115200);

  // WiFi.begin(ssid, password);
  // Serial.println("Connecting");
  // while (WiFi.status() != WL_CONNECTED)
  // {
  //   delay(500);
  //   Serial.print(".");
  // }
  // Serial.println("");
  // Serial.print("Connected to WiFi network with IP Address: ");
  // Serial.println(WiFi.localIP());

  // You MUST call .begin to wake up the screen
  // 0x3C is the standard I2C address for these Ideaspark boards
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("No pings yet!");
  display.display(); // This command actually "pushes" the data to the screen
}

void loop()
{
}