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

#ifndef __SENSOR_TYPE_H_INCLUDED__
#define __SENSOR_TYPE_H_INCLUDED__

#include <string>
#include <vector>

/** SensorType Abstract class.
 *  @brief  Used as an interface between SensorManager class and individual Sensor Drivers
 *
 *  Example:
 *  @code{.cpp}
 *  #include "mbed.h"
 *  #include "sensor_type.h"
 *
 *  class ExampleSensor : public Sensor
 *  {
 *  public:
 * 		virtual int GetName() { return "sensorname"; };
 * 		virtual std::vector<std::pair<std::string, std::string>> GetData();
 * 
 * 		// other public methods and members
 * 
 * 	private:
 * 		// private methods and members
 * 
 *  }
 *  @endcode
 */

class SensorType {
public:
	enum SensorStatus {
		DISCONNECT,
        CONNECT,
        DATA_OK,
        DATA_CRC_ERR,
        DATA_NOT_RDY,
		DATA_OUT_OF_RANGE,
	};
	
	virtual std::string GetName() = 0;
	virtual int GetData(std::vector<std::pair<std::string, std::string>>&) = 0;  // getting data
	virtual void Enable() = 0;
	virtual void Disable() = 0;
	virtual void Reset() = 0;
	
	std::string ConvertDataToString(float data);
	int ValidateData(float data, float data_min, float data_max);

	virtual ~SensorType() {};

	int error_counter;							// counter for error occurence (to reduce random error noise)
	std::vector<std::string> data_oor_list;		// vector of strings containing out_of_range data info
};

#endif