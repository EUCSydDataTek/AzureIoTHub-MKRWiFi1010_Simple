/*
  Azure_IoT_Hub_MKRWiFi1010_Simple 
  ================================

  This example securely connects to an Azure IoT Hub using MQTT over WiFi.
  It uses a private key stored in the ATECC508A and a selfsigned
  certificate for SSL/TLS authetication.

 It publishes a message every 5 seconds to "devices/{deviceId}/messages/events/"
         and subscribes to messages on the "devices/{deviceId}/messages/devicebound/#" topic.

  The circuit: Arduino MKR WiFi 1010
  Red LED: Connecting to WiFi
  Blue LED: Connecting to MQTT broker (Azure IoT Hub)
  Green LED: Connected and sending telemetry

  https://github.com/arduino/ArduinoCloudProviderExamples/tree/master/examples/AWS%20IoT/AWS_IoT_WiFi
*/

#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <ArduinoMqttClient.h>
#include <WiFiNINA.h> 
#include <utility/ECCX08SelfSignedCert.h>
#include "arduino_secrets.h"

/////// Enter your sensitive data in arduino_secrets.h
const char ssid[]        = SECRET_SSID;
const char pass[]        = SECRET_PASS;
const char broker[]      = SECRET_BROKER;
const char deviceId[]    = SECRET_DEVICE_ID;

String publishTopic = "devices/" + String(deviceId) + "/messages/events/";
String subscribeTopic = "devices/" + String(deviceId) + "/messages/devicebound/#";

WiFiClient    wifiClient;            // Used for the TCP socket connection
BearSSLClient sslClient(wifiClient); // Used for SSL/TLS connection, integrates with ECC508
MqttClient    mqttClient(sslClient);

unsigned long lastMillis = 0;

unsigned long getTime();
void initCertificate();
void connectWiFi();
void connectMQTT();
void publishMessage();
void onMessageReceived(int messageSize);

////////////////////////////////////// SETUP ////////////////////////////////////////////
void setup() 
{
    // RGB LEDS on board
  WiFiDrv::pinMode(25, OUTPUT);
  WiFiDrv::pinMode(26, OUTPUT);
  WiFiDrv::pinMode(27, OUTPUT);

  delay(3000);
  Serial.begin(9600);
  while (!Serial);

  initCertificate();

  mqttClient.setId(deviceId);

  // Set the username to "<broker>/<device id>/api-version=2021-04-12" and empty password
  String username = broker + String("/") + deviceId + String("/api-version=2021-04-12");

  mqttClient.setUsernamePassword(username, "");

  mqttClient.onMessage(onMessageReceived);
}

////////////////////////////////////// LOOP /////////////////////////////////////////////
void loop() 
{
  if (WiFi.status() != WL_CONNECTED) 
  {
    WiFiDrv::digitalWrite(26, HIGH);   //RED LED
    connectWiFi();
  }

  if (!mqttClient.connected()) 
  {
     WiFiDrv::digitalWrite(26, LOW);    //RED LED
     WiFiDrv::digitalWrite(27, HIGH);   //BLUE LED

    // MQTT client is disconnected, connect
    connectMQTT();

    WiFiDrv::digitalWrite(27, LOW);   //BLUE LED
    WiFiDrv::digitalWrite(25, HIGH); //GREEN LED
  }

  // poll for new MQTT messages and send keep alives
  mqttClient.poll();

  // publish a message roughly every 5 seconds.
  if (millis() - lastMillis > 5000) 
  {
    lastMillis = millis();

    publishMessage();
  }
}

void initCertificate()
{
  if (!ECCX08.begin()) 
  {
    Serial.println("No ECCX08 present!");
    while (1);
  }

  // Set a callback to get the current time - used to validate the servers certificate
  ArduinoBearSSL.onGetTime(getTime);

  // reconstruct the self signed cert
  ECCX08SelfSignedCert.beginReconstruction(0, 8);
  ECCX08SelfSignedCert.setCommonName(ECCX08.serialNumber());
  ECCX08SelfSignedCert.endReconstruction();

  // Set the ECCX08 slot to use for the private key and the accompanying public certificate for it
  sslClient.setEccSlot(0, ECCX08SelfSignedCert.bytes(), ECCX08SelfSignedCert.length());
}

unsigned long getTime() 
{ 
  return WiFi.getTime();
}

void connectWiFi() 
{
  Serial.print("Attempting to connect to SSID: ");
  Serial.print(ssid);
  Serial.print(" ");

  while (WiFi.begin(ssid, pass) != WL_CONNECTED) 
  {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println();

  Serial.println("You're connected to the network");
  Serial.println();
}

void connectMQTT() 
{
  Serial.print("Attempting to MQTT broker: ");
  Serial.print(broker);
  Serial.println(" ");

  while (!mqttClient.connect(broker, 8883)) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println();

  Serial.println("You're connected to the MQTT broker");
  Serial.println();

  // subscribe to a topic
  mqttClient.subscribe(subscribeTopic);
}

void publishMessage() 
{
  Serial.println("Publishing message");

  // send message, the Print interface can be used to set the message contents
  mqttClient.beginMessage(publishTopic);
  mqttClient.print("hello ");
  mqttClient.print(millis());
  mqttClient.endMessage();
}

void onMessageReceived(int messageSize) 
{
  // we received a message, print out the topic and contents
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  // use the Stream interface to print the contents
  while (mqttClient.available()) 
  {
    Serial.print((char)mqttClient.read());
  }
  Serial.println();

  Serial.println();
}