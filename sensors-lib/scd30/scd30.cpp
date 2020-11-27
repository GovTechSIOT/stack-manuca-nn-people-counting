#include "mbed.h"
#include "scd30.h"

/** Create a SCD30 object using the specified I2C object
 * @param sda - mbed I2C interface pin
 * @param scl - mbed I2C interface pin
 * @param I2C Frequency (in Hz)
 *
 * @return none
 */
Scd30::Scd30(PinName sda, PinName scl, int i2c_frequency)  : _i2c(sda, scl) {
        _i2c.frequency(i2c_frequency);
}

/** SCD30 Destructor
 *
 */
Scd30::~Scd30() {
}

/** Start Auto-Measurement 
 *
 * @param Barometer reading (in mB) or 0x0000
 *
 * @return enum SCDerror
 */
uint8_t Scd30::StartMeasurement(uint16_t baro)
{
    i2cbuff[0] = SCD30_CMMD_STRT_CONT_MEAS >> 8;
    i2cbuff[1] = SCD30_CMMD_STRT_CONT_MEAS & 255;
    i2cbuff[2] = baro >> 8;
    i2cbuff[3] = baro & 255;
    i2cbuff[4] = Scd30::CalcCrc2b(baro);
    int res = _i2c.write(SCD30_I2C_ADDR, i2cbuff, 5, false);
    if(res) return SCDNOACKERROR;
    return SCDNOERROR;
}

/** Stop Auto-Measurement 
 *
 * @return enum SCDerror
 */
uint8_t Scd30::StopMeasurement()
{
    i2cbuff[0] = SCD30_CMMD_STOP_CONT_MEAS >> 8;
    i2cbuff[1] = SCD30_CMMD_STOP_CONT_MEAS & 255;
    int res = _i2c.write(SCD30_I2C_ADDR, i2cbuff, 2, false);
    if(res) return SCDNOACKERROR;
    return SCDNOERROR;
}

/** Set Measurement Interval 
 *
 * @param Time between measurements (in seconds)
 *
 * @return enum SCDerror
 */
uint8_t Scd30::SetMeasInterval(uint16_t interval)
{
    i2cbuff[0] = SCD30_CMMD_SET_MEAS_INTVL >> 8;
    i2cbuff[1] = SCD30_CMMD_SET_MEAS_INTVL & 255;
    i2cbuff[2] = interval >> 8;
    i2cbuff[3] = interval & 255;
    i2cbuff[4] = Scd30::CalcCrc2b(interval);
    int res = _i2c.write(SCD30_I2C_ADDR, i2cbuff, 5, false);
    if(res) return SCDNOACKERROR;
    return SCDNOERROR;
}

/** Read Measurement Interval 
 *
 * @see Measurement Interval in private member variable
 *
 * @return enum SCDerror
 */
uint8_t Scd30::ReadMeasInterval()
{
    i2cbuff[0] = SCD30_CMMD_SET_MEAS_INTVL >> 8;
    i2cbuff[1] = SCD30_CMMD_SET_MEAS_INTVL & 255;
    int res = _i2c.write(SCD30_I2C_ADDR, i2cbuff, 2, false);
    if(res) return SCDNOACKERROR;
    
    _i2c.read(SCD30_I2C_ADDR | 1, i2cbuff, 3, false);
    uint16_t stat = (i2cbuff[0] << 8) | i2cbuff[1];
    meas_interval = stat;
    uint8_t dat = Scd30::CheckCrc2b(stat, i2cbuff[2]);
    
    if(dat == SCDCRCERROR) return SCDCRCERROR;
    return SCDNOERROR;
}

/** Get Serial Number
 *
 * @see ASCII Serial Number in private member variables
 *
 * @return enum SCDerror
 */
