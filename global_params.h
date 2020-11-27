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

#ifndef GLOBAL_PARAMS_H
#define GLOBAL_PARAMS_H

#include "mbed.h"
#include <string>
#include "MQTTClient.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"

/* Factory-set Device UUID */
extern const std::string device_uuid;

/* Global System State*/
extern std::string system_state;

/* RTOS Mutex*/ 
extern Mutex stdio_mutex;
extern Mutex mqtt_mutex;

/* Event flags */
extern EventFlags event_flags;
const uint32_t FLAG_MQTT_OK = (1U << 1);    // Signals MQTT is up

/* RTOS Mailboxes Declarations*/
typedef struct {
    char* sensor_type;
    char* value;
    int raw_time_stamp;
} llp_sensor_mail_t;
extern Mail<llp_sensor_mail_t, 256> llp_sensor_mail_box;    // Low-level platform (i/o-facing thread)

typedef struct {
    char* payload;
} comms_upstream_mail_t;
extern Mail<comms_upstream_mail_t, 256> comms_upstream_mail_box;

typedef struct {
    char* response;     // service response json
    char* service_id;   // service identifier
} service_response_mail_t;
extern Mail<service_response_mail_t, 256> service_response_mail_box;

typedef struct {
    char* endpoint_id;
    char* msg_id;
    char* param;
    char* value;
} mqtt_arrived_mail_t;
extern Mail<mqtt_arrived_mail_t, 128> mqtt_arrived_mail_box;

typedef struct {
    char* param;
    int value;
    char* msg_id;
    char* endpoint_id;
} sensor_control_mail_t;
extern Mail<sensor_control_mail_t, 64> sensor_control_mail_box;

/* For passing pointers to Subscription Manager Thread */
typedef struct{
    MQTT::Client<MQTTNetwork, Countdown> **mqtt_client_ptr;
    MQTTNetwork** mqtt_network_ptr;
    NetworkInterface* network;    
} mqtt_stack;

#endif  // GLOBAL_PARAMS_H