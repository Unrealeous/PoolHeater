#ifndef PCA9535_h
#define PCA9535_h

#include "Arduino.h"
#include <Wire.h>

// A class to control the PCA9535 chip.
// This is a simple IO chip that you can control via a SPI intereface
// 16 pins, each can be configured as an input or output.
// You can have up to 8 different devices (8 SPI addresses)
// Really cheap way to add more I/O to a project
class PCA9535
{
public:
	PCA9535(uint8_t PCAaddress, bool beginWire = false);
	uint8_t readInputGpioGroup0();
	uint8_t readInputGpioGroup1();
	uint8_t readOutputGpioGroup0();
	uint8_t readOutputGpioGroup1();
	void    writeGpioGroup0(uint8_t groupValue);
	void    writeGpioGroup1(uint8_t groupValue);
	uint8_t readInputPolarityGroup0();
	uint8_t readInputPolarityGroup1();
	void    setPolarityInversionGroup0(uint8_t groupValue);
	void    setPolarityInversionGroup1(uint8_t groupValue);
	uint8_t readConfigurationGroup0();
	uint8_t readConfigurationGroup1();
	void    setConfigurationGroup0(uint8_t configuration);
	void    setConfigurationGroup1(uint8_t configuration);

private:
	void writeI2c(uint8_t address, uint8_t data);
	void writeI2c(uint8_t address, uint8_t data1, uint8_t data2);
	uint8_t readI2c(uint8_t address);

	uint8_t mPCA9535Address;

	enum commandRegisters
	{
		inputP0 = 0,
		inputP1,
		outputP0,
		outputP1,
		polarityInversionP0,
		polarityInversionP1,
		configurationP0,
		configurationP1
	};
};

#endif
