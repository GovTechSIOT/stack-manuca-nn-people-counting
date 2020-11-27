/*******************************************************************************************************
 * Copyright (c) 2018-2020 Government Technology Agency of Singapore (GovTech)
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
#ifndef SENSOR_PROFILE_H
#define SENSOR_PROFILE_H

#include <string>
#include <vector>
#include <unordered_map>

/** SensorProfile class.
 *  @brief  Used to create a sensor profile for all sensor entities
 *
 *  Example:
 *  @code{.cpp}
 *  #include "mbed.h"
 *  #include "sensor_profile.h"
 *
 *  int main() 
 *  {
 *      SensorProfile sensors_profile;     
 *      sensors_profile.UpdateValue("temperature", "23.45", 0);
 *      printf("\r\n %s \r\n", sensors_profile.GetNewPacket());
 *  }
 *  @endcode
 */

class SensorProfile 
{
    public:
        /// Public exposed methods
        void UpdateValue(std::string entity_name, std::string value, int timestamp);        /// mailbox receives from sensor thread; struct { char* sensor_type, char* value, int raw_time_stamp }
        void ClearEntityList(void);
        void UpdateEntityList(int time_stamp);
        bool CheckEntityAvailability();
        std::string GetNewDecadaPacket();

    private:
        /// Internal methods used within the class
        std::string CreateDecadaPacket(void);                                               /// putting it altogeter: create json array with nested ojects and arrays that is DECADAcloud-compliant

        /// Class member variables
        const std::string decada_protocol_version_ = "1.0";                                 /// version of decada-compliant json protocol
        const std::string decada_method_of_device_ = "thing.measurepoint.post";             /// version of decada-compliant json protocol

        std::unordered_map<std::string, std::pair<std::string, int>> entity_value_pairs_;   /// collation of entity and value, timestamp pairs
};

#endif  // SENSOR_PROFILE_H