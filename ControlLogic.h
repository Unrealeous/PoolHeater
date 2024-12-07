#ifndef CONTROLLOGIC_h
#define CONTROLLOGIC_h

#include <optional>

#include <WiFi.h>
#include <PubSubClient.h>
#include "PoolHeater.h"

// The brains that control when to turn the spa on, of etc
// This class should really be a singleton as multiple instances are going to cause problems.
class ControlLogic
{
  public:
    ControlLogic();

    void Configure();

    // This should be called periodically to do stuff.
    void loop();

  private:
    bool ConnectMQTT();
    void mqcallback(char *topic, byte *payload, unsigned int length);

    void HandleNewPrice(std::optional<float> price);
    void HandleNewSOC(std::optional<float> SOC);
    void HandleNewDateTime(long month, long hour, long minute);
    void HandleMQTTLinkChange(bool isUp);

    WiFiClient espClient;
    PubSubClient mqclient;

    // This is the class that actually controlls the pool heater.
    PoolHeater poolHeater;

    // When the alarms clear and we can start, we want to wait until we see 
    // a lot that is wasn't just a one off.  
    uint16_t mAlarmClearCount = 0;

    // When the alarms Set and we have to stop, we want to wait until we see 
    // a lot that is wasn't just a one off.  We do get some spurous readings
    uint16_t mAlarmSetCount = 0;
};



#endif