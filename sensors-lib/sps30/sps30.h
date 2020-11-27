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

#ifndef SPS30_H
#define SPS30_H

#define SPS30_I2C_ADDR                  0xd2    //  left shifted by 1 bit from 0x69

#define SPS30_CMMD_STRT_MEAS            0x0010
#define SPS30_CMMD_STOP_MEAS            0x0104
#define SPS30_CMMD_GET_READY_STAT       0x0202
#define SPS30_CMMD_READ_MEAS            0x0300

#define SPS30_CMMD_AUTO_CLEAN_INTV      0x8004
#define SPS30_CMMD_START_FAN_CLEAN      0x5607

#define SPS30_CMMD_SOFT_RESET           0xD304

#define SPS30_CMMD_READ_SERIALNBR       0xD033
#define SPS30_CMMD_READ_ARTICLECODE     0xD025

#define SPS30_STRT_MEAS_WRITE_DATA      0x0300
    
#define SPS30_POLYNOMIAL                0x31    // P(x) = x^8 + x^5 + x^4 + 1 = 100110p01
#define SPS30_CRC_INIT                  0xff

#define SPS30_SN_SIZE                   33      // size of the s/n ascii string + CRC values

#define MASS_MAX            1000.00f
#define MASS_MIN            0.00f
#define NUM_MAX             3000.00f
#define NUM_MIN             0.00f

#define I2C_FREQUENCY_STD   100000              // SPS30 uses 100MHz for I2C communication

/** Create SPS30 controller class
 * @brief Driver for the SPS30 Particulate Matter sensor
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
 *      SensorType *sps30 = new Sps30(sda, scl, freq);
 *      sps30->Enable();
 *      sensor_masterlist_.push_back(sps30);
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
class Sps30 : public SensorType {

public:

     Sps30(PinName sda, PinName scl, int i2c_frequency);
    ~Sps30();
    std::string GetName();
    int GetData(std::vector<std::pair<std::string, std::string>>& data_list);
    void Enable();
	void Disable();
	// void Configure();   // To be done in SENP-286
	void Reset();
 
private:

    enum SPSError {
        SPSNOERROR,         //all ok
        SPSISREADY,         //ready status register
        SPSNOACKERROR,      //no I2C ACK error
        SPSCRCERROR,        //CRC error, any
    };

    uint8_t sn[33];     /**< ASCII Serial Number */

    uint16_t sps_ready;            /**< 1 = ready, 0 = busy */
    uint32_t clean_interval_i;     /** 32 unsigned bit in seconds */

    float mass_1p0_f;       /**< float of Mass Conc of PM1.0 */
    float mass_2p5_f;       /**< float of Mass Conc of PM2.5 */
    float mass_4p0_f;       /**< float of Mass Conc of PM4.0 */
    float mass_10p0_f;      /**< float of Mass Conc of PM10 */

    float num_0p5_f;        /**< float of Number Conc of PM0.5 */
    float num_1p0_f;        /**< float of Number Conc of PM1.0 */
    float num_2p5_f;        /**< float of Number Conc of PM2.5 */
    float num_4p0_f;        /**< float of Number Conc of PM4.0 */
    float num_10p0_f;       /**< float of Number Conc of PM10 */

    float typ_pm_size_f;    /**< float of Typical Particle Size */
    
    char i2cbuff[60];
    
    uint16_t clean_interval_m;    /**< High order 16 bit word of Auto Clean Interval */
    uint16_t clean_interval_l;    /**< High order 16 bit word of Auto Clean Interval */

    uint16_t mass_1p0_m;          /**< High order 16 bit word of Mass Conc of PM1.0 */
    uint16_t mass_1p0_l;          /**< Low order 16 bit word of Mass Conc of PM1.0 */
    uint16_t mass_2p5_m;          /**< High order 16 bit word of Mass Conc of PM2.5 */
    uint16_t mass_2p5_l;          /**< Low order 16 bit word of Mass Conc of PM2.5 */
    uint16_t mass_4p0_m;          /**< High order 16 bit word of Mass Conc of PM4.0 */
    uint16_t mass_4p0_l;          /**< Low order 16 bit word of Mass Conc of PM4.0 */
    uint16_t mass_10p0_m;         /**< High order 16 bit word of Mass Conc of PM10 */
    uint16_t mass_10p0_l;         /**< Low order 16 bit word of Mass Conc of PM10 */

    uint16_t num_0p5_m;           /**< High order 16 bit word of Number Conc of PM0.5 */
    uint16_t num_0p5_l;           /**< Low order 16 bit word of Number Conc of PM0.5 */
    uint16_t num_1p0_m;           /**< High order 16 bit word of Number Conc of PM1.0 */
    uint16_t num_1p0_l;           /**< Low order 16 bit word of Number Conc of PM1.0 */
    uint16_t num_2p5_m;           /**< High order 16 bit word of Number Conc of PM2.5 */
    uint16_t num_2p5_l;           /**< Low order 16 bit word of Number Conc of PM2.5 */
    uint16_t num_4p0_m;           /**< High order 16 bit word of Number Conc of PM4.0 */
    uint16_t num_4p0_l;           /**< Low order 16 bit word of Number Conc of PM4.0 */
    uint16_t num_10p0_m;          /**< High order 16 bit word of Number Conc of PM10 */
    uint16_t num_10p0_l;          /**< Low order 16 bit word of Number Conc of PM10 */

    uint16_t typ_pm_size_m;       /**< High order 16 bit word of Typical Particle Size */
    uint16_t typ_pm_size_l;       /**< Low order 16 bit word of Typical Particle Size */

    uint32_t mass_1p0_i;          /**< 32 bit int of Mass Conc of PM1.0 */
    uint32_t mass_2p5_i;          /**< 32 bit int of Mass Conc of PM2.5 */
    uint32_t mass_4p0_i;          /**< 32 bit int of Mass Conc of PM4.0 */
    uint32_t mass_10p0_i;         /**< 32 bit int of Mass Conc of PM10 */

    uint32_t num_0p5_i;           /**< 32 bit int of Number Conc of PM0.5 */
    uint32_t num_1p0_i;           /**< 32 bit int of Number Conc of PM1.0 */
    uint32_t num_2p5_i;           /**< 32 bit int of Number Conc of PM2.5 */
    uint32_t num_4p0_i;           /**< 32 bit int of Number Conc of PM4.0 */
    uint32_t num_10p0_i;          /**< 32 bit int of Number Conc of PM10 */

    uint32_t typ_pm_size_i;       /**< 32 bit int of Typical Particle Size */

    uint8_t StartMeasurement();
    uint8_t StopMeasurement();
    uint8_t GetSerialNumber();
    uint8_t GetReadyStatus();
    uint8_t ReadMeasurement();
    uint8_t ReadAutoCleanInterval();
    uint8_t SetAutoCleanInterval(uint32_t set_interval = 604800);
    uint8_t StartFanClean();
    uint8_t SoftReset();
    uint8_t CalcCrc2b(uint16_t seed);
    uint8_t CheckCrc2b(uint16_t seed, uint8_t crc_in);

protected:
    I2C     _i2c;    

};    
#endif
