/*
 WiFi Web Server LED Blink
 */
#include <WiFi.h>
#include <PubSubClient.h>

#include "PoolHeater.h"
#include "Passwords.h"

// The dlink is not connected to anything important.
// Normally you'd hide passwords.
extern const char *ssid;
extern const char *ssid_password;


// MQTT Broker
const char *mqtt_broker = "10.1.1.205";
extern const char *mqtt_username;
extern const char *mqtt_password;
const int mqtt_port = 1883;
const char *topicHeatExchangerThermalCutoffAlarm  = "PoolHeater/HeatExchangerThermalCutoff";
const char *topicCompressorThermalCutoffAlarm     = "PoolHeater/CompressorThermalCutoff";
const char *topicLowPressureSwitchAlarm           = "PoolHeater/LowPressureSwitch";
const char *topicHighPressureSwitchAlarm          = "PoolHeater/HighPressureSwitch";
const char *topicWaterFlowPressure                = "PoolHeater/WaterFlowPressure";

bool ledOn = false;
int counter = 0;

//NetworkServer server(80);

WiFiClient espClient;
PubSubClient mqclient(espClient);


PoolHeater poolHeater;

void mqcallback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
    }
    Serial.println();
    Serial.println("-----------------------");
}

inline const char * const BoolToString(bool b)
{
  return b ? "{\"value\": 1}" : "{\"value\": 0}";
}

void setup() {
  Serial.begin(115200);
  pinMode(5, OUTPUT);  // set the LED pin mode
  pinMode(2, OUTPUT);  // set the LED pin mode
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, ssid_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  mqclient.setServer(mqtt_broker, mqtt_port);
  mqclient.setCallback(mqcallback);

  while (!mqclient.connected()) {
      String client_id = "PoolHeater";
      Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
      if (mqclient.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
          Serial.println("Public EMQX MQTT broker connected");
      } else {
          Serial.print("failed with state ");
          Serial.print(mqclient.state());
          delay(2000);
      }
  }

  // Start the WIRE (I2C) library to use the poolHeater class
  poolHeater.Configure();


}


 

void loop() {
  /*
  NetworkClient client = server.accept();  // listen for incoming clients

  if (client) {                     // if you get a client,
    Serial.println("New Client.");  // print a message out the serial port
    String currentLine = "";        // make a String to hold incoming data from the client
    while (client.connected()) {    // loop while the client's connected
      if (client.available()) {     // if there's bytes to read from the client,
        char c = client.read();     // read a byte, then
        Serial.write(c);            // print it out the serial monitor
        if (c == '\n') {            // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: monospace; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: yellowgreen; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 32px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: gray;}</style></head>");

            client.println("<body><h1>ESP32 Web Server</h1>");
            client.println("<p>Control LED State</p>");

            // the content of the HTTP response follows the header:
            client.print("Click <a href=\"/H\">here</a> to turn the LED on pin 5 on.<br>");
            client.print("Click <a href=\"/L\">here</a> to turn the LED on pin 5 off.<br>");

            if (!ledOn)
              client.println("<p><a href=\"/16/on\"><button class=\"button\">ON</button></a></p>");
            else
              client.println("<p><a href=\"/16/off\"><button class=\"button button2\">OFF</button></a></p>");

            if (poolHeater.GetHeatExchangerThermalCutoffAlarm())
            {
              client.println("<p><a><button class=\"button\" disabled>Compressor Thermal Cutoff</button></a></p>");
            }
            else
            {
              client.println("<p><a><button class=\"button button2\" disabled>Compressor Thermal Cutoff</button></a></p>");
            }

            client.println("</body></html>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {  // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H") || currentLine.endsWith("GET /16/on")) {
          ledOn = true;
          digitalWrite(2, HIGH);  // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L") || currentLine.endsWith("GET /16/off")) {
          ledOn = false;
          digitalWrite(2, LOW);  // GET /L turns the LED off
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
  delay(1500);
  */

  delay(4500);
  //mqclient.loop();
  if (poolHeater.ReadInputData())
  {
    Serial.print("HeatExThermal: ");
    Serial.print(poolHeater.GetHeatExchangerThermalCutoffAlarm());
    Serial.print(" CompressorThermal: ");
    Serial.print(poolHeater.GetCompressorThermalCutoffAlarm());
    Serial.print(" LowPressure: ");
    Serial.print(poolHeater.GetLowPressureSwitchAlarm());
    Serial.print(" HighPressure: ");
    Serial.print(poolHeater.GetHighPressureSwitchAlarm());
    Serial.print(" WaterFlowPressure: ");
    Serial.println(poolHeater.GetWaterFlowSwitchAlarm());

    mqclient.publish(topicHeatExchangerThermalCutoffAlarm, BoolToString(poolHeater.GetHeatExchangerThermalCutoffAlarm()));
    mqclient.publish(topicCompressorThermalCutoffAlarm, BoolToString(poolHeater.GetCompressorThermalCutoffAlarm()));
    mqclient.publish(topicLowPressureSwitchAlarm, BoolToString(poolHeater.GetLowPressureSwitchAlarm()));
    mqclient.publish(topicHighPressureSwitchAlarm, BoolToString(poolHeater.GetHighPressureSwitchAlarm()));
    mqclient.publish(topicWaterFlowPressure, BoolToString(poolHeater.GetWaterFlowSwitchAlarm()));

    if (poolHeater.AnyAlarmsPresent())
    {
      Serial.println("Heater cannot be turned on at this time due to alarms");
    }
  }
  else
  {
    Serial.println("Failed to read any data");
  }

  if (ledOn)
  {
    ledOn = false;
    poolHeater.TurnFanOn(true);
  }
  else
  {
    ledOn = true;
    poolHeater.TurnFanOn(false);
  }

}


