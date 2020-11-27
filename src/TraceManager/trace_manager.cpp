/**
 * @defgroup trace_manager Trace Manager
 * @{
 */

#include "trace_manager.h"
#include "mbed.h"
#include "mbed_trace.h"
#include "global_params.h"
#include "json.h"
#include "conversions.h"
#include "time_engine.h"
#include "trace_macro.h"

#define TRACE_GROUP "TraceManager"

/**
 *  @brief  Create and populate json; Used for trace messages in response to a control command with msg_id issued from DECADAcloud.
 *  @author Yap Zi Qi
 *  @param  msg             Trace message (predefined X-Macros in trace_macro.h)
 *  @param  msg_id          Message id from DECADAcloud control command 
 *  @return String of decada-compliant json packet
 */
std::string CreateDecadaResponse(std::string msg, std::string msg_id)
{
    Json::Value details;
    details[msg] = "true";
    
    Json::Value message_content;
    message_content["id"] = msg_id;
    message_content["code"] = 200;
    message_content["data"] = details;
    
    Json::FastWriter fast_writer;
    std::string decada_message = fast_writer.write(message_content);
    decada_message.erase(std::remove(decada_message.begin(), decada_message.end(), '\n'), decada_message.end());
    
    return decada_message; 
}

/**
 *  @brief  Sends service response, pegged with its identifier and id, to CommunicationsThread. Used for response message to a service request from DECADAcloud with msg_id issued from endpoint.
 *  @author Yap Zi Qi
 *  @param  service_id      DECADAcloud Service Identifier for service response topic
 *  @param  msg_id          Message Id of the service request from DECADAcloud
 *  @param  msg             Trace message (predefined X-Macros in trace_macro.h)
 * 
 *  Example:
 *  @code{.cpp}
 *  service_id = "service1"
 *  msg_id = "q1w2e3r4r5";
 *  DecadaServiceResponse(service_id, msg_id, trace_name[SERVICE1]);
 *  @endcode
 */
void DecadaServiceResponse(std::string service_id, std::string msg_id, std::string msg)
{
    std::string snon = CreateDecadaResponse(msg, msg_id);
    service_response_mail_t *service_response_mail = service_response_mail_box.calloc();
    while (service_response_mail == NULL)
    {
        service_response_mail = service_response_mail_box.calloc();
        tr_warn("Memory full. NULL pointer allocated");
        wait(0.5); 
    }
    
    service_response_mail->response = StringToChar(snon);
    service_response_mail->service_id = StringToChar(service_id);
    service_response_mail_box.put(service_response_mail);

    wait(0.1);
    return;
}

/** @}*/