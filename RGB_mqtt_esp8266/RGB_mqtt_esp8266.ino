#include <RGBConverterLib.h>
//Check where comes from and if it is in use in project

/***************************************************
  Adafruit MQTT Library ESP8266 Example

  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino

  Works great with Adafruit's Huzzah ESP board & Feather
  ----> https://www.adafruit.com/product/2471
  ----> https://www.adafruit.com/products/2821

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Tony DiCola for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>


#include <FastLED.h>
//#define FASTLED_ESP8266_RAW_PIN_ORDER
#define NUM_LEDS 8
#define DATA_PIN 8
CRGBArray<NUM_LEDS> leds;
/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "Telenor1573vik"
#define WLAN_PASS       "Hvitlauk8Walkover9Anfinn9"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "10.0.0.15"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "pi"
#define AIO_KEY         "377yussA"


#define TFT_DC D1 // pin of your choice
#define TFT_RST D2 // pin of your choice
#define TFT_MOSI D7 // fixed pin
#define TFT_SCLK D5 // fixed pin
#define TFT_CS D6 // fixed pin




const char* host = "HUE2WS2812";
/************ Global State (you don't need to change this!) ******************/
const char* ssid = "Telenor1573vik";//"YOUR_SSID";
const char* password = "Hvitlauk8Walkover9Anfinn9";//"YOUR_PASSWORD";
int repeats = 3;


int oldhue = 0;
int oldsat = 0;
int oldval = 0;
// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiClientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish receivedData = Adafruit_MQTT_Publish(&mqtt, "/receivedData");
Adafruit_MQTT_Publish effectData = Adafruit_MQTT_Publish(&mqtt, "/effectData");

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe hue = Adafruit_MQTT_Subscribe(&mqtt, "myColor2/h");
Adafruit_MQTT_Subscribe sat = Adafruit_MQTT_Subscribe(&mqtt, "myColor2/s");
Adafruit_MQTT_Subscribe val = Adafruit_MQTT_Subscribe(&mqtt, "myColor2/v");
Adafruit_MQTT_Subscribe newStatus = Adafruit_MQTT_Subscribe(&mqtt, "myColor2/status");
Adafruit_MQTT_Subscribe effectStatus = Adafruit_MQTT_Subscribe(&mqtt, "myColor2/effect");
/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.println(F("Adafruit MQTT demo"));
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

  
  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
WiFi.mode(WIFI_STA);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&hue);
  mqtt.subscribe(&sat);
  mqtt.subscribe(&val);
  mqtt.subscribe(&effectStatus);
  

  ArduinoOTA.setHostname(host);

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  /* setup the OTA server */
  ArduinoOTA.begin();
  Serial.println("Ready");



    uint8_t red = 50;
  uint8_t green = 100;
  uint8_t blue = 150;
  double hue, saturation, lighting, value;
  
  String hex = "010509";
  RGBConverter::HexToRgb(hex, red, green, blue);
  RGBConverter::RgbToHsv(red, green, blue, hue, saturation, value);
  setNeopixel(hue,saturation,value);


}

int lightStatus;
uint32_t x=0;
int recvHue, recvSat,recvVal;
void loop() {

  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

  Adafruit_MQTT_Subscribe *subscription;

  String lastMsg; 
  while ((subscription = mqtt.readSubscription(100))) {
    if (subscription == &hue) {
      //lastMsg = (char *)onoffbutton.lastread;
      Serial.print(F("Got: "));
      Serial.println((char *)hue.lastread);
      recvHue = round(atoi((char *)hue.lastread));
      // Now we can publish stuff!
      Serial.print(F("\nRecieved "));
      Serial.print(recvHue);
      Serial.print("...");
      if (! receivedData.publish(recvHue)) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("OK!"));
      }
    }
    else if (subscription == &sat) {
      //lastMsg = (char *)onoffbutton.lastread;
      Serial.print(F("Got: "));
      Serial.println((char *)sat.lastread);
      recvSat = round(atoi((char *)sat.lastread));
    }
    else if (subscription == &val) {
      //lastMsg = (char *)onoffbutton.lastread;
      Serial.print(F("Got: "));
      Serial.println((char *)val.lastread);
      recvVal = round(atoi((char *)val.lastread));
    }
    else if (subscription == &effectStatus) {
      
      effectData.publish((char *)effectStatus.lastread);
      setNeopixelEffect(true, repeats);
      
    }    
    else if (subscription == &newStatus) {
      int tmpIn = round(atoi((char *)newStatus.lastread));//(char *)newStatus.lastread;//atoi((char *)newStatus.lastread);
      lightStatus = tmpIn;               
      /*
      if (tmpIn == 0){
        lightStatus = false;
      }
      else{//(tmpIn == 1){
        lightStatus = true;
      }
      */
      if (! receivedData.publish(tmpIn)) {
        Serial.println(F("Failed"));
      }
      else {
        Serial.println(F("OK!"));
      }
    }
    else{}
    if(recvHue != oldhue || recvSat != oldsat || recvVal != oldval){
      
      setNeopixel(recvHue,recvSat,recvVal);//,lightStatus);
    }
  }

  
  ArduinoOTA.handle();
  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  */
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}

void setNeopixel(int inHue, int inSat, int inVal){//,bool lightStatus2){
    if(inHue != oldhue || inSat != oldsat || inVal != oldval){
      for(int i = 0; i < NUM_LEDS; i++) {   
      // fade everything out
      //leds.fadeToBlackBy(20);
    
      // let's set an led value
      leds[i] = CHSV(inHue,inSat,inVal);//inSat,inVal);
    
      // now, let's first 20 leds to the top 20 leds, 
      //leds(NUM_LEDS/2,NUM_LEDS-1) = leds(NUM_LEDS/2 - 1 ,0);
      FastLED.delay(33);
    }
    oldhue = inHue;
    oldsat = inSat;
    oldval = inVal;
  }
}
void setNeopixelEffect(bool effectStatus,int repeats){
  static uint8_t huecount = 0;
  while (huecount < repeats){
    for(int j = 0 ; j<255;j++){
      for(int i = 0; i < NUM_LEDS/2; i++) {   
        // fade everything out
        //leds.fadeToBlackBy(20);
    
        // let's set an led value
        leds[i] = CHSV(j,255,255);
    
        // now, let's first 20 leds to the top 20 leds, 
        leds(NUM_LEDS/2,NUM_LEDS-1) = leds(NUM_LEDS/2 - 1 ,0);
        FastLED.delay(33);
      }
    }
    huecount++;
  }
} 
