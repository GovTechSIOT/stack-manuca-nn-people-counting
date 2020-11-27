/*******************************************************************************************************
 * Copyright (c) 2020 Government Technology Agency of Singapore (GovTech)
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
#ifndef DECADA_MANAGER_V2_H
#define DECADA_MANAGER_V2_H

#include <string>
#include <unordered_set>
#include "crypto_engine_v2.h"
#include "global_params.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"

/** DecadaManagerV2 class.
 *  @brief  MQTT instance that communicates with DECADA Cloud over TLS, and provisioned via dynamic activation.
 *
 *  Example:
 *  @code{.cpp}
 *  #include "mbed.h"
 *  #include "decada_manager_v2.h"
 *
 *  int main() 
 *  {
 *      std::string pub_topic = "/publish/1"; 
 *      std::string sub_topic = "/subscribe/2";
 *      std::string pub_msg = "{hello-manuca}";
 *
 *      NetworkInterface* network = NULL;
 *      bool network_connected= ConfigNetworkInterface(network);
 *
 *      if (network_connected)
 *      {
 *          DecadaManagerV2 decada(network);
 *          decada.Connect();
 *          decada.Subscribe(sub_topic.c_str());
 *          decada.Publish(pub_topic.c_str(), pub_msg);
 *      }
 *  }
 *  @endcode
 */

/* List of trusted Root CA Certificates
 * For DecadaManager: COMODO Root CA
 *
 * To add more root certificates, just concatenate them.
 */
const char SSL_CA_STORE_PEM[] =  
    "-----BEGIN CERTIFICATE-----\n"
    "MIIF2DCCA8CgAwIBAgIQTKr5yttjb+Af907YWwOGnTANBgkqhkiG9w0BAQwFADCB\n"
    "hTELMAkGA1UEBhMCR0IxGzAZBgNVBAgTEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4G\n"
    "A1UEBxMHU2FsZm9yZDEaMBgGA1UEChMRQ09NT0RPIENBIExpbWl0ZWQxKzApBgNV\n"
    "BAMTIkNPTU9ETyBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTAwMTE5\n"
    "MDAwMDAwWhcNMzgwMTE4MjM1OTU5WjCBhTELMAkGA1UEBhMCR0IxGzAZBgNVBAgT\n"
    "EkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UEBxMHU2FsZm9yZDEaMBgGA1UEChMR\n"
    "Q09NT0RPIENBIExpbWl0ZWQxKzApBgNVBAMTIkNPTU9ETyBSU0EgQ2VydGlmaWNh\n"
    "dGlvbiBBdXRob3JpdHkwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQCR\n"
    "6FSS0gpWsawNJN3Fz0RndJkrN6N9I3AAcbxT38T6KhKPS38QVr2fcHK3YX/JSw8X\n"
    "pz3jsARh7v8Rl8f0hj4K+j5c+ZPmNHrZFGvnnLOFoIJ6dq9xkNfs/Q36nGz637CC\n"
    "9BR++b7Epi9Pf5l/tfxnQ3K9DADWietrLNPtj5gcFKt+5eNu/Nio5JIk2kNrYrhV\n"
    "/erBvGy2i/MOjZrkm2xpmfh4SDBF1a3hDTxFYPwyllEnvGfDyi62a+pGx8cgoLEf\n"
    "Zd5ICLqkTqnyg0Y3hOvozIFIQ2dOciqbXL1MGyiKXCJ7tKuY2e7gUYPDCUZObT6Z\n"
    "+pUX2nwzV0E8jVHtC7ZcryxjGt9XyD+86V3Em69FmeKjWiS0uqlWPc9vqv9JWL7w\n"
    "qP/0uK3pN/u6uPQLOvnoQ0IeidiEyxPx2bvhiWC4jChWrBQdnArncevPDt09qZah\n"
    "SL0896+1DSJMwBGB7FY79tOi4lu3sgQiUpWAk2nojkxl8ZEDLXB0AuqLZxUpaVIC\n"
    "u9ffUGpVRr+goyhhf3DQw6KqLCGqR84onAZFdr+CGCe01a60y1Dma/RMhnEw6abf\n"
    "Fobg2P9A3fvQQoh/ozM6LlweQRGBY84YcWsr7KaKtzFcOmpH4MN5WdYgGq/yapiq\n"
    "crxXStJLnbsQ/LBMQeXtHT1eKJ2czL+zUdqnR+WEUwIDAQABo0IwQDAdBgNVHQ4E\n"
    "FgQUu69+Aj36pvE8hI6t7jiY7NkyMtQwDgYDVR0PAQH/BAQDAgEGMA8GA1UdEwEB\n"
    "/wQFMAMBAf8wDQYJKoZIhvcNAQEMBQADggIBAArx1UaEt65Ru2yyTUEUAJNMnMvl\n"
    "wFTPoCWOAvn9sKIN9SCYPBMtrFaisNZ+EZLpLrqeLppysb0ZRGxhNaKatBYSaVqM\n"
    "4dc+pBroLwP0rmEdEBsqpIt6xf4FpuHA1sj+nq6PK7o9mfjYcwlYRm6mnPTXJ9OV\n"
    "2jeDchzTc+CiR5kDOF3VSXkAKRzH7JsgHAckaVd4sjn8OoSgtZx8jb8uk2Intzna\n"
    "FxiuvTwJaP+EmzzV1gsD41eeFPfR60/IvYcjt7ZJQ3mFXLrrkguhxuhoqEwWsRqZ\n"
    "CuhTLJK7oQkYdQxlqHvLI7cawiiFwxv/0Cti76R7CZGYZ4wUAc1oBmpjIXUDgIiK\n"
    "boHGhfKppC3n9KUkEEeDys30jXlYsQab5xoq2Z0B15R97QNKyvDb6KkBPvVWmcke\n"
    "jkk9u+UJueBPSZI9FoJAzMxZxuY67RIuaTxslbH9qh17f4a+Hg4yRvv7E491f0yL\n"
    "S0Zj/gA0QHDBw7mh3aZw4gSzQbzpgJHqZJx64SIDqZxubw5lT2yHh17zbqD5daWb\n"
    "QOhTsiedSrnAdyGN/4fy3ryM7xfft0kL0fJuMAsaDk527RH89elWsn2/x20Kk4yl\n"
    "0MC2Hb46TpSi125sC8KKfPog88Tk5c0NqMuRkrF8hey1FGlmDoLnzc7ILaZRfyHB\n"
    "NVOFBkpdn627G190\n"
    "-----END CERTIFICATE-----\n";

