#include "mbed.h"
#include "sensor_type.h"
#include "conversions.h"

/**
 *  @brief  Convert float data value into string format.
 *  @author Yap Zi Qi
 *  @param  data    data in float type
 *  @return Data value in string format
 */
std::string SensorType::ConvertDataToString(float data)
{
	char buf[32];
	std::string data_str = DoubleToChar(buf, data, 2);
	return data_str;
}

/**
 *  @brief  Validate if data is within valid range.
 *  @author Yap Zi Qi
 *  @param  data        data in float type
 *  @param  data_min    minimum valid data value
 *  @param  data_max    maximum valid data value
 *  @return enum SensorStatus
 */
int SensorType::ValidateData(float data, float data_min, float data_max)
{
	if (data > data_max | data < data_min) 
        {
            return DATA_OUT_OF_RANGE;
        }
    else return DATA_OK;
}