#include "PoolHeater.h"


template<typename T> 
T myXOR(T x, T y)
{
   return (x | y) & (~x | ~y);
}



PoolHeater::PoolHeater() :
  mConnected(false),
  mHeatExchangerThermalCutoffAlarm(1),
  mCompressorThermalCutoffAlarm(1),
  mLowPressureSwitchAlarm(1),
  mHighPressureSwitchAlarm(1),
  mWaterFlowSwitchAlarm(1),
  mPoolLink(I2C_POOL_HEATER, true)
{
}

void PoolHeater::Configure()
{
  mPoolLink.setConfigurationGroup0(0x00);  // 1 is read, 0 is write.  All pins READ
  mPoolLink.setConfigurationGroup1(0xFF);  // 1 is read, 0 is write.  All pins READ

  // Make sure everything is off.
  mPoolLink.writeGpioGroup0(0);
}

bool PoolHeater::ReadInputData()
{
  mConnected = true;

  uint8_t data = 0;
  
  data = mPoolLink.readInputGpioGroup0();
  mDefrostFB = data & RELAY_DEFROST;
  mFourWayValveFB = data & RELAY_FOURWAY_VALVE;
  mFanFB = data & RELAY_FAN;
  mCompressorFB = data & RELAY_COMPRESSOR;

  data = mPoolLink.readInputGpioGroup1();

  mHeatExchangerThermalCutoffAlarm = data & HEAT_EXCHANGE_THERMAL_CUTOFF;
  mCompressorThermalCutoffAlarm = data & COMPRESSOR_THERMAL_CUTOFF;
  mLowPressureSwitchAlarm = data & LOW_PRESSURE_SWITCH;
  mHighPressureSwitchAlarm = data & HIGH_PRESSURE_SWITCH;
  mWaterFlowSwitchAlarm = data & WATER_FLOW_SWITCH;

  return mConnected;
}

bool PoolHeater::AnyAlarmsPresent()
{
  bool result = GetHeatExchangerThermalCutoffAlarm() ||
                GetCompressorThermalCutoffAlarm() ||
                GetLowPressureSwitchAlarm() ||
                GetHighPressureSwitchAlarm() ||
                GetWaterFlowSwitchAlarm();

  return result;
}

bool PoolHeater::DoRelaySettingsMatchActual() const
{
  // Using the bits, create the same byte contains what should be set.
  uint8_t settings = (mDefrostFB | mFourWayValveFB << 2 | mFanFB << 2 | mCompressorFB << 3);
  return (settings == mDesiredRelaySettings);
}

void PoolHeater::TurnFanAndCompressorOn(bool turnOn)
{
  if (turnOn)
  {
    mDesiredRelaySettings = RELAY_FAN | RELAY_COMPRESSOR;
  }
  else
  {
    mDesiredRelaySettings = 0;  // everything off.
  }

  // Read the current settings and if we don't match, send them
  if (!DoRelaySettingsMatchActual())
  {
    Serial.print("Writing the desired settings ");
    mPoolLink.writeGpioGroup0(mDesiredRelaySettings);
  }
}

void PoolHeater::SetPermissivesPrice(bool isOk)
{
  mPermissives.PriceOk = isOk;
}
void PoolHeater::SetPermissivesSOC(bool isOk)
{
  mPermissives.SOCOk = isOk;
}
void PoolHeater::SetPermissivesDateTime(bool isOk)
{
  mPermissives.TimeOk = isOk;
}
void PoolHeater::SetPermissivesMQLink(bool isOk)
{
  mPermissives.MqTTLinkOk = isOk;
}
bool PoolHeater::GetPermissivesOk() const
{
  return mPermissives.PriceOk && 
         mPermissives.SOCOk &&
         mPermissives.TimeOk && 
         mPermissives.MqTTLinkOk;
}