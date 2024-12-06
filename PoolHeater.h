#ifndef POOLHEATER_h
#define POOLHEATER_h

#include <Wire.h>
#include "PCA9535.h"


#define I2C_POOL_HEATER 0x21


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

  // Are there any alarms that present. 
  bool AnyAlarmsPresent();

  // Check if our current relay settings match what we think we have set.
  bool DoRelaySettingsMatchActual() const;

  bool GetHeatExchangerThermalCutoffAlarm() const { return mHeatExchangerThermalCutoffAlarm; }
  bool GetCompressorThermalCutoffAlarm() const { return mCompressorThermalCutoffAlarm; }
  bool GetLowPressureSwitchAlarm() const { return mLowPressureSwitchAlarm; }
  bool GetHighPressureSwitchAlarm() const { return mHighPressureSwitchAlarm; }
  bool GetWaterFlowSwitchAlarm() const { return mWaterFlowSwitchAlarm; }

  bool GetDefrostFB() const { return mDefrostFB; }
  bool GetFourWayValveFB() const { return mFourWayValveFB; }
  bool GetFanFB() const { return mFanFB; }
  bool GetCompressorFB() const { return mCompressorFB; }

  // This can be used to turn on/off the fan and compressor. We don't run one without the other.
  void TurnFanAndCompressorOn(bool turnOn);

  // If both fan and compressor are actually on then this returns true
  bool GetFanOn() const { return mFanFB; }
  bool GetCompressorOn() const { return mCompressorFB; }

  struct Permissives_struct
  {
    bool PriceOk = false;
    bool TimeOk = false;;
    bool MqTTLinkOk = false;
  };

  void SetPermissivesPrice(bool isOk);
  void SetPermissivesDateTime(bool isOk);
  void SetPermissivesMQLink(bool isOk);

  // Check if all the permissives are in a true state.
  bool GetPermissivesOk() const;
  
  // This could be individual getters
  const Permissives_struct& GetPermissivesRawData() const { return mPermissives; }

private:
  bool mConnected;
  bool mHeatExchangerThermalCutoffAlarm;
  bool mCompressorThermalCutoffAlarm;
  bool mLowPressureSwitchAlarm;
  bool mHighPressureSwitchAlarm;
  bool mWaterFlowSwitchAlarm;

  // These hold what we read back from the heater.
  bool mDefrostFB;
  bool mFourWayValveFB;
  bool mFanFB;
  bool mCompressorFB;

  // Store the relay settings so we can turn bits on and off and not
  // affect other settings.
  uint8_t mDesiredRelaySettings=0;

  // We also have some permissives that need to be true to allow the system
  // to run.  We store them on this class so it knows when its ok to run or not.
  Permissives_struct mPermissives;

  // This class is the phycial link to the pool controller.
  PCA9535 mPoolLink;

  // PCA9536 IO1 has these read only inputs
  enum {
    WATER_FLOW_SWITCH            =1,
    HIGH_PRESSURE_SWITCH         =2,
    LOW_PRESSURE_SWITCH          =4,
    COMPRESSOR_THERMAL_CUTOFF    =8,
    HEAT_EXCHANGE_THERMAL_CUTOFF =16
  };

// PCA9536 IO0 has these write only outputs
  enum {
    RELAY_DEFROST       = 1,
    RELAY_FOURWAY_VALVE = 2,
    RELAY_FAN           = 4,
    RELAY_COMPRESSOR    = 8
  };

};



#endif