uint8_t Scd30::GetSerialNumber()
{
    i2cbuff[0] = SCD30_CMMD_READ_SERIALNBR >> 8;
    i2cbuff[1] = SCD30_CMMD_READ_SERIALNBR & 255;
    int res = _i2c.write(SCD30_I2C_ADDR, i2cbuff, 2, false);
    if(res) return SCDNOACKERROR;
    
    int i = 0;
    for(i = 0; i < sizeof(sn); i++) sn[i] = 0;
    for(i = 0; i < sizeof(i2cbuff); i++) i2cbuff[i] = 0;
    
    _i2c.read(SCD30_I2C_ADDR | 1, i2cbuff, SCD30_SN_SIZE, false);
    int t = 0;
    for(i = 0; i < SCD30_SN_SIZE; i +=3) {
        uint16_t stat = (i2cbuff[i] << 8) | i2cbuff[i + 1];
        sn[i - t] = stat >> 8;
        sn[i - t + 1] = stat & 255;
        uint8_t dat = Scd30::CheckCrc2b(stat, i2cbuff[i + 2]);
        t++;
        if(dat == SCDCRCERROR) return SCDCRCERROR;
        if(stat == 0) break;
    }

    return SCDNOERROR;
}

/** Get Ready Status register 
 *
 * @see Ready Status result in Scd_ready
 *
 * @return enum SCDerror
 */
uint8_t Scd30::GetReadyStatus()
{
    i2cbuff[0] = SCD30_CMMD_GET_READY_STAT >> 8;
    i2cbuff[1] = SCD30_CMMD_GET_READY_STAT & 255;
    int res = _i2c.write(SCD30_I2C_ADDR, i2cbuff, 2, false);
    if(res) return SCDNOACKERROR;
    
    _i2c.read(SCD30_I2C_ADDR | 1, i2cbuff, 3, false);
    uint16_t stat = (i2cbuff[0] << 8) | i2cbuff[1];
    scd_ready = stat;
    uint8_t dat = Scd30::CheckCrc2b(stat, i2cbuff[2]);
    
    if(dat == SCDCRCERROR) return SCDCRCERROR;
    return SCDNOERROR;
}

/** Get all data values (CO2, Temp and Hum) 
 *
 * @see Results in private member variables
 *
 * @return enum SCDerror
 */
uint8_t Scd30::ReadMeasurement()
{
    i2cbuff[0] = SCD30_CMMD_READ_MEAS >> 8;
    i2cbuff[1] = SCD30_CMMD_READ_MEAS & 255;
    int res = _i2c.write(SCD30_I2C_ADDR, i2cbuff, 2, false);
    if(res) return SCDNOACKERROR;
    
    _i2c.read(SCD30_I2C_ADDR | 1, i2cbuff, 18, false);
    
    uint16_t stat = (i2cbuff[0] << 8) | i2cbuff[1];
    co2m = stat;
    uint8_t dat = Scd30::CheckCrc2b(stat, i2cbuff[2]);
    if(dat == SCDCRCERROR) return SCDCRCERROR;
    
    stat = (i2cbuff[3] << 8) | i2cbuff[4];
    co2l = stat;
    dat = Scd30::CheckCrc2b(stat, i2cbuff[5]);
    if(dat == SCDCRCERROR) return SCDCRCERROR;
    
    stat = (i2cbuff[6] << 8) | i2cbuff[7];
    tempm = stat;
    dat = Scd30::CheckCrc2b(stat, i2cbuff[8]);
    if(dat == SCDCRCERROR) return SCDCRCERROR;
    
    stat = (i2cbuff[9] << 8) | i2cbuff[10];
    templ = stat;
    dat = Scd30::CheckCrc2b(stat, i2cbuff[11]);
    if(dat == SCDCRCERROR) return SCDCRCERROR;
    
    stat = (i2cbuff[12] << 8) | i2cbuff[13];
    humm = stat;
    dat = Scd30::CheckCrc2b(stat, i2cbuff[14]);
    if(dat == SCDCRCERROR) return SCDCRCERROR;
    
    stat = (i2cbuff[15] << 8) | i2cbuff[16];
    huml = stat;
    dat = Scd30::CheckCrc2b(stat, i2cbuff[17]);
    if(dat == SCDCRCERROR) return SCDCRCERROR;
    
    co2i = (co2m << 16) | co2l ;
    tempi = (tempm << 16) | templ ;
    humi = (humm << 16) | huml ;
    
    co2f = *(float*)&co2i;
    tempf = *(float*)&tempi;
    humf = *(float*)&humi;
    
    return SCDNOERROR;
}

