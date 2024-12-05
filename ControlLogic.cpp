#include "ControlLogic.h"

#include <sstream>
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
const char *subPowerwallPrice                     = "Powerwall/Price";

bool ledOn = false;
int counter = 0;

inline const char * const BoolToString(bool b)
{
  return b ? "{\"value\": 1}" : "{\"value\": 0}";
}

std::optional<float> charArrayToFloat(const unsigned char* arr, size_t len) 
{
    std::optional<float> undef;
    if (len == 0) {
        return undef;
    }

    std::string numString(reinterpret_cast<const char*>(arr), len);

    // Check if the string contains valid number format
    for (std::string::size_type i = 0; i < numString.size(); ++i) 
    {
      char c = numString[i];
      // Handle a negative number.
      if (i == 0 && c=='-')
        continue;
      
      // Make sure it looks like a number.
      if (!std::isdigit(c) && c != '.') {
          return undef;
      }
    }

    std::istringstream iss(numString);
    float r;  
    if ((iss >> r)) {
        return r;
    }
    return undef;
}


void ControlLogic::mqcallback(char *topic, byte *payload, unsigned int length) 
{
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
    }

    if (strcmp("Powerwall/Price", topic) == 0)
    {
      Serial.println("We have a price");
      moPrice = charArrayToFloat(payload, length);
      if (moPrice)
        Serial.println(*moPrice);
    }
    Serial.println();
    Serial.println("-----------------------");
}

ControlLogic::ControlLogic() :
  mqclient(espClient)
{}

void ControlLogic::Configure()
{
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

  // We want to bind our callback to this class so we have to do a bit of bind magic here
  // or else you end up with a static error compiler issue.
  using std::placeholders::_1;
  using std::placeholders::_2;
  using std::placeholders::_3;
  mqclient.setCallback(std::bind(&ControlLogic::mqcallback, this, _1, _2, _3));

  ConnectMQTT();

  // Start the WIRE (I2C) library to use the poolHeater class
  poolHeater.Configure();
}

void ControlLogic::ConnectMQTT()
{
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

  if (mqclient.subscribe("Powerwall/Price", 0))
  {
    Serial.print("Subscribed to MQTT topic ");
    Serial.println(subPowerwallPrice);
  }
}

void ControlLogic::loop()
{
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

  if (mqclient.connected())
  {
    // Call the mqClint loop so it has a chance to process received messages.
    mqclient.loop();
  }
  else
  {
    ConnectMQTT();
  }
}

