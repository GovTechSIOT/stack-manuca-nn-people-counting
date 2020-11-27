/**
 * @defgroup sensor_thread Sensor Thread
 * @{
 */

#include <string>
#include "threads.h"
#include "mbed_trace.h"
#include "rtos.h"
#include "global_params.h"
#include "conversions.h"
#include "persist_store.h"
#include "time_engine.h"
#include "trace_macro.h"
#include "trace_manager.h"
#include "tmp75.h"
#include "Ardu_Camera.h"

#define TMP75_ADDR      0x4B

void execute_sensor_control(int& current_cycle_interval)
{
    #undef TRACE_GROUP
    #define TRACE_GROUP "SensorThread"

    /* Timeout set to 1 ms */ 
    osEvent evt = sensor_control_mail_box.get(1);
    if (evt.status == osEventMail)
    {
        sensor_control_mail_t *sensor_control_mail = (sensor_control_mail_t *)evt.value.p;
        std::string param = sensor_control_mail->param;
        free(sensor_control_mail->param);
        int value = sensor_control_mail->value;
        std::string msg_id = sensor_control_mail->msg_id;
        free(sensor_control_mail->msg_id);
        std::string endpoint_id = sensor_control_mail->endpoint_id;
        free(sensor_control_mail->endpoint_id);

        if ((param == "sensor_poll_rate") && (value >= 10))      // Lowest bound - 10 seconds
        {
            tr_info("Sensor poll rate changed to %d", value);
            DecadaServiceResponse(endpoint_id, msg_id, trace_name[POLL_RATE_UPDATE]);
            sensor_control_mail_box.free(sensor_control_mail);
            WriteCycleInterval(to_string(value*1000));          // Convert to miliseconds and save to persistence 
            current_cycle_interval = value*1000;
        }
        sensor_control_mail_box.free(sensor_control_mail);
    }

    return;
}

 /* [rtos: thread_3] SensorThread */
void sensor_thread(void)
{   
    #undef TRACE_GROUP
    #define TRACE_GROUP  "SensorThread"

    const int sensor_thread_sleep_ms = 1000;
    
    const PinName cam_cs_pin = PinName::PB_12;      //SPI2_CS
    const PinName cam_spi_mosi_pin = PinName::PC_3; //SPI2_MOSI
    const PinName cam_spi_miso_pin = PinName::PC_2; //SPI2_MISO
    const PinName cam_spi_sclk_pin = PinName::PB_10; //SPI2_SCK
    const PinName cam_i2c_data_pin = PinName::PF_0; //I2C2_DATA
    const PinName cam_i2c_sclk_pin = PinName::PF_1;  //I2C2_SCLK

    tr_debug("Initializing watchdog");
    Watchdog &watchdog = Watchdog::get_instance();

    int current_cycle_interval = StringToInt(ReadCycleInterval());
    int current_poll_count = current_cycle_interval / sensor_thread_sleep_ms;
    int poll_counter = 0;

    tr_debug("Initializing camera class");
    // Statically allocate memory to avoid runtime memory allocation issues
    static Ardu_Camera arducam(cam_cs_pin, 
                cam_spi_mosi_pin, cam_spi_miso_pin, cam_spi_sclk_pin,
                cam_i2c_data_pin, cam_i2c_sclk_pin);
    tr_debug("Camera class initialized successfully");
    arducam.Enable();

    while (1) 
    {
        tr_debug("Executing sensor poll loop");
        tr_debug("Kicking watchdog...");
        watchdog.kick();

       /* Wait for MQTT connection to be up before continuing */
        event_flags.wait_all(FLAG_MQTT_OK, osWaitForever, false);

        current_poll_count = current_cycle_interval / sensor_thread_sleep_ms;
        tr_debug("Poll_counter: %d", poll_counter);
        if (poll_counter == 0)
        {
            tr_debug("Adding header to sensor data stream");
            /* Start of sensor data stream - Add header */
            llp_sensor_mail_t * llp_mail = llp_sensor_mail_box.calloc();
            while (llp_mail == NULL)
            {
                llp_mail = llp_sensor_mail_box.calloc();
                tr_warn("Memory full. NULL pointer allocated");
                ThisThread::sleep_for(500);
            }
            llp_mail->sensor_type = StringToChar("header");
            llp_mail->value = StringToChar("start");
            llp_mail->raw_time_stamp = RawRtcTimeNow();
            llp_sensor_mail_box.put(llp_mail);

            std::vector<std::pair<std::string, std::string>> s_data;

            /* Poll other sensors here */
            tr_info("Polling camera sensor");
            int cam_stat = arducam.GetData(s_data);
            if (cam_stat == SensorType::DATA_NOT_RDY || cam_stat == SensorType::DATA_CRC_ERR)
            {
                tr_warn("Camera data error");
            }
            if (cam_stat == SensorType::DATA_OK)
            {   

                llp_mail = llp_sensor_mail_box.calloc();
                while (llp_mail == NULL)
                {
                    llp_mail = llp_sensor_mail_box.calloc();
                    tr_warn("Memory full. NULL pointer allocated");
                    ThisThread::sleep_for(500);
                }

                llp_mail->sensor_type = StringToChar(s_data[0].first);
                llp_mail->value = StringToChar(s_data[0].second);
                llp_mail->raw_time_stamp = RawRtcTimeNow();
                llp_sensor_mail_box.put(llp_mail);
            }



            /* End of sensor data stream  - Add footer */
            tr_debug("Adding footer to sensor data stream");
            llp_mail = llp_sensor_mail_box.calloc();
            while (llp_mail == NULL)
            {
                llp_mail = llp_sensor_mail_box.calloc();
                tr_warn("Memory full. NULL pointer allocated");
                ThisThread::sleep_for(500);
            }
            llp_mail->sensor_type = StringToChar("header");
            llp_mail->value = StringToChar("end");
            llp_mail->raw_time_stamp = RawRtcTimeNow();
            llp_sensor_mail_box.put(llp_mail);
        }
        poll_counter++;

        if (poll_counter >= current_poll_count)
        {
            poll_counter = 0;
        }

        execute_sensor_control(current_cycle_interval);
        
        watchdog.kick();

        ThisThread::sleep_for(sensor_thread_sleep_ms);
    }
}
 
 /** @}*/
