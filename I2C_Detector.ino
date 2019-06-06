//
// I2C Detector - scan and identify devices on an I2C bus
//
// The purpose of this code is to provide a tiny bit of code which can serve
// to detect not only the addresses of I2C devices, but what type of device each one is.
// So far, I've added the 15 devices I've personally used or found to be reliably detected
// based on their register contents. I encourage people to do pull requests to add support
// for more devices to make this code have wider appeal.

// There are plenty of I2C devices which appear at fixed addresses, yet don't have unique
// "Who_Am_I" registers or other data to reliably identify them. It's certainly possible to
// write code which initializes these devices and tries to verify their identity. This can
// potentially damage them and would necessarily increase the code size. I would like to keep
// the size of this code small enough so that it can be included in many microcontroller 
// projects where code space is scarce.

// This partical code was written for Arduino, but it's generic enough to run on any system
// (embedded or Linux) which has an I2C bus. I used my Bit Bang library to support using any
// GPIO pins for the I2C scan, but there's nothing wrong with using the Wire library or
// Linux's I2C file device for accomplishing the same thing.

// Copyright (c) 2019 BitBank Software, Inc.
// Written by Larry Bank
// email: bitbank@pobox.com
// Project started 25/02/2019
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// Uses my Bit Bang I2C library. You can find it here:
// https://github.com/bitbank2/BitBang_I2C

//#define USE_BITBANG

#ifdef USE_BITBANG
#include <BitBang_I2C.h>
// Arbitrary pins I used for testing with an ATmega328p
#define SDA_PIN 0xc0
#define SCL_PIN 0xc1
#else
#include <Wire.h>
#endif

#ifndef USE_BITBANG
void I2CScan(uint8_t *pMap)
{
uint8_t addr, err;

  memset(pMap, 0, 16);
  for(addr = 1; addr < 127; addr++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(addr);
    err = Wire.endTransmission();
 
    if (err == 0) // a device responded
       pMap[addr>>3] |= (1 << (addr & 7));
  }
}
int I2CReadRegister(uint8_t addr, uint8_t reg, uint8_t *pData, int len)
{
int i;

  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.endTransmission();

  Wire.requestFrom(addr, len);
  i = 0;
  while (i < len && Wire.available())
  {
     pData[i] = Wire.read();
     i++;
  }
  return i;
}
#endif
//
// If you don't need the explicit device names displayed, disable this code by
// commenting out the next line
//
#define SHOW_NAME
#ifdef SHOW_NAME
const char *szNames[]  = {"Unknown","SSD1306","SH1106","VL53L0X","BMP180", "BMP280","BME280",
                "MPU-60x0", "MPU-9250", "MCP9808","LSM6DS3", "ADXL345", "ADS1115","MAX44009",
                "MAG3110", "CCS811", "HTS221", "LPS25H", "LSM9DS1","LM8330", "DS3231"};
