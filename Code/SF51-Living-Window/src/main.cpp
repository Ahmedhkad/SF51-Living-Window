#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>

#include "settings.h" //GPIO NodeMCU good-pins
#include "secret.h"   //Wifi and MQTT server info

const char *ssid = ssidWifi;
const char *password = passWifi;
const char *mqtt_server = mqttURL;
const char *deviceName = mqttClient;

WiFiClient espClient;
PubSubClient client(espClient);

StaticJsonDocument<100> doc;
StaticJsonDocument<100> docLDR;
StaticJsonDocument<100> updater;

int device;
int valuejson;
int datajson;

boolean Motor1DOWNstate = true;
boolean Motor2DOWNstate = true;
boolean Motor3DOWNstate = true;

unsigned long currentMillis;
int lastCount = 0;
int count = 0;
unsigned long WifiDelayMillis = 0;
const long WifiDelayInterval = 5500; // interval to check wifi and mqtt
unsigned long LDRDelayMillis = 0;
const long LDRDelayInterval = 60000; // interval to check wifi and mqtt

void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.hostname(deviceName); // DHCP Hostname (useful for finding device for static lease)
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
  {
    Serial.println("STA Failed to configure");
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(300);
    Serial.print(".");
  }

  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length)
{

  for (unsigned i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  DeserializationError error = deserializeJson(doc, payload);
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
  // Print the values to data types
  device = doc["device"].as<unsigned int>();
  valuejson = doc["value"].as<unsigned int>();
  datajson = doc["data"].as<unsigned int>();

  switch (device)
  {
  case 1:
    if (valuejson == 1 && Motor1DOWNstate) // Motor 1 UP
    {
      digitalWrite(Motor1Enable, HIGH);
      digitalWrite(Motor1, LOW);
      digitalWrite(Middle, HIGH);
      client.publish("Living-Window-Motor1", "UP");
      Motor1DOWNstate = false;
      client.publish("Living-Window-Motor1-Dstate", "OFF");
    }
    else if (valuejson == 2 && !Motor1DOWNstate) // Motor 1 DOWN
    {
      digitalWrite(Motor1Enable, HIGH);
      digitalWrite(Motor1, HIGH);
      digitalWrite(Middle, LOW);
      client.publish("Living-Window-Motor1", "DOWN");
      Motor1DOWNstate = true;
      client.publish("Living-Window-Motor1-Dstate", "ON");
    }
    else if (valuejson == 3) // Motor 1 OFF
    {
      digitalWrite(Motor1Enable, LOW);
      client.publish("Living-Window-Motor1", "OFF");
      // client.publish("SF51-Projector-Screen",
      //                "{\"device\":\"5\",\"value\":\"0\"}"); // Power Supply OFF
    }
    break;
  case 2:
    if (valuejson == 1 && Motor2DOWNstate) // Motor 2 UP
    {
      digitalWrite(Motor2Enable, HIGH);
      digitalWrite(Motor2, LOW);
      digitalWrite(Middle, HIGH);
      client.publish("Living-Window-Motor2", "UP");
      Motor2DOWNstate = false;
      client.publish("Living-Window-Motor2-Dstate", "OFF");
    }
    else if (valuejson == 2 && !Motor2DOWNstate) // Motor 2 DOWN
    {
      digitalWrite(Motor2Enable, HIGH);
      digitalWrite(Motor2, HIGH);
      digitalWrite(Middle, LOW);
      client.publish("Living-Window-Motor2", "DOWN");
      Motor2DOWNstate = true;
      client.publish("Living-Window-Motor2-Dstate", "ON");
    }
    else if (valuejson == 3) // Motor 2 OFF
    {
      digitalWrite(Motor2Enable, LOW);
      client.publish("Living-Window-Motor2", "OFF");
    }
    break;
  case 3:
    if (valuejson == 1 && Motor3DOWNstate) // Motor 3 UP
    {
      digitalWrite(Motor3A, HIGH);
      digitalWrite(Motor3B, LOW);
      client.publish("Living-Window-Motor3", "UP");
      Motor3DOWNstate = false;
      client.publish("Living-Window-Motor3-Dstate", "OFF");
    }
    else if (valuejson == 2 && !Motor3DOWNstate) // Motor 3 DOWN
    {
      digitalWrite(Motor3A, LOW);
      digitalWrite(Motor3B, HIGH);
      client.publish("Living-Window-Motor3", "DOWN");
      Motor3DOWNstate = true;
      client.publish("Living-Window-Motor3-Dstate", "ON");
    }
    else if (valuejson == 3) // Motor 3 OFF
    {
      digitalWrite(Motor3A, LOW);
      digitalWrite(Motor3B, LOW);
      client.publish("Living-Window-Motor3", "OFF");
    }
    break;

  case 4:
    if (valuejson == 1) // LIGHT ON
    {
      digitalWrite(LED, HIGH);
      client.publish("Living-Window-LED", "ON");
    }
    else if (valuejson == 2) // Motor 1 UP
    {
      analogWrite(LED, datajson);
    }
    else if (valuejson == 3) // Motor 1 UP
    {
      digitalWrite(LED, LOW);
      client.publish("Living-Window-LED", "OFF");
    }
    break;
  case 5:
    if (valuejson == 1) // far  DOWN
    {
      char buffer2[200];
      docLDR["Living-Window-LDR"] = analogRead(LDR);
      serializeJson(docLDR, buffer2);
      client.publish("Living-Window-LDR", buffer2);
    }
    else if (valuejson == 2) // far Motor  UP
    {
      if (datajson > 1)
      {
        const long LDRDelayInterval = datajson * 1000;
      }
    }
    break;

  case 6:
    if (valuejson == 1) // Motor 1 UP
    {
      digitalWrite(Motor1Enable, HIGH);
      digitalWrite(Motor1, HIGH);
      digitalWrite(Middle, LOW);
      client.publish("Living-Window-Motor1", "UP");
    }
    else if (valuejson == 2) // Motor 1 DOWN
    {
      digitalWrite(Motor1Enable, HIGH);
      digitalWrite(Motor1, LOW);
      digitalWrite(Middle, HIGH);
      client.publish("Living-Window-Motor1", "DOWN");
    }
    else if (valuejson == 3) // Motor 1 OFF
    {
      digitalWrite(Motor1Enable, LOW);
      client.publish("Living-Window-Motor1", "OFF");
    }
    break;

  case 7:
    if (valuejson == 1) // Motor 2 UP
    {
      digitalWrite(Motor2Enable, HIGH);
      digitalWrite(Motor2, HIGH);
      digitalWrite(Middle, LOW);
      client.publish("Living-Window-Motor2", "UP");
    }
    else if (valuejson == 2) // Motor 2 DOWN
    {
      digitalWrite(Motor2Enable, HIGH);
      digitalWrite(Motor2, LOW);
      digitalWrite(Middle, HIGH);
      client.publish("Living-Window-Motor2", "DOWN");
    }
    else if (valuejson == 3) // Motor 2 OFF
    {
      digitalWrite(Motor2Enable, LOW);
      client.publish("Living-Window-Motor2", "OFF");
    }
    break;
  case 8:
    if (valuejson == 1) // Motor 3 UP
    {
      digitalWrite(Motor3A, HIGH);
      digitalWrite(Motor3B, LOW);
      client.publish("Living-Window-Motor3", "UP");
    }
    else if (valuejson == 2) // Motor 3 DOWN
    {
      digitalWrite(Motor3A, LOW);
      digitalWrite(Motor3B, HIGH);
      client.publish("Living-Window-Motor3", "DOWN");
    }
    else if (valuejson == 3) // Motor 3 OFF
    {
      digitalWrite(Motor3A, LOW);
      digitalWrite(Motor3B, LOW);
      client.publish("Living-Window-Motor3", "OFF");
    }
    break;

  case 9:                                // States case
    if (valuejson == 1 && datajson == 1) // Motor 1 DOWNstate
    {
      Motor1DOWNstate = true;
      client.publish("Living-Window-Motor1-Dstate", "ON");
    }
    else if (valuejson == 1 && datajson == 0) // Motor 1 DOWNstate
    {
      Motor1DOWNstate = false;
      client.publish("Living-Window-Motor1-Dstate", "OFF");
    }

    if (valuejson == 2 && datajson == 1) // Motor 2 DOWNstate
    {
      Motor2DOWNstate = true;
      client.publish("Living-Window-Motor2-Dstate", "ON");
    }
    else if (valuejson == 2 && datajson == 0) // Motor 2 DOWNstate
    {
      Motor2DOWNstate = false;
      client.publish("Living-Window-Motor2-Dstate", "OFF");
    }

    if (valuejson == 3 && datajson == 1) // Motor 3 DOWNstate
    {
      Motor3DOWNstate = true;
      client.publish("Living-Window-Motor3-Dstate", "ON");
    }
    else if (valuejson == 3 && datajson == 0) // Motor 3 DOWNstate
    {
      Motor3DOWNstate = false;
      client.publish("Living-Window-Motor3-Dstate", "OFF");
    }

    break;
  default:
    client.publish("ERRORS", "SF51-Living-Window Not Found");
    break;
  }
}

void reconnect()
{
  // Loop until we're reconnected
  if (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    uint32_t timestamp=millis()/1000;
    char clientid[23];
    snprintf(clientid,23, mqttClient"%02X",timestamp);
    Serial.print("Client ID: ");
    Serial.println(clientid);

    if (client.connect(clientid, mqttName, mqttPASS, mqttWillTopic, 0, true, "offline"))
    {
      Serial.print(subscribeTopic);
      Serial.println(" Connected");
      // Once connected, publish an announcement...
      client.publish(mqttWillTopic, "online", true);
      // ... and resubscribe
      client.subscribe(subscribeTopic);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      count = count + 1;
    }
  }
}

void setup()
{
  pinMode(Motor1Enable, OUTPUT);
  pinMode(Motor1, OUTPUT);
  pinMode(Motor2Enable, OUTPUT);
  pinMode(Motor2, OUTPUT);
  pinMode(Middle, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(Motor3A, OUTPUT);
  pinMode(Motor3B, OUTPUT);
  pinMode(LDR, INPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqttPORT);
  client.setCallback(callback);

  ArduinoOTA.setHostname(mqttClient);
  ArduinoOTA.setPort(otaPort);
  ArduinoOTA.setPassword(otaPass);
  ArduinoOTA.begin();
}

void loop()
{
  ArduinoOTA.handle();
  currentMillis = millis();

  if (currentMillis - WifiDelayMillis >= WifiDelayInterval)
  {
    WifiDelayMillis = currentMillis;
    if (!client.connected())
    {
      Serial.println("reconnecting ...");
      reconnect();
    }
    else if (lastCount != count)
    {
      char buffer[200];
      updater["Disconnected"] = count;
      serializeJson(updater, buffer);
      client.publish(mqttDisconnectTopic, buffer, true);
      lastCount = count;
    }
  }

  if (currentMillis - LDRDelayMillis >= LDRDelayInterval)
  {
    LDRDelayMillis = currentMillis;

    char buffer2[200];
    docLDR["Living-Window-LDR"] = analogRead(LDR);
    serializeJson(docLDR, buffer2);
    client.publish("Living-Window-LDR", buffer2);
  }
  client.loop();
}