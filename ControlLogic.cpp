#include "ControlLogic.h"

#include <sstream>
#include <ArduinoJson.h>
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
const char *topicHeatExchangerThermalCutoffAlarm  = "PoolHeater/Inputs/HeatExchangerThermalCutoff";
const char *topicCompressorThermalCutoffAlarm     = "PoolHeater/Inputs/CompressorThermalCutoff";
const char *topicLowPressureSwitchAlarm           = "PoolHeater/Inputs/LowPressureSwitch";
const char *topicHighPressureSwitchAlarm          = "PoolHeater/Inputs/HighPressureSwitch";
const char *topicWaterFlowPressure                = "PoolHeater/Inputs/WaterFlowPressure";


const char *topicDefrost                          = "PoolHeater/Relay/DefrostRelay";
const char *topicFourWayValveFB                   = "PoolHeater/Relay/FourWayValve";
const char *topicFanFB                            = "PoolHeater/Relay/FanRelay";
const char *topicCompressorFB                     = "PoolHeater/Relay/CompressorRelay";

const char *topicPermissivePrice                  = "PoolHeater/Permissive/Price";
const char *topicPermissiveDateTime               = "PoolHeater/Permissive/DateTime";
const char *topicPermissiveMQTT                   = "PoolHeater/Permissive/MQTT";
const char *topicExternalFlowMeter                = "PoolHeater/Permissive/ExteranlFlowMeter";

const char *subPowerwallPrice                     = "Powerwall/Price";
const char *subPowerwallTime                      = "Powerwall/Time";
const char *subPowerwallSOC                       = "Powerwall/SOC";    // State of charge


bool ledOn = false;
int counter = 0;

inline const char * const BoolToString(bool b)
{
  return b ? "{\"value\": 1}" : "{\"value\": 0}";
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

  // If we can't connect, it won't go further.
  while (!ConnectMQTT());

  // Start the WIRE (I2C) library to use the poolHeater class
  poolHeater.Configure();
}

bool ControlLogic::ConnectMQTT()
{
  uint8_t count = 0;

  while (!mqclient.connected()) {
      String client_id = "PoolHeater";
      Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
      if (mqclient.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
          Serial.println("Public EMQX MQTT broker connected");
      } else {
          Serial.print("failed with state ");
          Serial.print(mqclient.state());
          delay(2000);
          count++;

          if (count > 20)
          {
            HandleMQTTLinkChange(false);
            return false;
          }
      }
      HandleMQTTLinkChange(true);
  }

  if (mqclient.subscribe(subPowerwallPrice, 0))
  {
    Serial.print("Subscribed to MQTT topic ");
    Serial.println(subPowerwallPrice);
  }
  if (mqclient.subscribe(subPowerwallTime, 0))
  {
    Serial.print("Subscribed to MQTT topic ");
    Serial.println(subPowerwallTime);
  }
  if (mqclient.subscribe(subPowerwallSOC, 0))
  {
    Serial.print("Subscribed to MQTT topic ");
    Serial.println(subPowerwallSOC);
  }   
  return true;
}

