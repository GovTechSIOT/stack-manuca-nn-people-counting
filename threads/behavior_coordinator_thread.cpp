/**
 * @defgroup behavior_coordinator_thread Behavior Coordinator Thread
 * @{
 */

#include <string>
#include "threads.h"
#include "mbed_trace.h"
#include "rtos.h"
#include "mbed.h"
#include "global_params.h"
#include "conversions.h"
#include "persist_store.h"
#include "sensor_profile.h"
#include "time_engine.h"

/* [rtos: thread_2] BehaviorCoordinatorThread */
void behavior_coordinator_thread(void) 
{
    #undef TRACE_GROUP
    #define TRACE_GROUP  "BehaviorCoordinatorThread"

    const uint32_t behav_thread_sleep_ms = 500;
    Watchdog &watchdog = Watchdog::get_instance();

    SensorProfile sensors_profile;

    bool send_packets = false;
    
    while (1) 
    {   
        /* Wait for MQTT connection to be up before continuing */
        event_flags.wait_all(FLAG_MQTT_OK, osWaitForever, false);
        
        osEvent evt = llp_sensor_mail_box.get(1);
        if (evt.status == osEventMail) 
        {
            llp_sensor_mail_t *llp_mail = (llp_sensor_mail_t*) evt.value.p;
            std::string entity = llp_mail->sensor_type;
            free(llp_mail->sensor_type);
            std::string new_value = llp_mail->value;
            free(llp_mail->value);
            int new_time_stamp = llp_mail->raw_time_stamp;

            if (entity == "header")
            {
                if (new_value == "start")   // start of data stream from sensor thread
                {
                    sensors_profile.ClearEntityList();
                }
                else                        // end of data stream from sensor thread
                {
                    send_packets = true;
                }
            }
            else
            {
                sensors_profile.UpdateValue(entity, new_value, new_time_stamp);
            }

            llp_sensor_mail_box.free(llp_mail);
        }
        
        /* 
         *  Add analytics algorithms here. You can extract measure point profile data, and manipulate them before sending upstream. 
         *  Examples are Naive Bayes, Support Vector Machine (SVM) and Neural Networks (using CMSIS-NN).
         */

        if (send_packets)
        {
            stdio_mutex.lock();
            comms_upstream_mail_t *comms_upstream_mail = comms_upstream_mail_box.calloc();
            while (comms_upstream_mail == NULL)
            {
                comms_upstream_mail = comms_upstream_mail_box.calloc();
                tr_warn("Memory full. NULL pointer allocated");
                ThisThread::sleep_for(500);
            }
            comms_upstream_mail->payload = StringToChar(sensors_profile.GetNewDecadaPacket());
            comms_upstream_mail_box.put(comms_upstream_mail);
            stdio_mutex.unlock();
            send_packets = false;
        }

        watchdog.kick();

        ThisThread::sleep_for(behav_thread_sleep_ms);
    }
}

 /** @}*/