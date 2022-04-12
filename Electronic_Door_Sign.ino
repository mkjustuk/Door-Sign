//Install [Adafruit_NeoPixel_Library](https://github.com/adafruit/Adafruit_NeoPixel) first.
//SimpleTimer used here is - https://playground.arduino.cc/Code/SimpleTimer/#Download
#include <Adafruit_NeoPixel.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <SimpleTimer.h>

#define PIN   D4
#define LED_NUM 7

/////////////////////////////////
//USER CONFIGURED SECTION START//
//UPDATE THESE TO BE CORRECT BEFORE COMPILING

//Update the first two MAP values in line 136 to get an accurate % battery output

const char* ssid = "SSID"; // your router ssid here
const char* password = "PASS"; // your router password here
const char* mqtt_server = "x.x.x.x"; // the ip address of your MQTT broker
const int mqtt_port = 1883;  // only change if you use a non-standard port
const char *mqtt_user = "USER"; // the username you have set for a mqtt user
const char *mqtt_pass = "MQTTPASS";  // mqtt user password
const char *mqtt_client_name = "OfficeSign"; // Must be unique, client connections can't have the same connection name

//USER CONFIGURED SECTION END//
///////////////////////////////

WiFiClient espClient;
PubSubClient client(espClient);
SimpleTimer timer;

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel leds = Adafruit_NeoPixel(LED_NUM, PIN, NEO_GRB + NEO_KHZ800);

//Setup the constants and variables
bool boot = true;
int led_show = 1; //This will be 1 if active, 0 if not
int meeting_status = 0; //This will be 1 if in meeting, 0 if not
unsigned int raw = 0;
float volt = 0.0; //Ready for reading the battery voltage

//Manage disconnects and reconnects to MQTT
void reconnect()
{
  // Loop until we're reconnected
  int retries = 0;
  while (!client.connected())
  {
    if (retries < 100)
    {
      // Attempt to connect
      if (client.connect(mqtt_client_name, mqtt_user, mqtt_pass, "OfficeSign/status", 0, 0, "Unknown"))
      {
        // Once connected, publish an announcement...
        if (boot == false)
        {
          client.publish("checkIn/OfficeSign", "Reconnected");
        }
        if (boot == true)
        {
          client.publish("checkIn/OfficeSign", "Rebooted");
          boot = false;
        }
        // ... and resubscribe
        client.subscribe("OfficeSign/meeting");
      }
      else
      {
        retries++;
        // Wait 5 seconds before retrying
        delay(5000);
      }
    }
    if (retries >= 100)
    {
      ESP.restart();
    }
  }
}

//We check for four payloads on the topic OfficeSign/meeting
//We are in a meeting or not but also sleeping or awake to save power overnight or when the device is idle
void callback(char* topic, byte* payload, unsigned int length)
{
  String newTopic = topic;
  payload[length] = '\0';
  String newPayload = String((char *)payload);
  if (newTopic == "OfficeSign/meeting")
  {
    if (newPayload == "meeting")
    {
      meeting_status = 1;
    }
    if (newPayload == "nomeeting")
    {
      meeting_status = 0;
    }
    if (newPayload == "sleeping")
    {
      led_show = 0;
    }
    if (newPayload == "awake")
    {
      led_show = 1;
    }
  }
}

// Wifi, MQTT and timer setup

void setup()
{
  WiFi.mode(WIFI_STA);
  WiFi.hostname("OFFICE_SIGN");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  timer.setInterval(60000, sendInfoSign);

  leds.begin(); // This initializes the NeoPixel library.
  pinMode(A0, INPUT); //Set the A0 pin to read the battery voltage (we've soldered the jumper J2 on the battery shield to allow this)
}

/////
//Functions
/////

//Read, package and then send via the topic OfficeSign/status, the voltage of the battery at this time
void sendInfoSign()
{
  volt = map(analogRead(A0),600,970,0,100);  //You may need to update these for different battery capacities - this is based on 4000mah
  char charBatt[] = "00.0";
  dtostrf(volt, 4, 1, charBatt);
  client.publish("OfficeSign/status", charBatt); //Send the voltage % on an MQTT status topic
}

//initialize the led arrangement
void led_set(uint8 R, uint8 G, uint8 B) {
  for (int i = 1; i < LED_NUM; i++) {
    leds.setPixelColor(i, leds.Color(R, G, B));
    leds.show();
    delay(175);
  }
}

void no_meeting() {
  led_set(0, 10, 0);//green circle
  led_set(0, 0, 0);//off
  delay(250);
}

void meeting() {  //red cross
  leds.setPixelColor(0, leds.Color(10, 0, 0));
  leds.setPixelColor(2, leds.Color(10, 0, 0));
  leds.setPixelColor(3, leds.Color(10, 0, 0));
  leds.setPixelColor(5, leds.Color(10, 0, 0));
  leds.setPixelColor(6, leds.Color(10, 0, 0));
  leds.show();
  delay(150); //off
  leds.setPixelColor(0, leds.Color(0, 0, 0));
  leds.setPixelColor(2, leds.Color(0, 0, 0));
  leds.setPixelColor(3, leds.Color(0, 0, 0));
  leds.setPixelColor(5, leds.Color(0, 0, 0));
  leds.setPixelColor(6, leds.Color(0, 0, 0));
  leds.show();
  delay(550);
}

void noleds() {  //'sleep' mode where the LEDs are off
  led_set(0, 0, 0);//off
  leds.clear();
}

void loop()
{
  if (!client.connected()) //check for MQTT connection
  {
    reconnect();
  }
  client.loop();
  timer.run(); //send voltage back

  if (meeting_status == 0 && led_show == 1)
  {
    no_meeting();  //green circle
  }
  if (meeting_status == 1 && led_show == 1)
  {
    meeting();  //red cross
  }
  if (led_show == 0)
  {
    noleds(); //off
  }
}