void ControlLogic::loop()
{
  static uint8_t delayCount = 0;

  poolHeater.ReadInputData();

  Serial.print("HeatExThermal: ");
  Serial.print(poolHeater.GetHeatExchangerThermalCutoffAlarm());
  Serial.print(" CompressorThermal: ");
  Serial.print(poolHeater.GetCompressorThermalCutoffAlarm());
  Serial.print(" LowPressure: ");
  Serial.print(poolHeater.GetLowPressureSwitchAlarm());
  Serial.print(" HighPressure: ");
  Serial.print(poolHeater.GetHighPressureSwitchAlarm());
  Serial.print(" WaterFlowPressure: ");
  Serial.print(poolHeater.GetWaterFlowSwitchAlarm());

  Serial.print(" Defrost: ");
  Serial.print(poolHeater.GetDefrostFB());
  Serial.print(" FourWayValve: ");
  Serial.print(poolHeater.GetFourWayValveFB());
  Serial.print(" Fan: ");
  Serial.print(poolHeater.GetFanFB());
  Serial.print(" Compressor: ");
  Serial.print(poolHeater.GetCompressorFB());
  Serial.print(" FlowMeterCount: ");
  Serial.println(mFlowMeterReads);

  // We don't want to send this stuff every second, 10 seconds will do
  delayCount = (delayCount + 1) % 10;
  if (!delayCount)
  {
    // We should have received a number of flow meter readings
    // After evalating, reset flow meter reads to zero so we'll know when water has stopped
    // Level Test pulse frequency pulse characteristics: (Hz) = [0.2 * Q] ± 3% (proficiency tests) (Q is flow L / min)
    if (mFlowMeterReads < 5)
      poolHeater.SetPermissivesExternalFlowMeter(false);
    else
    {
      poolHeater.SetPermissivesExternalFlowMeter(true);
    }
    mFlowMeterReads = 0;

    mqclient.publish(topicHeatExchangerThermalCutoffAlarm, BoolToString(poolHeater.GetHeatExchangerThermalCutoffAlarm()));
    mqclient.publish(topicCompressorThermalCutoffAlarm, BoolToString(poolHeater.GetCompressorThermalCutoffAlarm()));
    mqclient.publish(topicLowPressureSwitchAlarm, BoolToString(poolHeater.GetLowPressureSwitchAlarm()));
    mqclient.publish(topicHighPressureSwitchAlarm, BoolToString(poolHeater.GetHighPressureSwitchAlarm()));
    mqclient.publish(topicWaterFlowPressure, BoolToString(poolHeater.GetWaterFlowSwitchAlarm()));

    mqclient.publish(topicDefrost, BoolToString(poolHeater.GetDefrostFB()));
    mqclient.publish(topicFourWayValveFB, BoolToString(poolHeater.GetFourWayValveFB()));
    mqclient.publish(topicFanFB , BoolToString(poolHeater.GetFanFB()));
    mqclient.publish(topicCompressorFB, BoolToString(poolHeater.GetCompressorFB()));

    mqclient.publish(topicPermissivePrice, BoolToString(poolHeater.GetPermissivesRawData().PriceOk));
    mqclient.publish(topicPermissiveDateTime, BoolToString(poolHeater.GetPermissivesRawData().TimeOk));
    mqclient.publish(topicPermissiveMQTT, BoolToString(poolHeater.GetPermissivesRawData().MqTTLinkOk));
    mqclient.publish(topicExternalFlowMeter, BoolToString(poolHeater.GetPermissivesRawData().externalFlowMeterOk));
  }

  if (poolHeater.AnyAlarmsPresent() || !poolHeater.GetPermissivesOk())
  {
    mAlarmClearCount = 0;
    mAlarmSetCount += 1;
    if (mAlarmSetCount > 3)
    {
      Serial.println("Heater cannot be turned on at this time due to alarms or permissives");
      poolHeater.TurnFanAndCompressorOn(false);
    }
  }
  else
  {
    mAlarmSetCount = 0;
    mAlarmClearCount += 1;

    // So when the alarms clear, we don't just turn it on.  We wait 20 seconds and then turn it on.
    // this is just to stop intermittent turn ons if we get a spurious read.
    if (mAlarmClearCount > 20)
    {
      poolHeater.TurnFanAndCompressorOn(true);
    }
  }


  // We can sometimes disconnect from MQTT server so reconnect if we have.
  if (!mqclient.connected())
  {


    // If we can't reconnect, shut down safely.
    ConnectMQTT();
  }
  else
  {
    // Call the mqClint loop so it has a chance to process received messages.
    mqclient.loop();
  }
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
      std::optional<float> price = charArrayToFloat(payload, length);
      HandleNewPrice(price);
    }
    else if (strcmp("Powerwall/Time", topic) == 0)
    {
      // Allocate the JSON document
      JsonDocument doc;

      // Deserialize the JSON document
      DeserializationError error = deserializeJson(doc, payload);

      // Test if parsing succeeds
      if (error) {
        Serial.print(F("deserializeJson() of datetime failed: "));
        Serial.println(error.f_str());
        return;
      }
      else
      {
        long hour = doc["hour"];
        long min  = doc["minutes"];
        long month = doc["month"];
        Serial.print("We have a Time ");
        Serial.print(hour);
        Serial.print(" ");
        Serial.print(min);
        Serial.print(" ");
        Serial.print(month);
        HandleNewDateTime(month, hour, min);
      }      
    }
    else if (strcmp("Powerwall/SOC", topic) == 0)
    {
      Serial.println("We have a SOC");
      std::optional<float> SOC = charArrayToFloat(payload, length);
      HandleNewSOC(SOC);
    }
}

void ControlLogic::HandleNewPrice(std::optional<float> price)
{
  // We don't pay too much.
  if (!price || *price > 25.0)
    poolHeater.SetPermissivesPrice(false);
  else
  {
    if (*price < 20.0)
      poolHeater.SetPermissivesPrice(true);
  }
}

void ControlLogic::HandleNewSOC(std::optional<float> SOC)
{
  // We don't pay too much.
  if (!SOC || *SOC < 70.0)
    poolHeater.SetPermissivesSOC(false);
  else
  {
    poolHeater.SetPermissivesSOC(true);
  }
}

void ControlLogic::HandleNewDateTime(long month, long hour, long minute)
{
    bool isOk = true;
    switch(month)
    {
      case 11:
      case 12:
      case 1:
      case 2:
      case 3:
        break;
      default:
        isOk = false;
    }
    switch(hour)
    {
      case 11:
      case 12:
      case 1:
      case 2:
      case 3:
      case 4:
        break;
      default:
        isOk = false;
    }
    poolHeater.SetPermissivesDateTime(true);
}

void ControlLogic::HandleMQTTLinkChange(bool isUp)
{
    poolHeater.SetPermissivesMQLink(isUp);
}

void ControlLogic::HandleFlowMeterRead()
{
  mFlowMeterReads += 1;
}