class DecadaManagerV2 : protected CryptoEngineV2
{
    public:
        DecadaManagerV2(NetworkInterface*& net)
        {
            network_ = net;
        };
        
        /* Publish & Subscribe */
        bool Connect(void);
        bool Publish(const char* topic, std::string payload);
        bool Subscribe(const char* topic);
        bool Reconnect(void);
        bool RenewCertificate(void);
        mqtt_stack* GetMqttStackPointer(void);

    private:
        /* DECADA Provisioning */
        std::string GetDecadaRootCertificateAuthority(void);
        std::string GetAccessToken(void);
        std::string CheckDeviceCreation(void);
        std::string CreateDeviceInDecada(std::string default_name);
        std::pair<std::string, std::string> GetClientCertificate(void);
        std::pair<std::string, std::string> RenewClientCertificate(void);
        
        /* Network Connection */
        bool ConnectMqttNetwork(std::string root_ca, std::string client_cert, std::string private_key);
        bool ConnectMqttClient(void);

        /* Network Disconnection */
        void DisconnectMqttNetwork(void);
        void DisconnectMqttClient(void);

        /* Reconnection */
        bool ReconnectMqttNetwork(std::string root_ca, std::string client_cert, std::string private_key);
        bool ReconnectMqttClient(void);
        bool ReconnectMqttService(std::string root_ca, std::string client_cert, std::string private_key);

        const std::string decada_product_key_ = MBED_CONF_APP_DECADA_PRODUCT_KEY;
        const std::string decada_access_key_ = MBED_CONF_APP_DECADA_ACCESS_KEY;
        const std::string decada_access_secret_ = MBED_CONF_APP_DECADA_ACCESS_SECRET;
        const std::string decada_ou_id_ = MBED_CONF_APP_DECADA_OU_ID;
        const std::string api_url_ = MBED_CONF_APP_DECADA_API_URL;
        const std::string broker_ip_ = "mqtt.decada.gov.sg";      
        const int mqtt_server_port_ = 18885;
        std::string device_secret_;

        NetworkInterface* network_ = NULL;
        MQTTNetwork* mqtt_network_ = NULL;
        MQTTNetwork** mqtt_network_ptr_ = &mqtt_network_;
        MQTT::Client<MQTTNetwork, Countdown>* mqtt_client_ = NULL;
        MQTT::Client<MQTTNetwork, Countdown>** mqtt_client_ptr_  = &mqtt_client_;
        mqtt_stack stack_;

        std::unordered_set<std::string> sub_topics_;
};

 #endif  // DECADA_MANAGER_V2_H