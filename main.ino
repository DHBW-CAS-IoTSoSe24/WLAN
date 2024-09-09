#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <EEPROM.h>
// #include <FastLED.h>
#include <PubSubClient.h>
#include <DHT.h>

#define LIGHTBARRIER_INDOOR              13         // Sensor "inside" the room
#define LIGHTBARRIER_OUTDOOR             12         // Sensor "outside" the room

#define LB_RED                           21
#define LB_GREEN                          1
#define LB_BLUE                           3

#define STATUS_RED                       18
#define STATUS_GREEN                      5
#define STATUS_BLUE                      19

// #define NUM_LEDS 1
// #define DATA_PIN 8

// CRGB leds[NUM_LEDS];

#define EEPROM_ADDR_VERSION               0         // Length:  1
#define EEPROM_ADDR_HEARTBEAT_ACTIVE      1         // Length:  1
#define EEPROM_ADDR_WIFI_SSID             8         // Length: 32
#define EEPROM_ADDR_WIFI_PASS            40         // Length: 32
#define EEPROM_ADDR_MQTT_SADDR           72         // Length: 64
#define EEPROM_ADDR_MQTT_SPORT          136         // Length:  4
#define EEPROM_ADDR_MQTT_TOPIC          140         // Length: 32
#define EEPROM_ADDR_MQTT_USER           172         // Length: 32
#define EEPROM_ADDR_MQTT_PASS           204         // Length: 32
#define EEPROM_ADDR_SENSOR_ID           236         // Length: 32

#define AP_SSID "ESP32-Sensor"
#define AP_PASS "esp32-sensor"

IPAddress apIP(8,8,8,8);
IPAddress netMsk(255,255,255,0);
DNSServer dnsServer;
WebServer server(80);

WiFiClient espClient;
PubSubClient client(espClient);

// Definiere den DHT-Sensor
#define DHTPIN 32
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

hw_timer_t *timer = NULL;


struct app_flags {
  bool send_heartbeat;
  bool mqttavail;
  bool dht11avail;
  bool lightbarrieravail;
};

struct app {
  struct app_flags flags;
  uint32_t systick;
  String ssid;
  String pass;
  String mqttserveraddr;
  uint16_t mqttserverport;
  String mqttservertopic;
  String mqttserveruser;
  String mqttserverpass;
  String sensorid;
  float last_temperature;
  float last_humidity;
  uint8_t last_lbstate;
};

struct app app = {
  .flags = {
    .send_heartbeat = false,
    .mqttavail = false,
    .dht11avail = true,             // Change if DHT11 not available
    .lightbarrieravail = true       // Change if Light Barrier not available
  },
  .systick = 0u,
  .ssid = "",                       // 32 Byte
  .pass = "",                       // 32 Byte
  .mqttserveraddr = "",             // 64 Byte
  .mqttserverport = 1883,
  .mqttservertopic = "",            // 32 Byte
  .mqttserveruser = "",             // 32 Byte
  .mqttserverpass = "",             // 32 Byte
  .sensorid = "",                   // 16 Byte
  .last_temperature = 0.0,
  .last_humidity = 0.0
};


static const char responsePortal[] PROGMEM = R"===(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32-Sensor</title>
  <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=0" />
</head>
<body>
  <h1>ESP32-Sensor Einstellungen</h1>
  <p>Hier kann der ESP32-Sensor konfiguriert werden.</p>
  <form action="/setup">
    <label for="ssid">WiFi-SSID: </label>
    <input type="text" id="ssid" name="ssid" maxlength="32">
    <br>
    <br>
    <label for="pass">WiFi-Pass: </label>
    <input type="password" id="pass" name="pass" maxlength="32">
    <br>
    <br>
    <br>
    <br>
    <label for="mqttserveraddr">MQTT-Server Adresse: </label>
    <input type="text" id="mqttserveraddr" name="mqttserveraddr" maxlength="64">
    <br>
    <br>
    <label for="mqttserverport">MQTT-Server Port: </label>
    <input type="number" id="mqttserverport" name="mqttserverport">
    <br>
    <br>
    <label for="mqttservertopic">MQTT-Server Topic: </label>
    <input type="text" id="mqttservertopic" name="mqttservertopic" maxlength="32">
    <br>
    <br>
    <label for="mqttserveruser">MQTT-Server User: </label>
    <input type="text" id="mqttserveruser" name="mqttserveruser" maxlength="32">
    <br>
    <br>
    <label for="mqttserverpass">MQTT-Server Pass: </label>
    <input type="password" id="mqttserverpass" name="mqttserverpass" maxlength="32">
    <br>
    <br>
    <input type="submit" value="Speichern">
  </form>
</body>
</html>
)===";

void handleRoot() {
  server.send(200, "text/plain", "ESP32 WiFi Sensor");
}

