#include <Adafruit_NeoPixel.h>
#include <ezTime.h>
#include <WiFiManager.h>

#include "font.h"

#define CHAR_WIDTH 5
#define CHAR_HEIGHT 5
#define LED_PIN 8
#define LED_COUNT 25

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

uint32_t red = strip.Color(255, 0, 0);
uint32_t green = strip.Color(0, 255, 0);
uint32_t blue = strip.Color(0, 0, 255);
uint32_t amber = strip.Color(255, 140, 0);

Timezone myTZ;

void DrawPixel(uint32_t colour, uint8_t x, uint8_t y, uint8_t brightness)
{
  uint8_t led = (4 - y) * 5 + (4 - x); //  Calc LED from x,y coords, 0,0 is bottom left pixel
  uint8_t red = ((colour >> 16) & 0xff) * brightness / 100;
  uint8_t green = ((colour >> 8) & 0xff) * brightness / 100;
  uint8_t blue = (colour & 0xff) * brightness / 100;
  strip.setPixelColor(led, red, green, blue);
  // strip.show();
}

// Based on code from https://jared.geek.nz/2014/jan/custom-fonts-for-microcontrollers
void DrawChar(uint32_t colour, char c, uint8_t brightness)
{
  uint8_t x, y;

  // Convert the character to an index
  c = c & 0x7F;
  if (c < ' ')
  {
    c = 0;
  }
  else
  {
    c -= ' ';
  }

  // 'font' is a multidimensional array of [96][char_width]
  const uint8_t *chr = font[c];
  // Draw pixels
  for (x = 0; x < CHAR_WIDTH; x++)
  {
    for (y = 0; y < CHAR_HEIGHT; y++)
    {
      if (chr[x] >> 2 & (1 << y))
      {
        DrawPixel(colour, x, y, brightness);
      }
      strip.show();
    }
  }
}

void FadeChar(uint32_t colour, char c)
{
  for (uint8_t br = 20; br <= 100; br = br + 5)
  {
    DrawChar(colour, c, br);
  }
  delay(100);

  for (uint8_t br = 100; br > 0; br = br - 5)
  {
    DrawChar(colour, c, br);
  }
  delay(100);

  strip.clear();
  strip.show(); // Initialize all pixels to 'off'
}

void FadeString(uint32_t colour, String s)
{
  char buffer[s.length() + 1];
  s.toCharArray(buffer, s.length() + 1);
  for (int i = 0; i < s.length() + 1; i++)
    FadeChar(colour, buffer[i]);
}

void GetNTP()
{

  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_5dBm);
  WiFi.setHostname("esp32clock");
  WiFiManager wm;
  wm.setDebugOutput(false);

  // TODO: probably should enable the user to select a timezone
  // and set a hostname on the Wifi manager setup screen
  bool res;
  res = wm.autoConnect("5x5_Clock", "Clock123"); // create password protected ap
  if (!res)
  {
    Serial.println(F("Failed to start Wifi Manager AP"));
  }
  else
  {
    Serial.println(F("[+] Connected to Wi-Fi"));
    Serial.println(F("[+] Syncing NTP"));
    waitForSync();
    Serial.println("[+] UTC: " + UTC.dateTime());

    if (myTZ.setLocation(F("Europe/London")))
    {
      Serial.println(F("[+] Timezone lookup OK"));
      Serial.print("[+] UK: ");
      Serial.println(myTZ.dateTime());
    }
    else
    {
      Serial.println("[-] Timezone lookup failed, will use UTC");
      myTZ = UTC;
    }
  }
  WiFi.disconnect();
}

void setup()
{

  strip.begin();
  strip.setBrightness(20);
  strip.show(); // Initialize all pixels to 'off'

  FadeString(red, "Wifi");
  GetNTP();
  FadeString(green, "Up");
}

void loop()
{

  events();

  if (minuteChanged())
  {
    FadeString(blue, myTZ.dateTime("dmy"));
  }

  FadeString(amber, myTZ.dateTime("H:i"));
}
