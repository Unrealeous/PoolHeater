#include "PoolHeater.h"

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
}

bool PoolHeater::ReadInputData()
{
  mConnected = true;

  uint8_t data = mPoolLink.readInputGpioGroup1();

  mHeatExchangerThermalCutoffAlarm = data & HEAT_EXCHANGE_THERMAL_CUTOFF;
  mCompressorThermalCutoffAlarm = data & COMPRESSOR_THERMAL_CUTOFF;
  mLowPressureSwitchAlarm = data & LOW_PRESSURE_SWITCH;
  mHighPressureSwitchAlarm = data & HIGH_PRESSURE_SWITCH;
  mWaterFlowSwitchAlarm = data & WATER_FLOW_SWITCH;

  return mConnected;
}

bool PoolHeater::AnyAlarmsPresent()
{
  return  GetHeatExchangerThermalCutoffAlarm() ||
          GetCompressorThermalCutoffAlarm() ||
          GetLowPressureSwitchAlarm() ||
          GetHighPressureSwitchAlarm() ||
          GetWaterFlowSwitchAlarm();
}

void PoolHeater::TurnFanOn(bool turnOn)
{
  if (turnOn)
    mPoolLink.writeGpioGroup0(FAN);
  else
    mPoolLink.writeGpioGroup0(0);
}