void handleNotFound() {
  server.sendHeader("Location", "/portal");
  server.send(302, "text/plain", "Weiterleiten..");
}

void handleSetup() {
  if (server.hasArg("ssid")) {
    String ssid = server.arg("ssid");
    if (ssid.length() > 0) {
      app.ssid = ssid;
      EEPROM.writeString(EEPROM_ADDR_WIFI_SSID, app.ssid);
    }
  }

  if (server.hasArg("pass")) {
    String pass = server.arg("pass");
    if (pass.length() > 0) {
      app.pass = pass;
      EEPROM.writeString(EEPROM_ADDR_WIFI_PASS, app.pass);
    }
  }

  if (app.ssid.length() > 4) {
    WiFi.begin(app.ssid, app.pass);
  }

  if (server.hasArg("mqttserveraddr")) {
    String mqttserveraddr = server.arg("mqttserveraddr");
    if (mqttserveraddr.length() > 0) {
      app.mqttserveraddr = mqttserveraddr;
      EEPROM.writeString(EEPROM_ADDR_MQTT_SADDR, app.mqttserveraddr);
    }
  }

  if (server.hasArg("mqttserverport")) {
    String mqttserverport = server.arg("mqttserverport");
    if (mqttserverport.toInt() != 0) {
      app.mqttserverport = mqttserverport.toInt();
      EEPROM.writeInt(EEPROM_ADDR_MQTT_SPORT, app.mqttserverport);
    }
  }

  if (server.hasArg("mqttservertopic")) {
    String mqttservertopic = server.arg("mqttservertopic");
    if (mqttservertopic.length() > 0) {
      app.mqttservertopic = mqttservertopic;
      EEPROM.writeString(EEPROM_ADDR_MQTT_TOPIC, app.mqttservertopic);
    }
  }

  if (server.hasArg("mqttserveruser")) {
    String mqttserveruser = server.arg("mqttserveruser");
    if (mqttserveruser.length() > 0) {
      app.mqttserveruser = mqttserveruser;
      EEPROM.writeString(EEPROM_ADDR_MQTT_USER, app.mqttserveruser);
    }
  }

  if (server.hasArg("mqttserverpass")) {
    String mqttserverpass = server.arg("mqttserverpass");
    if (mqttserverpass.length() > 0) {
      app.mqttserverpass = mqttserverpass;
      EEPROM.writeString(EEPROM_ADDR_MQTT_PASS, app.mqttserverpass);
    }
  }

  if (app.mqttserveraddr.length() > 0 && app.mqttserverport != 0 && app.mqttservertopic.length() > 0) {
    client.setClient(espClient);
    client.setServer(app.mqttserveraddr.c_str(),app.mqttserverport);
    app.flags.mqttavail = true;
  }

  EEPROM.commit();
  server.send(200, "text/plain", "Einstellungen übermittelt.");
}

void setLBStatColor(int r, int g, int b) {
  analogWrite(LB_RED, r);
  analogWrite(LB_GREEN, g);
  analogWrite(LB_BLUE, b);
}


void led_off() {
  digitalWrite(STATUS_RED, LOW);
  digitalWrite(STATUS_GREEN, LOW);
  digitalWrite(STATUS_BLUE, LOW);
}

void led_show_red() {
  // leds[0].r = 10;
  // leds[0].g = 0;
  // leds[0].b = 0;
  // FastLED.show();
  digitalWrite(STATUS_RED, HIGH);
  digitalWrite(STATUS_GREEN, LOW);
  digitalWrite(STATUS_BLUE, LOW);
}
void led_show_green() {
  // leds[0].r = 0;
  // leds[0].g = 10;
  // leds[0].b = 0;
  // FastLED.show();
  digitalWrite(STATUS_RED, LOW);
  digitalWrite(STATUS_GREEN, HIGH);
  digitalWrite(STATUS_BLUE, LOW);
}
void led_show_blue() {
  // leds[0].r = 0;
  // leds[0].g = 0;
  // leds[0].b = 10;
  // FastLED.show();
  digitalWrite(STATUS_RED, LOW);
  digitalWrite(STATUS_GREEN, LOW);
  digitalWrite(STATUS_BLUE, HIGH);
}


// tick the clock (increment time by one second)
void tick() {
  app.systick += 1;
}


