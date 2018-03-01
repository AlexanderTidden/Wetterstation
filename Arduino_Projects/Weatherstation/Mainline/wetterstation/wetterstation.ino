
/*******************************OLED**********************************/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET LED_BUILTIN //4 *no reset PIN on OLED display
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

/*******************************DHT**********************************/


#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include <Adafruit_BMP280.h>


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
#include "user_interface.h"

/************************************* DEBUG *********************************/
#define DEBUG 1
/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "HZN246467407"
#define WLAN_PASS       "yhgewxid6xDc"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "AlexanderTidden"
#define AIO_KEY         "c4c79f4cec504cf699a50ffdced7c511"

/************************* DHT sensor pinning and version *********************************/
#define DHTPIN 2     // what digital pin we're connected to

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

/************************* Initialize DHT sensor *********************************/

DHT dht(DHTPIN, DHTTYPE);


#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11 
#define BMP_CS 10

Adafruit_BMP280 bme; // I2C
//Adafruit_BMP280 bme(BMP_CS); // hardware SPI
//Adafruit_BMP280 bme(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);

/************************* switchable supply voltage for sensor *********************************/
const int VSuppPin =  0;      // the number of the pin to control the transitor

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish temp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");

Adafruit_MQTT_Publish humi = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");

Adafruit_MQTT_Publish batt = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/battery");

Adafruit_MQTT_Publish reason = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/resetreason");

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff");

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

// Mode of internal ADC
ADC_MODE(ADC_VCC);

void setup() {
  #if (DEBUG ==1)
  //initialize serial
  Serial.begin(115200);
  // Wait for serial to initialize.
  while(!Serial) { }
  Serial.println("serial initialized");
 #endif
 
  pinMode(VSuppPin, OUTPUT);
  delay(10);
  digitalWrite(VSuppPin, HIGH);
  
// initialize DHT Sensor
  dht.begin();

  if (!bme.begin()) {  
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }
  
 #if (DEBUG ==1)
  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
 #endif
 
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
 #if (DEBUG ==1)
    Serial.print(".");
 #endif   
  }
 #if (DEBUG ==1)
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
 #endif   
 
  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&onoffbutton);


    // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done
  
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(2000);             // wait 2s
  display.clearDisplay();  // Clear the buffer.
}

uint32_t x=0;

void loop() {

  rst_info *rsti;
  rsti = ESP.getResetInfoPtr();
  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

#if (DEBUG ==1)
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
#endif 
 
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);
  
#if (DEBUG ==1)
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
#endif 

//read VCC from internal ADC
float battery = ESP.getVcc();
#if (DEBUG ==1)
  Serial.print("Battery: ");
  Serial.print(battery);
#endif 


    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.println(" *C");
    
    Serial.print("Pressure = ");
    Serial.print(bme.readPressure());
    Serial.println(" Pa");

    Serial.print("Approx altitude = ");
    Serial.print(bme.readAltitude(1013.25)); // this should be adjusted to your local forcase
    Serial.println(" m");
    
    Serial.println();
    delay(2000);


  //clear old values from display
  display.clearDisplay();

    // display values  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Humidity: ");
  display.print(h);
  display.println(" %");
  display.println("Temperature: ");
  display.print(t);
  display.println(" *C");
  display.display();

  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &onoffbutton) {
      #if (DEBUG ==1)
      Serial.print(F("Got: "));
      Serial.println((char *)onoffbutton.lastread);
      #endif
    }
  }

  // Now we can publish stuff!
  #if (DEBUG ==1)
  Serial.print(F("\nSending temperature val "));
  Serial.print(t);
  Serial.print("...");
  #endif
  if (! temp.publish(t)) {
    #if (DEBUG ==1)
    Serial.println(F("Failed"));
    #endif
  } else {
    #if (DEBUG ==1)
    Serial.println(F("OK!"));
    #endif
  }

#if (DEBUG ==1)
  Serial.print(F("\nSending humidity val "));
  Serial.print(h);
  Serial.print("...");
  #endif
  if (! humi.publish(h)) {
    #if (DEBUG ==1)
    Serial.println(F("Failed"));
    #endif
  } else {
    #if (DEBUG ==1)
    Serial.println(F("OK!"));
    #endif
  }

#if (DEBUG ==1)
  Serial.print(F("\nSending battery val "));
  Serial.print(battery);
  Serial.print("...");
#endif
  if (! batt.publish(battery)) {
    #if (DEBUG ==1)
    Serial.println(F("Failed"));
    #endif
  } else {
    #if (DEBUG ==1)
    Serial.println(F("OK!"));
    #endif
  }

#if (DEBUG ==1)
  Serial.print(F("\nSending reset reason"));
  Serial.println(String("ResetInfo.reason = ") + rsti->reason);
  Serial.print("...");
#endif
  if (! reason.publish(rsti->reason)) {
    #if (DEBUG ==1)
    Serial.println(F("Failed"));
    #endif
  } else {
    #if (DEBUG ==1)
    Serial.println(F("OK!"));
    #endif
  }
  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }

  delay(1000);
  display.clearDisplay();
  //switch off voltage
  digitalWrite(VSuppPin, LOW); 

// go to deep sleep mode
#if (DEBUG == 1)
   Serial.println("Going into deep sleep for 300 seconds");
#endif
   ESP.deepSleep(10e6); // 1e6 is 1 second
   delay(100);
    
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
