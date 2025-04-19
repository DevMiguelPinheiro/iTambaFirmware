#include <WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// MQTT Broker
const char* mqtt_server = "YOUR_MQTT_BROKER";
const int mqtt_port = 1883;

// MQTT Topics
const char* time_topic = "iTamba/sensor/set_time";
const char* test_on_topic = "iTamba/feeder/teste/on";
const char* test_off_topic = "iTamba/feeder/teste/off";
const char* temp_topic = "iTamba/temperature";

// Pin definitions
const int relay1_pin = 25;
const int relay2_pin = 26;
const int temp_sensor_pin = 15;

// Default feeding times (6:00, 12:00, 18:00)
const int default_hours[3] = {6, 12, 18};
const int default_minutes[3] = {0, 0, 0};

// Variables to store feeding times
int feeding_hours[3] = {6, 12, 18};  // Initialize with default times
int feeding_minutes[3] = {0, 0, 0};  // Initialize with default times

// Temperature sensor setup
OneWire oneWire(temp_sensor_pin);
DallasTemperature sensors(&oneWire);

// NTP setup
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

WiFiClient espClient;
PubSubClient client(espClient);

// Temperature reading interval (in milliseconds)
const unsigned long temp_interval = 30000; // 30 seconds
unsigned long last_temp_read = 0;

// Time synchronization interval (in milliseconds)
const unsigned long time_sync_interval = 3600000; // 1 hour
unsigned long last_time_sync = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (String(topic) == time_topic) {
    // Parse the time string (format: "HH:MM,HH:MM,HH:MM")
    int index = 0;
    int lastIndex = 0;
    for (int i = 0; i < 3; i++) {
      index = message.indexOf(',', lastIndex);
      String timeStr;
      if (index == -1) {
        timeStr = message.substring(lastIndex);
      } else {
        timeStr = message.substring(lastIndex, index);
        lastIndex = index + 1;
      }
      
      // Parse hours and minutes
      int colonIndex = timeStr.indexOf(':');
      if (colonIndex != -1) {
        feeding_hours[i] = timeStr.substring(0, colonIndex).toInt();
        feeding_minutes[i] = timeStr.substring(colonIndex + 1).toInt();
      }
    }
    Serial.println("Feeding times updated");
  }
  else if (String(topic) == test_on_topic && message == "1") {
    digitalWrite(relay1_pin, HIGH);
    digitalWrite(relay2_pin, HIGH);
    Serial.println("Test ON - Relays activated");
  }
  else if (String(topic) == test_off_topic) {
    digitalWrite(relay1_pin, LOW);
    digitalWrite(relay2_pin, LOW);
    Serial.println("Test OFF - Relays deactivated");
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32FishFeeder")) {
      Serial.println("connected");
      client.subscribe(time_topic);
      client.subscribe(test_on_topic);
      client.subscribe(test_off_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  // Initialize temperature sensor
  sensors.begin();

  // Set relay pins as output
  pinMode(relay1_pin, OUTPUT);
  pinMode(relay2_pin, OUTPUT);
  
  // Initialize relays to LOW
  digitalWrite(relay1_pin, LOW);
  digitalWrite(relay2_pin, LOW);

  setup_wifi();
  
  // Initialize NTP client
  timeClient.begin();
  timeClient.setTimeOffset(-3 * 3600); // Adjust for your timezone (e.g., -3 for Brazil)
  timeClient.update();
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void syncTime() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - last_time_sync >= time_sync_interval) {
    timeClient.update();
    last_time_sync = currentMillis;
    Serial.println("Time synchronized with NTP server");
  }
}

void checkFeedingTime() {
  syncTime();
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  
  for (int i = 0; i < 3; i++) {
    if (currentHour == feeding_hours[i] && currentMinute == feeding_minutes[i]) {
      // Activate relays for feeding
      digitalWrite(relay1_pin, HIGH);
      digitalWrite(relay2_pin, HIGH);
      delay(1000); // Keep relays on for 1 second
      digitalWrite(relay1_pin, LOW);
      digitalWrite(relay2_pin, LOW);
      Serial.println("Feeding time!");
    }
  }
}

void readAndPublishTemperature() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - last_temp_read >= temp_interval) {
    sensors.requestTemperatures();
    float temperature = sensors.getTempCByIndex(0);
    
    if (temperature != DEVICE_DISCONNECTED_C) {
      char tempString[8];
      dtostrf(temperature, 6, 2, tempString);
      client.publish(temp_topic, tempString);
      Serial.print("Temperature published: ");
      Serial.println(temperature);
    }
    
    last_temp_read = currentMillis;
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  checkFeedingTime();
  readAndPublishTemperature();
  delay(1000); // Check every second
} 