#endif
// supported devices
enum {
  DEVICE_UNKNOWN = 0,
  DEVICE_SSD1306,
  DEVICE_SH1106,
  DEVICE_VL53L0X,
  DEVICE_BMP180,
  DEVICE_BMP280,
  DEVICE_BME280,
  DEVICE_MPU6000,
  DEVICE_MPU9250,
  DEVICE_MCP9808,
  DEVICE_LSM6DS3,
  DEVICE_ADXL345,
  DEVICE_ADS1115,
  DEVICE_MAX44009,
  DEVICE_MAG3110,
  DEVICE_CCS811,
  DEVICE_HTS221,
  DEVICE_LPS25H,
  DEVICE_LSM9DS1,
  DEVICE_LM8330,
  DEVICE_DS3231
};
//
// Figure out what device is at that address
//
int DiscoverDevice(uint8_t i)
{
uint8_t j, cTemp[8];
int iDevice = DEVICE_UNKNOWN;

  if (i == 0x3c || i == 0x3d) // Probably an OLED display
  {
    uint8_t c;
    I2CReadRegister(i, 0x00, cTemp, 1);
    cTemp[0] &= 0xbf; // mask off power on/off bit
    if (cTemp[0] == 0x8) // SH1106
       iDevice = DEVICE_SH1106;
    else if (cTemp[0] == 3 || cTemp[0] == 6)
       iDevice = DEVICE_SSD1306;
    return iDevice;
  }
//  else if (i == 0x5b) // MLX90615?
//  {
//    I2CReadRegister(i, 0x10, cTemp, 3);
//    for (j=0; j<3; j++) Serial.println(cTemp[j], HEX);
//  }
  else // try to identify it from the known devices using register contents
  {
    // Check for VL53L0X
    I2CReadRegister(i, 0xc0, cTemp, 3);
    if (cTemp[0] == 0xee && cTemp[1] == 0xaa && cTemp[2] == 0x10)
       return DEVICE_VL53L0X;

    // Check for CCS811
    I2CReadRegister(i, 0x20, cTemp, 1);
    if (cTemp[0] == 0x81) // Device ID
       return DEVICE_CCS811;

    // Check for LSM9DS1 magnetometer/gyro/accel sensor from STMicro
    I2CReadRegister(i, 0x0f, cTemp, 1);
    if (cTemp[0] == 0x68) // WHO_AM_I
       return DEVICE_LSM9DS1;

    // Check for LPS25H pressure sensor from STMicro
    I2CReadRegister(i, 0x0f, cTemp, 1);
    if (cTemp[0] == 0xbd) // WHO_AM_I
       return DEVICE_LPS25H;
    
    // Check for HTS221 temp/humidity sensor from STMicro
    I2CReadRegister(i, 0x0f, cTemp, 1);
    if (cTemp[0] == 0xbc) // WHO_AM_I
       return DEVICE_HTS221;
    
    // Check for MAG3110
    I2CReadRegister(i, 0x07, cTemp, 1);
    if (cTemp[0] == 0xc4) // WHO_AM_I
       return DEVICE_MAG3110;

    // Check for LM8330 keyboard controller
    I2CReadRegister(i, 0x80, cTemp, 2);
    if (cTemp[0] == 0x0 && cTemp[1] == 0x84) // manufacturer code + software revision
       return DEVICE_LM8330;

    // Check for MAX44009
    if (i == 0x4a || i == 0x4b)
    {
      for (j=0; j<8; j++)
        I2CReadRegister(i, j, &cTemp[j], 1); // check for power-up reset state of registers
      if ((cTemp[2] == 3 || cTemp[2] == 2) && cTemp[6] == 0 && cTemp[7] == 0xff)
         return DEVICE_MAX44009;
    }
       
    // Check for ADS1115
    I2CReadRegister(i, 0x02, cTemp, 2); // Lo_thresh defaults to 0x8000
    I2CReadRegister(i, 0x03, &cTemp[2], 2); // Hi_thresh defaults to 0x7fff
    if (cTemp[0] = 0x80 && cTemp[1] == 0x00 && cTemp[2] == 0x7f && cTemp[3] == 0xff)
       return DEVICE_ADS1115;

    // Check for MCP9808
    I2CReadRegister(i, 0x06, cTemp, 2); // manufacturer ID && get device ID/revision
    I2CReadRegister(i, 0x07, &cTemp[2], 2); // need to read them individually
    if (cTemp[0] == 0 && cTemp[1] == 0x54 && cTemp[2] == 0x04 && cTemp[3] == 0x00)
       return DEVICE_MCP9808;
       
    // Check for BMP280/BME280
    I2CReadRegister(i, 0xd0, cTemp, 1);
    if (cTemp[0] == 0x55) // BMP180
       return DEVICE_BMP180;
    else if (cTemp[0] == 0x58)
       return DEVICE_BMP280;
    else if (cTemp[0] == 0x60) // BME280
       return DEVICE_BME280;

    // Check for LSM6DS3
    I2CReadRegister(i, 0x0f, cTemp, 1); // WHO_AM_I
    if (cTemp[0] == 0x69)
       return DEVICE_LSM6DS3;
       
    // Check for ADXL345
    I2CReadRegister(i, 0x00, cTemp, 1); // DEVID
    if (cTemp[0] == 0xe5)
       return DEVICE_ADXL345;
       
    // Check for MPU-60x0 and MPU-9250
    I2CReadRegister(i, 0x75, cTemp, 1);
    if (cTemp[0] == (i & 0xfe)) // Current I2C address (low bit set to 0)
       return DEVICE_MPU6000;
    else if (cTemp[0] == 0x71)
       return DEVICE_MPU9250;

    // Check for DS3231 RTC
    I2CReadRegister(i, 0x0e, cTemp, 1); // read the control register
    if (i == 0x68 && cTemp[0] == 0x1c) // fixed I2C address and power on reset value  
       return DEVICE_DS3231;
  }
  return iDevice;
}
void setup() {
#ifdef USE_BITBANG
  I2CInit(SDA_PIN, SCL_PIN, 100000L);
#else
  Wire.begin();
#endif
  delay(100); // allow devices to power up
  Serial.begin(9600);
}

void loop() {
uint8_t map[16];
uint8_t i;
int iDevice;

  Serial.println("Starting I2C Scan");
  I2CScan(map); // get bitmap of connected I2C devices
  if (map[0] == 0xff) // something is wrong with the I2C bus
  {
    Serial.println("I2C bus is being pulled low by a bad device; unable to run scan");
  }
  else
  {
    for (i=1; i<128; i++) // skip address 0 (general call address) since more than 1 device can respond
    {
      if (map[i>>3] & (1 << (i & 7))) // device found
      {
        Serial.print("Device found at 0x");
        Serial.print(i, HEX);
        iDevice = DiscoverDevice(i);
        Serial.print(", type = ");
  #ifdef SHOW_NAME
        Serial.println(szNames[iDevice]); // show the device name as a string
  #else
        Serial.println(iDevice); // show the device name as the enum index
  #endif
      }
    }
  }
  delay(5000);
}
