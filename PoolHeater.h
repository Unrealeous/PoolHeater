#ifndef POOLHEATER_h
#define POOLHEATER_h

#include <Wire.h>
#include "PCA9535.h"


#define I2C_POOL_HEATER 0x21

template<typename T> 
T myXOR(T x, T y)
{
   return (x | y) & (~x | ~y);
}


// The class to control the SPANet
// We have an I2C link to read parameters and turn on/off relays
class PoolHeater
{
public:
	PoolHeater();

  // This needs to be called at the start so configure the inputs and outputs.
  void Configure();

  // Read the input data.  Return true if we have a successful read.
  bool ReadInputData();

  bool isConnected() const { return mConnected; }

  bool AnyAlarmsPresent();

  bool GetHeatExchangerThermalCutoffAlarm() const { return mHeatExchangerThermalCutoffAlarm; }
  bool GetCompressorThermalCutoffAlarm() const { return mCompressorThermalCutoffAlarm; }
  bool GetLowPressureSwitchAlarm() const { return mLowPressureSwitchAlarm; }
  bool GetHighPressureSwitchAlarm() const { return mHighPressureSwitchAlarm; }
  bool GetWaterFlowSwitchAlarm() const { return mWaterFlowSwitchAlarm; }

  void TurnFanOn(bool turnOn);

private:
  bool mConnected;
  bool mHeatExchangerThermalCutoffAlarm;
  bool mCompressorThermalCutoffAlarm;
  bool mLowPressureSwitchAlarm;
  bool mHighPressureSwitchAlarm;
  bool mWaterFlowSwitchAlarm;

  PCA9535 mPoolLink;

  enum {
    WATER_FLOW_SWITCH            =1,
    HIGH_PRESSURE_SWITCH         =2,
    LOW_PRESSURE_SWITCH          =4,
    COMPRESSOR_THERMAL_CUTOFF    =8,
    HEAT_EXCHANGE_THERMAL_CUTOFF =16
  };

  enum {
    DEFROST       = 1,
    FOURWAY_VALVE = 2,
    FAN           = 4,
    COMPRESSOR    = 8
  };

  uint8_t mRelaySettings=0;
};



#endif