/** Set Temperature offset
 *
 * @param Temperature offset (value in 0.01 degrees C)
 *
 * @return enum SCDerror
 */
uint8_t Scd30::SetTemperatureOffs(uint16_t temp)
{
    i2cbuff[0] = SCD30_CMMD_SET_TEMP_OFFS >> 8;
    i2cbuff[1] = SCD30_CMMD_SET_TEMP_OFFS & 255;
    i2cbuff[2] = temp >> 8;
    i2cbuff[3] = temp & 255;
    i2cbuff[4] = Scd30::CalcCrc2b(temp);
    int res = _i2c.write(SCD30_I2C_ADDR, i2cbuff, 5, false);
    if(res) return SCDNOACKERROR;
    return SCDNOERROR;
}

/** Set Altitude Compensation
 *
 * @param Altitude (in meters)
 *
 * @return enum SCDerror
 */
uint8_t Scd30::SetAltitudeComp(uint16_t alt)
{
    i2cbuff[0] = SCD30_CMMD_SET_ALT_COMP >> 8;
    i2cbuff[1] = SCD30_CMMD_SET_ALT_COMP & 255;
    i2cbuff[2] = alt >> 8;
    i2cbuff[3] = alt & 255;
    i2cbuff[4] = Scd30::CalcCrc2b(alt);
    int res = _i2c.write(SCD30_I2C_ADDR, i2cbuff, 5, false);
    if(res) return SCDNOACKERROR;
    return SCDNOERROR;
}

/** Activate or Deactivate Automatic Self-Calibration (ASC)
 *
 * @param Activate (1), Deactivate (0)
 *
 * @return enum SCDerror
 */
uint8_t Scd30::ActivateASC(uint16_t activate)
{
    i2cbuff[0] = SCD30_CMMD_ASC >> 8;
    i2cbuff[1] = SCD30_CMMD_ASC & 255;
    i2cbuff[2] = activate >> 8;
    i2cbuff[3] = activate & 255;
    i2cbuff[4] = Scd30::CalcCrc2b(activate);
    int res = _i2c.write(SCD30_I2C_ADDR, i2cbuff, 2, false);
    if(res) return SCDNOACKERROR;
    return SCDNOERROR;
}

/** Set Forced Recalibration value (FRC)
 *
 * @param Reference CO2 concentration (in ppm)
 * 
 * @return enum SCDerror
 */
uint8_t Scd30::SetFRCvalue(uint16_t conc)
{
    i2cbuff[0] = SCD30_CMMD_FRC >> 8;
    i2cbuff[1] = SCD30_CMMD_FRC & 255;
    i2cbuff[2] = conc >> 8;
    i2cbuff[3] = conc & 255;
    i2cbuff[4] = Scd30::CalcCrc2b(conc);
    int res = _i2c.write(SCD30_I2C_ADDR, i2cbuff, 2, false);
    if(res) return SCDNOACKERROR;
    return SCDNOERROR;
}

/** Perform a soft reset
 *
 * @return enum SCDerror
 */
uint8_t Scd30::SoftReset()
{
    i2cbuff[0] = SCD30_CMMD_SOFT_RESET >> 8;
    i2cbuff[1] = SCD30_CMMD_SOFT_RESET & 255;
    int res = _i2c.write(SCD30_I2C_ADDR, i2cbuff, 2, false);
    if(res) return SCDNOACKERROR;
    return SCDNOERROR;
}
    
/** Calculate the SCD30 CRC value
 *
 * @param 16 bit value to perform a CRC check on
 *
 * @return 8 bit CRC value
 */
uint8_t Scd30::CalcCrc2b(uint16_t seed)
{
  uint8_t bit;                  // bit mask
  uint8_t crc = SCD30_CRC_INIT; // calculated checksum
  
  // calculates 8-Bit checksum with given polynomial

    crc ^= (seed >> 8) & 255;
    for(bit = 8; bit > 0; --bit)
    {
      if(crc & 0x80) crc = (crc << 1) ^ SCD30_POLYNOMIAL;
      else           crc = (crc << 1);
    }

    crc ^= seed & 255;
    for(bit = 8; bit > 0; --bit)
    {
      if(crc & 0x80) crc = (crc << 1) ^ SCD30_POLYNOMIAL;
      else           crc = (crc << 1);
    }
    
  return crc;
}

