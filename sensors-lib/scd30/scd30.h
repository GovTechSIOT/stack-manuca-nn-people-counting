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

#ifndef SCD30_H
#define SCD30_H

#define SCD30_I2C_ADDR                  0xc2

#define SCD30_CMMD_STRT_CONT_MEAS       0x0010
#define SCD30_CMMD_STOP_CONT_MEAS       0x0104
#define SCD30_CMMD_SET_MEAS_INTVL       0x4600
#define SCD30_CMMD_GET_READY_STAT       0x0202
#define SCD30_CMMD_READ_MEAS            0x0300
#define SCD30_CMMD_D_A_SELF_CALIB       0x5306
#define SCD30_CMMD_FORCE_CALIB_VAL      0x5204
#define SCD30_CMMD_SET_TEMP_OFFS        0x5403
#define SCD30_CMMD_SET_ALT_COMP         0x5102
#define SCD30_CMMD_SOFT_RESET           0xd304
#define SCD30_CMMD_ASC                  0x5306
#define SCD30_CMMD_FRC                  0x5204

#define SCD30_CMMD_READ_SERIALNBR       0xD033
    
#define SCD30_POLYNOMIAL                0x31   // P(x) = x^8 + x^5 + x^4 + 1 = 100110001
#define SCD30_CRC_INIT                  0xff

#define SCD30_SN_SIZE                   33      //size of the s/n ascii string + CRC values

#define CO2_MAX         10000.00f
#define CO2_MIN         0.00f
#define TEMP_MAX        70.00f
#define TEMP_MIN        -40.00f
#define HUM_MAX         100.00f
#define HUM_MIN         0.00f

#define I2C_FREQUENCY   400000

/** Create SCD30 driver class
 * @brief Driver for the SCD30 CO2, RH/T sensor
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
 *      SensorType *scd30 = new Scd30(sda, scl, freq);
 *      scd30->Enable();
 *      sensor_masterlist_.push_back(scd30);
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
class Scd30 : public SensorType {

public:
    Scd30(PinName sda, PinName scl, int i2c_frequency);
    ~Scd30();
    std::string GetName();
    int GetData(std::vector<std::pair<std::string, std::string>>&);
    void Enable();
	void Disable();
    // void Configure();   // To be done in SENP-286
	void Reset();
 
private:
    
    enum SCDError {
        SCDNOERROR,         //all ok
        SCDISREADY,         //ready status register
        SCDNOACKERROR,      //no I2C ACK error
        SCDCRCERROR,        //CRC error, any
    };

    uint8_t sn[24];         /**< ASCII Serial Number */

    uint16_t scd_ready;     /* 1 = ready, 0 = busy */
    uint16_t meas_interval; /* measurement interval */

    float co2f;             /* float of CO2 concentration */
    float tempf;            /* float of Temp */
    float humf;             /* float of Hum */
    
    char i2cbuff[34];
    
    uint16_t co2m;          /**< High order 16 bit word of CO2 */
    uint16_t co2l;          /**< Low  order 16 bit word of CO2 */
    uint16_t tempm;         /**< High order 16 bit word of Temp */
    uint16_t templ;         /**< Low order 16 bit word of Temp */
    uint16_t humm;          /**< High order 16 bit word of Hum */
    uint16_t huml;          /**< Low order 16 bit word of Hum */

    uint32_t co2i;          /* 32 bit int of CO2 */
    uint32_t tempi;         /* 32 bit int of Temp */
    uint32_t humi;          /* 32 bit int of Hum */

    uint8_t StartMeasurement(uint16_t baro);
    uint8_t StopMeasurement();
    uint8_t SetMeasInterval(uint16_t interval);
    uint8_t ReadMeasInterval();
    uint8_t GetSerialNumber();
    uint8_t GetReadyStatus();
    uint8_t ReadMeasurement();
    uint8_t SetTemperatureOffs(uint16_t temp);
    uint8_t SetAltitudeComp(uint16_t alt);
    uint8_t ActivateASC(uint16_t activate);
    uint8_t SetFRCvalue(uint16_t conc);
    uint8_t SoftReset();
    uint8_t CalcCrc2b(uint16_t seed);
    uint8_t CheckCrc2b(uint16_t seed, uint8_t crcIn);
    
protected:
    I2C     _i2c;    

};    
#endif
