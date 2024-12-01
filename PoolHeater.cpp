#include "PoolHeater.h"

PoolHeater::PoolHeater() :
  mConnected(false),
  mHeatExchangerThermalCutoffAlarm(1),
  mCompressorThermalCutoffAlarm(1),
  mLowPressureSwitchAlarm(1),
  mHighPressureSwitchAlarm(1),
  mWaterFlowSwitchAlarm(1),
  mPoolLink(I2C_POOL_HEATER)
{
}

void PoolHeater::Configure()
{
  Wire.begin();
  mPoolLink.setConfigurationGroup0(0x00);  // All pins READ
  mPoolLink.setConfigurationGroup1(0xFF);  // All pins WRITE
}

bool PoolHeater::ReadInputData()
{
  mConnected = false;

  while(Wire.available())
  {
    const uint8_t HEAT_EXCHANGE_THERMAL_CUTOFF =16;
    const uint8_t COMPRESSOR_THERMAL_CUTOFF    =8;
    const uint8_t LOW_PRESSURE_SWITCH          =4;
    const uint8_t HIGH_PRESSURE_SWITCH         =2;
    const uint8_t WATER_FLOW_SWITCH            =1;

    uint8_t data = mPoolLink.readInputGpioGroup1();

    mHeatExchangerThermalCutoffAlarm = data & HEAT_EXCHANGE_THERMAL_CUTOFF;
    mCompressorThermalCutoffAlarm = data & COMPRESSOR_THERMAL_CUTOFF;
    mLowPressureSwitchAlarm = data & LOW_PRESSURE_SWITCH;
    mHighPressureSwitchAlarm = data & HIGH_PRESSURE_SWITCH;
    mWaterFlowSwitchAlarm = data & WATER_FLOW_SWITCH;

    mConnected = true;
  }
  return mConnected;
}
