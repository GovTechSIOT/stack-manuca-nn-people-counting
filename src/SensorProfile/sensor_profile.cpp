/**
 * @defgroup sensor_profile Sensor Profile
 * @{
 */
#include "sensor_profile.h"
#include "mbed.h"
#include "mbed_trace.h"
#include "json.h"
#include "conversions.h"
#include "time_engine.h"
#include "global_params.h"

#define TRACE_GROUP "SensorProfile"

/**
 *  @brief  Public method that would return boolean of entity availability in entity_value_pairs_
 *  @author Yap Zi Qi
 *  @return bool of data availability in entity_value_pairs_
 */
bool SensorProfile::CheckEntityAvailability()
{
    if (entity_value_pairs_.size() > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 *  @brief  Update hashmap of entity with its value and timestamp.
 *  @author Yap Zi Qi
 *  @param  entity_name Name of data entity
 *  @param  value       New sensor value
 *  @param  time_stamp  Raw system timestamp of sensor value
 */
void SensorProfile::UpdateValue(std::string entity_name, std::string value, int time_stamp)
{
    entity_value_pairs_[entity_name] = make_pair(value, time_stamp); 
    return;
}

/**
 *  @brief  Public method that clear entity_value_pairs_.
 *  @author Lau Lee Hong
 */
void SensorProfile::ClearEntityList(void)
{
    entity_value_pairs_.clear();
}

/**
 *  @brief  Public method that updates entity_value_pairs_ by removing any entity that has not been updated
 *  @author Yap Zi Qi
 *  @param  time_stamp  Raw system timestamp of the start of data stream from sensor thread
 */
void SensorProfile::UpdateEntityList(int time_stamp)
{
    for (auto& it : entity_value_pairs_)
    {
        int entity_timestamp = it.second.second;
        if (entity_timestamp < time_stamp)
        {
            entity_value_pairs_.erase(it.first);
        }
    }
}

/**
 *  @brief  Public method that would return the most updated decada-compliant json packet.
 *  @author Lau Lee Hong
 *  @return std::string of decada-compliant json packet
 */
std::string SensorProfile::GetNewDecadaPacket(void)
{
    std::string decada_packet = CreateDecadaPacket();
    
    return decada_packet;
}

/**
 *  @brief  Create and populate json using DECADAcloud-compliant styling; Used for sensor messages.
 *  @author Lau Lee Hong, Yap Zi Qi
 *  @param  time_stamp   Raw system timestamp (in miliseconds) of the start of data stream from sensor thread
 *  @return String of DECADAcloud-compliant json packet for sensor messages
 */
std::string SensorProfile::CreateDecadaPacket(void)
{
    Json::Value measure_points;
    for (auto& it: entity_value_pairs_)
    {
        std::string entity_name = it.first;
        std::string entity_value = it.second.first;
        double value_float = StringToDouble(entity_value);
        measure_points[entity_name] = value_float;
    }
    
    Json::Value params;
    params["measurepoints"] = measure_points;
    
    Json::Value message_content;
    message_content["id"] = device_uuid;
    message_content["version"] = decada_protocol_version_;
    message_content["params"] = params;
    message_content["method"] = decada_method_of_device_;
    
    Json::FastWriter fast_writer;
    std::string decada_message = fast_writer.write(message_content);
    decada_message.erase(std::remove(decada_message.begin(), decada_message.end(), '\n'), decada_message.end());
    
    return decada_message;     
}

/** @}*/