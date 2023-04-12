# AzureIoTHub-MKRWiFi1010_Simple

  This example securely connects to an Azure IoT Hub using MQTT over WiFi.
  It uses a private key stored in the ATECC508A and a selfsigned
  certificate for SSL/TLS authetication.

  It publishes a message every 5 seconds to arduino/outgoing
  topic and subscribes to messages on the arduino/incoming
  topic.

 It publishes a message every 5 seconds to "devices/{deviceId}/messages/events/"
         and subscribes to messages on the "devices/{deviceId}/messages/devicebound/#" topic.

  The circuit: Arduino MKR WiFi 1010
  - Red LED: Connecting to WiFi
  - Blue LED: Connecting to MQTT broker (Azure IoT Hub)
  - Green LED: Connected and sending telemetry

  https://github.com/arduino/ArduinoCloudProviderExamples/tree/master/examples/AWS%20IoT/AWS_IoT_WiFi