void setup() {

  // FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);

  pinMode(STATUS_RED, OUTPUT);
  pinMode(STATUS_GREEN, OUTPUT);
  pinMode(STATUS_BLUE, OUTPUT);
  
  led_show_red();

  pinMode(LB_RED, OUTPUT);
  pinMode(LB_GREEN, OUTPUT);
  pinMode(LB_BLUE, OUTPUT);

  pinMode(LIGHTBARRIER_INDOOR, INPUT);
  pinMode(LIGHTBARRIER_OUTDOOR, INPUT);

  reset_lightbarrier_sensor();

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(AP_SSID, AP_PASS);

  dnsServer.setTTL(5);
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", apIP);

  server.on("/", handleRoot);
  server.on("/portal", []() {
    server.send(200, "text/html", responsePortal);
  });
  server.on("/setup", handleSetup);
  server.onNotFound(handleNotFound);
  server.begin();

  timer = timerBegin(1000000);
  timerAttachInterrupt(timer, &tick);
  timerAlarm(timer, 1000000, true, 0);

  EEPROM.begin(256);
  app.ssid = EEPROM.readString(EEPROM_ADDR_WIFI_SSID);
  app.pass = EEPROM.readString(EEPROM_ADDR_WIFI_PASS);
  app.mqttserveraddr = EEPROM.readString(EEPROM_ADDR_MQTT_SADDR);
  app.mqttserverport = EEPROM.readUInt(EEPROM_ADDR_MQTT_SPORT);
  app.mqttservertopic = EEPROM.readString(EEPROM_ADDR_MQTT_TOPIC);
  app.mqttserveruser = EEPROM.readString(EEPROM_ADDR_MQTT_USER);
  app.mqttserverpass = EEPROM.readString(EEPROM_ADDR_MQTT_PASS);
  app.sensorid = EEPROM.readString(EEPROM_ADDR_SENSOR_ID);

  if (app.sensorid.length() != 16) {
    char hexstring[17];
    for (int i = 0; i < 16; i++) {
      int randomValue = random(0, 16);
      hexstring[i] = "0123456789ABCDEF"[randomValue];
    }
    app.sensorid.concat(hexstring);
    EEPROM.writeString(EEPROM_ADDR_SENSOR_ID, app.sensorid);
  }

  if (app.ssid.length() > 4) {
    WiFi.begin(app.ssid, app.pass);
  }

  if (app.mqttserveraddr.length() > 0 && app.mqttserverport != 0 && app.mqttservertopic.length() > 0) {
    client.setClient(espClient);
    client.setServer(app.mqttserveraddr.c_str(),app.mqttserverport);
    app.flags.mqttavail = true;
  }

  if (app.flags.dht11avail == true) {
    dht.begin();
  }

}

void updateLED() {
  if (WiFi.status() == WL_CONNECTED && client.connected()) {
    led_show_green();
  }
  else if (WiFi.status() == WL_CONNECTED && !client.connected()) {
    led_show_blue();
  }
  else {
    led_show_red();
  }
}

void loop() {
  if (app.flags.mqttavail) {
    if (!client.connected()) {
      client.connect("ESP32-WLAN", app.mqttserveruser.c_str(), app.mqttserverpass.c_str());
    }
    else {
      client.loop();
    }
  }

  if (app.systick % 10 == 0) {
    String send_to = "wlan/" + app.mqttservertopic + "/" + app.sensorid;

    if (app.flags.send_heartbeat) {
      // Check time
    }

    if (client.connected()) {
      if (app.flags.dht11avail) {
        float temperature = dht.readTemperature();
        float humidity = dht.readHumidity();

        if (!isnan(temperature) && !isnan(humidity)) {
          app.last_temperature = temperature;
          app.last_humidity = humidity;
          String toSend = "{\"t\":" +  String(temperature) + ", \"h\":" + String(humidity) + "}";
          client.publish(send_to.c_str(), toSend.c_str());
        }
      }
    }

  }

  if (app.flags.lightbarrieravail) {
    String send_to = "wlan/" + app.mqttservertopic + "/" + app.sensorid;
    changeState(is_barrier_open(LIGHTBARRIER_INDOOR, 10), is_barrier_open(LIGHTBARRIER_OUTDOOR, 10));

    switch(get_sensor_state()) {
      case 0:
        // Open
        setLBStatColor(0, 0, 0);
        if (app.last_lbstate != 0) {
          String toSend = "{\"lb\":0}";
          client.publish(send_to.c_str(), toSend.c_str());
        }
        app.last_lbstate = 0;
        break;
      case 1:
        // Outgoing
        setLBStatColor(255, 0, 0);
        if (app.last_lbstate != 1) {
          String toSend = "{\"lb\":1}";
          client.publish(send_to.c_str(), toSend.c_str());
        }
        app.last_lbstate = 1;
        break;
      case 2:
        // Incoming
        setLBStatColor(0, 255, 0);
        if (app.last_lbstate != 2) {
          String toSend = "{\"lb\":2}";
          client.publish(send_to.c_str(), toSend.c_str());
        }
        app.last_lbstate = 2;
        break;
      case 3:
        // Blocked
        setLBStatColor(0, 0, 255);
        if (app.last_lbstate != 3) {
          String toSend = "{\"lb\":3}";
          client.publish(send_to.c_str(), toSend.c_str());
        }
        app.last_lbstate = 3;
        break;
    }
  }

  server.handleClient();

  updateLED();
  delay(10);
}