/** Compare received CRC value with calculated CRC value
 *
 * @param 16 bit value to perform a CRC check on
 * @param 8 bit value to compare CRC values
 *
 * @return enum SCDerror
 */
uint8_t Scd30::CheckCrc2b(uint16_t seed, uint8_t crcIn)
{
    uint8_t crcCalc = Scd30::CalcCrc2b(seed);
    if(crcCalc != crcIn) return SCDCRCERROR;
    return SCDNOERROR;
}

/**************************   INTERFACE METHODS   *******************************/

/** Get Sensor Name (Overrides SensorType virtual func)
 *
 * @return sensor name in string
 */
std::string Scd30::GetName()
{
    return "scd30";
}

/** Get Sensor Data (Overrides SensorType virtual func)
 * 
 * @param   data_list   vector of string pairs (data_name, data_value)
 *
 * @return  enum SensorStatus in SensorType base class
 */
int Scd30::GetData(std::vector<std::pair<std::string, std::string>>& data_list)
{
    uint8_t dat = GetReadyStatus();
    if (dat == SCDNOACKERROR)
    {
        return SensorType::DISCONNECT;
    }

    if (scd_ready == SCDISREADY)
    {
        uint8_t crcc = ReadMeasurement();
        if (crcc == SCDNOERROR)
        {
            data_oor_list.clear();
            std::string co2 = ConvertDataToString(co2f);
            std::string temp = ConvertDataToString(tempf);
            std::string hum = ConvertDataToString(humf);

            int ret = ValidateData(co2f, CO2_MIN, CO2_MAX);
            if (ret == DATA_OUT_OF_RANGE)
            {
                std::string msg = "_co2_out_of_range_" + co2;
                data_oor_list.push_back(msg);
            }
            data_list.push_back(make_pair("co2", co2));

            ret = ValidateData(tempf, TEMP_MIN, TEMP_MAX);
            if (ret == DATA_OUT_OF_RANGE)
            {
                std::string msg = "_temperature_out_of_range_" + temp;
                data_oor_list.push_back(msg);
            }
            data_list.push_back(make_pair("temperature", temp));

            ret = ValidateData(humf, HUM_MIN, HUM_MAX);
            if (ret == DATA_OUT_OF_RANGE)
            {
                std::string msg = "_humidity_out_of_range_" + hum;
                data_oor_list.push_back(msg);
            }
            data_list.push_back(make_pair("humidity", hum));

            return SensorType::DATA_OK;
        }
        else if (crcc == SCDNOACKERROR)
        {
            return SensorType::DISCONNECT;
        }
        else
        {
            return SensorType::DATA_CRC_ERR;
        }
    }
    else
    {   
        return SensorType::DATA_NOT_RDY;
    }
}

/** Enables Sensor (Overrides SensorType virtual func)
 *
 */
void Scd30::Enable() 
{
    Scd30::StartMeasurement(0);
}

/** Disables Sensor (Overrides SensorType virtual func)
 *
 */
void Scd30::Disable() 
{
    Scd30::StopMeasurement();
}

/** Configures Sensor (Overrides SensorType virtual func)   // To be done in SENP-286
 * 
 */
// void Scd30::Configure(uint16_t interval_s=2, uint16_t baro=0, uint16_t temp_off=0, uint16_t alt_comp=0) 
// {
//     Scd30::StopMeasurement();
//     uint8_t set_baro = Scd30::StartMeasurement(baro);
//     uint8_t set_temp = Scd30::SetTemperatureOffs(temp_off);
//     uint8_t set_alt = Scd30::SetAltitudeComp(alt_comp);
//     uint8_t set_intv = Scd30::SetMeasInterval(interval_s);
// }

/** Resets Sensor (Overrides SensorType virtual func)
 *
 */
void Scd30::Reset() 
{
    Scd30::SoftReset();
}

/********************************************************************************/
