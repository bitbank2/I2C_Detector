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
#include <BitBang_I2C.h>

// Arbitrary pins I used for testing with an ATmega328p
// Define as -1, -1 to use the Wire library over the default I2C interface
#define SDA_PIN -1
#define SCL_PIN -1

//
// If you don't need the explicit device names displayed, disable this code by
// commenting out the next line
//
#define SHOW_NAME
#ifdef SHOW_NAME
const char *szNames[]  = {"Unknown","SSD1306","SH1106","VL53L0X","BMP180", "BMP280","BME280",
                "MPU-60x0", "MPU-9250", "MCP9808","LSM6DS3", "ADXL345", "ADS1115","MAX44009",
                "MAG3110", "CCS811", "HTS221", "LPS25H", "LSM9DS1","LM8330", "DS3231", "LIS3DH",
                "LIS3DSH","INA219","SHT3X","HDC1080"};
#endif

void setup() {
  Serial.begin(9600);
  I2CInit(SDA_PIN, SCL_PIN, 100000L);
  delay(100); // allow devices to power up
}

void loop() {
uint8_t map[16];
uint8_t i;
int iDevice, iCount;

  Serial.println("Starting I2C Scan");
  I2CScan(map); // get bitmap of connected I2C devices
  if (map[0] == 0xfe) // something is wrong with the I2C bus
  {
    Serial.println("I2C pins are not correct or the bus is being pulled low by a bad device; unable to run scan");
  }
  else
  {
    iCount = 0;
    for (i=1; i<128; i++) // skip address 0 (general call address) since more than 1 device can respond
    {
      if (map[i>>3] & (1 << (i & 7))) // device found
      {
        iCount++;
        Serial.print("Device found at 0x");
        Serial.print(i, HEX);
        iDevice = I2CDiscoverDevice(i);
        Serial.print(", type = ");
  #ifdef SHOW_NAME
        Serial.println(szNames[iDevice]); // show the device name as a string
  #else
        Serial.println(iDevice); // show the device name as the enum index
  #endif
      }
    } // for i
    Serial.print(iCount, DEC);
    Serial.println(" device(s) found");
  }
  delay(5000);
}
