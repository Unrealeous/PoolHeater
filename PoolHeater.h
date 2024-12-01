#ifndef POOLHEATER_h
#define POOLHEATER_h

#include "PCA9535.h"

#define I2C_POOL_HEATER 0x21

template<typename T> 
T myXOR(T x, T y)
{
   return (x | y) & (~x | ~y);
}

class PoolHeater
{
public:
	PoolHeater();

  // This needs to be called at the start so configure the inputs and outputs.
  void Configure();

  // Read the input data.  Return true if we have a successful read.
  bool ReadInputData();

  bool isConnected() const { return mConnected; }

  bool GetHeatExchangerThermalCutoffAlarm() const { return mHeatExchangerThermalCutoffAlarm; }
  bool GetCompressorThermalCutoffAlarm() const { return mCompressorThermalCutoffAlarm; }
  bool GetLowPressureSwitchAlarm() const { return mLowPressureSwitchAlarm; }
  bool GetHighPressureSwitchAlarm() const { return mHighPressureSwitchAlarm; }
  bool GetWaterFlowSwitchAlarm() const { return mWaterFlowSwitchAlarm; }

private:
  bool mConnected;
  bool mHeatExchangerThermalCutoffAlarm;
  bool mCompressorThermalCutoffAlarm;
  bool mLowPressureSwitchAlarm;
  bool mHighPressureSwitchAlarm;
  bool mWaterFlowSwitchAlarm;

  PCA9535 mPoolLink;
};



#endif