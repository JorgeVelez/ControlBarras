
#include <Arduino.h>
#include <FastLED.h>
#include <ETH.h>

#include <OSCMessage.h>
#include <WiFi.h>
#include <WiFiUdp.h>

static bool eth_connected = false;
static bool artnet_connected = false;

// UDP specific
 WiFiUDP Udp;
#define ART_NET_PORT 6454
int update_rate = 16;

// LED settings
const int clockPin = 4;//D5 esp32
#define NUM_STRIPS 8
#define NUM_LEDS_PER_STRIP 216
#define NUM_LEDS NUM_LEDS_PER_STRIP * NUM_STRIPS

CRGB leds[NUM_LEDS];

uint8_t RedB;
uint8_t GreenB;
uint8_t BlueB;

int brightness = 255;

unsigned long DELAY_TIME = 0; // 10 sec
unsigned long INTERVAL = 1000;

int packetCounter = 0;
int frameCounter = 0;

void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
    case SYSTEM_EVENT_ETH_START:
      Serial.println("ETH Started");
      //set eth hostname here
      ETH.setHostname("esp32-ethernet");
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;
      initialize();

      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case SYSTEM_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    default:
      break;
  }
}


void initTest()
{
  Serial.println("");
  Serial.println("Testing red1");
  for (int i = 0 ; i < NUM_LEDS ; i++) {
    leds[i] = CRGB(255, 0, 0);
  }
  FastLED.show();
  delay(500);
  Serial.println("Testing green");

  for (int i = 0 ; i < NUM_LEDS ; i++) {
    leds[i] = CRGB(0, 255, 0);
  }
  FastLED.show();
  delay(500);
  Serial.println("Testing blue");

  for (int i = 0 ; i < NUM_LEDS ; i++) {
    leds[i] = CRGB(0, 0, 255);
  }
  FastLED.show();
  delay(500);
  Serial.println("start listen ing");

  for (int i = 0 ; i < NUM_LEDS ; i++) {
    leds[i] = CRGB(0, 0, 0);
  }
  FastLED.show();
}

void setup()
{
  Serial.begin(115200);

  WiFi.onEvent(WiFiEvent);
  ETH.begin();
  //ETH.config(IPAddress(192, 168, 1, 9),IPAddress(192, 168, 1, 1),IPAddress(255, 255, 255, 0),IPAddress(192, 168, 1, 1),IPAddress(192, 168, 1, 1));

}

void initialize()
{
  Serial.print("initializing:");

  Udp.begin(ART_NET_PORT);

  FastLED.addLeds<APA102, 2, clockPin , BGR>(leds, 0, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<APA102, 14, clockPin, BGR>(leds, NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<APA102, 15, clockPin, BGR>(leds, 2 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<APA102, 12, clockPin, BGR>(leds, 3 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<APA102, 13, clockPin, BGR>(leds, 4 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<APA102, 32, clockPin, BGR>(leds, 5 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<APA102, 33, clockPin, BGR>(leds, 6 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  //FastLED.addLeds<APA102, 39, clockPin, BGR>(leds, 7 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  //FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed

  FastLED.setBrightness(brightness);

  initTest();
}

void CambiaColorLed(OSCMessage &msg) {
  uint8_t barra = msg.getInt(0);
  uint8_t hue = msg.getInt(1);
  uint8_t sat = msg.getInt(2);
  uint8_t val = msg.getInt(3);
  Serial.print("barra:");
  Serial.print(barra);
  Serial.print(" ");
  Serial.print("hue:");
  Serial.print(hue);
  Serial.print(" ");
  Serial.print("sat:");
  Serial.print(sat);
  Serial.print(" ");
  Serial.print("val:");
  Serial.println(val);
  
  for (int i = 0 ; i < NUM_LEDS ; i++) {
    leds[i] = CHSV(hue, sat, val);
  }
    FastLED.show(); 
}

void receiveMessage() {
  OSCMessage inmsg;
  int size = Udp.parsePacket();

  if (size > 0) {
    while (size--) {
      inmsg.fill(Udp.read());
    }
    if (!inmsg.hasError()) {
      inmsg.dispatch("/led", CambiaColorLed);
      packetCounter++;
    } 
    else { auto error = inmsg.getError(); }
  }
}

void loop() {
  receiveMessage();
  delay(update_rate);

//  if (millis()  >= (DELAY_TIME + INTERVAL) && eth_connected) {
//    DELAY_TIME = millis(); // finished delay -- single shot, once only
//    Serial.print("frames: ");
//    Serial.print(frameCounter);
//    Serial.print("packets: ");
//    Serial.println(packetCounter);
//    frameCounter = 0;
//    packetCounter = 0;
//
//    Serial.print("ETH MAC: ");
//    Serial.print(ETH.macAddress());
//    Serial.print(", IPv4: ");
//    Serial.print(ETH.localIP());
//  }

}
