/**
 * @defgroup event_manager_thread Event Manager Thread
 * @{
 */
 
#include <string>
#include <queue>
#include "threads.h"
#include "mbed_trace.h"
#include "rtos.h"
#include "global_params.h"
#include "conversions.h"
#include "param_control.h"

 /* [rtos: thread_4] EventManagerThread */
void event_manager_thread(void)
{
    #undef TRACE_GROUP
    #define TRACE_GROUP  "EventManagerThread"
   
    const uint32_t evtmgr_sleep_ms = 5000;
    Watchdog &watchdog = Watchdog::get_instance();

    while (1)
    {
        // Wait for MQTT connection to be up before continuing
        event_flags.wait_all(FLAG_MQTT_OK, osWaitForever, false);
        
        osEvent evt = mqtt_arrived_mail_box.get(1); 
        if (evt.status == osEventMail) 
        {
            mqtt_arrived_mail_t *mqtt_arrived_mail = (mqtt_arrived_mail_t*) evt.value.p;
            std::string endpoint_id = mqtt_arrived_mail->endpoint_id;
            free(mqtt_arrived_mail->endpoint_id);
            std::string msg_id = mqtt_arrived_mail->msg_id;
            free(mqtt_arrived_mail->msg_id);
            std::string param = mqtt_arrived_mail->param;
            free(mqtt_arrived_mail->param);
            std::string value = mqtt_arrived_mail->value;

            int int_value = StringToInt(value);
            DistributeControlMessage(param, int_value, msg_id, endpoint_id);

            mqtt_arrived_mail_box.free(mqtt_arrived_mail);
        }

        watchdog.kick();

        ThisThread::sleep_for(evtmgr_sleep_ms);
    }
}
 
 /** @}*/