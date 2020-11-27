/*******************************************************************************************************
 * Copyright (c) 2018-2019 Government Technology Agency of Singapore (GovTech)
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied.
 *
 * See the License for the specific language governing permissions and limitations under the License.
 *******************************************************************************************************/

#include "sensors-lib/sensor_type.h"

#ifndef TMP75_H
#define TMP75_H

#define TMP75_I2C_ADDR			0x96	

#define TMP75_CMMD_READ_TEMP_REG	0x00
#define TMP75_CMMD_CONFIG_REG		0x01
#define TMP75_CMMD_TEMP_LOW_REG	0x02
#define TMP75_CMMD_TEMP_HIGH_REG	0x03

#define TMP75_CONF_ONE_SHOT		0x80
#define TMP75_CONF_CONV_RATE		0x60
#define TMP75_CONF_FAULT_Q		0x18
#define TMP75_CONF_ALRT_POL		0x04
#define TMP75_CONF_ALRT_MODE		0x02
#define TMP75_CONF_SD_MODE		0x01

#define TMP75_CONF_RESERVED		0xFF

#define TMP75_ACTIVE_LOW			0
#define TMP75_ACTIVE_HIGH		1

#define TMP75_I2C_FREQUENCY		400000

#define TMP75_UPPER_LIMIT		125.00
#define TMP75_LOWER_LIMIT		-40.00

#define TMP75_RESOLUTION			0.0625

/** Create TMP75 driver class
 * @brief Driver for the TMP75 Temperature sensor
 * Inherits SensorType virtual functions essential for interfacing with SensorManager
 *
 * Example:
 * @code{.cpp}
 * // in class SensorManager
 * 
 * std::vector<SensorType*> sensor_masterlist_;
 * 
 * void SensorManager::CreateSensor()
 * {
 *      SensorType *tmp75 = new Tmp75(sda, scl);
 *      tmp75->Enable();
 *      sensor_masterlist_.push_back(tmp75);
 * }
 * 
 * void SensorManager::GetSensorsData()
 * {
 *      for (int i=0; i<sensor_masterlist_.size(); i++)
 *      {
 *          std::vector<std::pair<std::string, std::string>> data;
 *          SensorType *snr = sensor_masterlist[i];
 *          int stat = snr->GetData(data);
 *      }
 * }
 * @endcode
 */

class Tmp75 : public SensorType {

public:
	enum TmpStatus {
		TMPNOACK,
		TMPACK,
		TMPOK,
		TMPALERT,
		TMPCONFIGFAIL,
	};

	struct {
		bool oneshot;	// one-shot set as 1 to start one shot measurement (only in shutdown mode, always reads as 0)
		bool pol;		// polarity ( 0 [default] = active low )
		bool tmode;	// thermostat mode ( 0 [default] = comparator mode | 1 = interrupt mode )
		bool shutdown;	// shutdown mode ( 0 [default] = cont. conversion mode | 1 = shutdown mode )
		int fqueue;	// fault queue ( 0 [default] = 1 fault | 1 = 2 faults | 2 = 4 faults | 3 = 6 faults )
		int cvrate;	// conversion rate for continuous-conversion mode ( 0 [default] = 37Hz | 1 = 18Hz | 2 = 9Hz | 3 = 4Hz )
	} tmp_config_t;

	Tmp75(PinName sda, PinName scl, PinName alert_pin=PinName::PD_10, int i2c_frequency=TMP75_I2C_FREQUENCY);
	~Tmp75();

	std::string GetName();
	int GetData(std::vector<std::pair<std::string, std::string>>& data_list);
	void Enable();
	void Disable();
	void Reset();

	int Configure(float t_low=40, float t_high=50);

private:
	char i2cbuff[4];

	bool active_;	// 0: deactivated, 1: active

	float temp_high_;
	float temp_low_;
	float temp_data_;

	uint8_t StartOneMeasurement();
	uint8_t StartMeasurement();
	uint8_t StopMeasurement();
	void HardReset();
	uint8_t SetConfigReg();
	uint8_t SetTempLow(float val);
	uint8_t SetTempHigh(float val);
	uint8_t ReadConfigReg();
	uint8_t ReadTempLow();
	uint8_t ReadTempHigh();
	uint8_t ReadTemp();
	uint8_t ReadAlert();
	float GetTempData();
	float GetTempLow();
	float GetTempHigh();

protected:
	I2C 		_i2c;
	DigitalIn	_alert;

};

#endif
