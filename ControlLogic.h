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
    void ConnectMQTT();
    void mqcallback(char *topic, byte *payload, unsigned int length);

    // The current electricity price.  It is optional as if we don't know what it is, it won't be set
    std::optional<float> moPrice;

    WiFiClient espClient;
    PubSubClient mqclient;

    // This is the class that actually controlls the pool heater.
    PoolHeater poolHeater;

};



#endif