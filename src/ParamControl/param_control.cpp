/**
 * @defgroup param_control Parameter Control
 * @{
 */
 
#include "mbed.h"
#include "mbed-trace/mbed_trace.h"
#include "param_control.h"
#include "global_params.h"
#include "conversions.h"

#define TRACE_GROUP  "ParamControl"

/**
 *  @brief  Fan-out the control message from MQTT subscribe to respective thread
 *  @author Lau Lee Hong
 *  @param  param   parameter to be modified
 *  @param  value   value of parameter to be modified
 */
void DistributeControlMessage (std::string param, int value, std::string msg_id, std::string endpoint_id)
{
    if (param.find("sensor") != std::string::npos)
    {
        sensor_control_mail_t *sensor_control_mail = sensor_control_mail_box.calloc();
        while (sensor_control_mail == NULL)
        {
            sensor_control_mail = sensor_control_mail_box.calloc();
            tr_warn("Memory full. NULL pointer allocated");
            wait(0.5);
        }
        
        sensor_control_mail->param = StringToChar(param);
        sensor_control_mail->value = value;
        sensor_control_mail->msg_id = StringToChar(msg_id);
        sensor_control_mail->endpoint_id = StringToChar(endpoint_id);
        sensor_control_mail_box.put(sensor_control_mail);
        
        wait(0.1);
    }
}

/** @}*/