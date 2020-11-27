/**
 * @defgroup decada_manager_v2 DECADA Manager V2
 * @{
 */

#include "decada_manager_v2.h"
#include "mbedtls/sha256.h"
#include "mbedtls/sha1.h"
#include "mbed_trace.h"
#include "https_request.h"
#include "json.h"
#include "conversions.h"
#include "device_uid.h"
#include "persist_store.h"
#include "subscription_callback.h"
#include "time_engine.h"

#undef TRACE_GROUP
#define TRACE_GROUP  "DecadaManagerV2"

/**
 *  @brief      Setup network connection to DECADA cloud.
 *  @details    This method will attempt to do dynamic provisioning on-the-fly with DECADA if device is not already provisioned.
 *  @author     Lau Lee Hong
 *  @return     Successful(1)/unsuccessful(0) in establishing conenection to DECADA cloud
 */
bool DecadaManagerV2::Connect(void)
{
    bool success = true;

    /* Initialize random number generator with RTC as seed */
    srand(time(NULL));
    
    const std::string decada_root_ca = GetDecadaRootCertificateAuthority();

    /* Create this device in DECADA Cloud as a device entity */
    device_secret_ = CheckDeviceCreation();
    bool first_registration_attempt = true;
    while (device_secret_ == "invalid")
    {
        if(!first_registration_attempt)
        {
            ThisThread::sleep_for(500);
        }
        tr_info("Creating device in DECADA...");
        device_secret_ = CreateDeviceInDecada("core-" + device_uuid);

        first_registration_attempt = false;
    }
    tr_info("Device is created in DECADA.");

    /* Request for SSL Client Certificate if device was not already provisioned */
    std::string client_cert = ReadClientCertificate();
    if (client_cert == "" || client_cert == "invalid" || ReadSSLPrivateKey() == "")
    {
        tr_info("Requesting client certificate from DECADA...");
        const std::pair<std::string, std::string> client_cert_data = GetClientCertificate();
        client_cert = client_cert_data.first;
        const std::string client_cert_serial_number = client_cert_data.second;
        WriteClientCertificate(client_cert);
        WriteClientCertificateSerialNumber(client_cert_serial_number);
        tr_info("New client certificate is generated.");
    }
    else
    {
        tr_info("Using existing client certificate.");
    }

    /* Establish MQTT Connection */
    success = success && ConnectMqttNetwork(decada_root_ca, client_cert, ReadSSLPrivateKey());
    success = success && ConnectMqttClient();

    return success;
}

/**
 *  @brief  Publish payload via MQTT.
 *  @author Lau Lee Hong
 *  @param  topic       MQTT publish topic
 *  @param  payload     Outgoing MQTT message
 *  @return Successful(1)/unsuccessful(0) mqtt publish
 */
bool DecadaManagerV2::Publish(const char* topic, std::string payload)
{   
    stdio_mutex.lock();

    char* mqtt_payload = (char*) payload.c_str();

    /* Publish MQTT message */
    MQTT::Message message;
    message.retained = false;
    message.dup = false;
    message.payload = mqtt_payload;
    message.qos = MQTT::QOS0;
    message.payloadlen = (payload.length());
        
    int rc = mqtt_client_->publish(topic, message);

    stdio_mutex.unlock();
    
    if (rc != MQTT::SUCCESS)
    {
        /* Mosquitto broker is disconnected */ 
        tr_warn("rc from MQTT publish is %d", rc);
        return false;
    }
    else
    {
        tr_debug("MQTT Message published");
    }
    
    return true;
}

/**
 *  @brief  Subscribe to the MQTT topic.
 *  @author Lau Lee Hong
 *  @param  topic   MQTT topic
 *  @return Successful(1)/unsuccessful(0) mqtt subscription service
 */
bool DecadaManagerV2::Subscribe(const char* topic)
{
    sub_topics_.insert(topic);

    int rc = mqtt_client_->subscribe(topic, MQTT::QOS1, SubscriptionMessageArrivalCallback);
    
    if (rc != MQTT::SUCCESS) 
    {
        tr_err("rc from MQTT subscribe is %d", rc);
        
        return false;
    }
    else
    {
        tr_info("MQTT subscription service online");
    }
        
    return true;  
}

/**
 *  @brief  Attempt to re-establish connection to DECADA. 
 *  @author Lau Lee Hong
 *  @return Successful(1)/unsuccessful(0)
 */
bool DecadaManagerV2::Reconnect(void)
{
    bool success = true;
    success = success && ReconnectMqttService(GetDecadaRootCertificateAuthority(), ReadClientCertificate(), ReadSSLPrivateKey());

    return success;  
}

/**
 *  @brief      Renew DECADA Client Certificate.
 *  @details    This method will invoke a software reset; New bootup will connect to DECADA using the renewed certificate.
 *  @author     Lau Lee Hong
 *  @return     true: success, false: failed
 */
bool DecadaManagerV2::RenewCertificate(void)
{
    tr_debug("Renewing SSL Client Certificate");

    const std::pair<std::string, std::string> client_cert_data = RenewClientCertificate();
    const std::string client_cert = client_cert_data.first;
    const std::string client_cert_serial_number = client_cert_data.second;

    if ((client_cert != "invalid") && (client_cert_serial_number != "invalid"))
    {
        WriteClientCertificate(client_cert_data.first);
        WriteClientCertificateSerialNumber(client_cert_data.second);
        
        tr_warn("Client Certificate has been renewed; System will reset.");
        NVIC_SystemReset();

        return true;
    }
    else
    {
        return false;
    }
}

/**
 *  @brief  Returns mqtt stack pointers, to pass to another thread.
 *  @author Lau Lee Hong
 *  @return mqtt connectivity pointers
 */
mqtt_stack* DecadaManagerV2::GetMqttStackPointer(void)
{
    mqtt_stack* stack_ptr = &stack_;

    stack_ptr->mqtt_client_ptr = mqtt_client_ptr_;
    stack_ptr->mqtt_network_ptr = mqtt_network_ptr_;
    stack_ptr->network = network_;
    
    return stack_ptr;
}

/**
 *  @brief  Gets DECADA SSL Root CA for MQTT Connection.
 *  @author Lau Lee Hong
 *  @return PEM-formatted SSL Root CA - COMODO Cert.
 */
std::string DecadaManagerV2::GetDecadaRootCertificateAuthority(void)
{
    return SSL_CA_STORE_PEM;
}

/**
 *  @brief      Returns an access token for subsequent REST calls usage using appKey and appSecret.
 *  @details    The token expires 2 hours after creation, thus it is not recommended to be stored/cached in software.
 *  @author     Lau Lee Hong
 *  @return     DECADA REST API access token
 */
std::string DecadaManagerV2::GetAccessToken(void)
{   
    const std::string timestamp_ms = MsPaddingIntToString(RawRtcTimeNow());
    const std::string signing_params = decada_access_key_ + timestamp_ms + decada_access_secret_;
    const std::string signature = ToLowerCase(GenericSHA256Generator(signing_params));

    Json::Value message_content;
    message_content["appKey"] = decada_access_key_;
    message_content["encryption"] = signature;
    message_content["timestamp"] = timestamp_ms;
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    std::string body = Json::writeString(builder, message_content);
    const char* body_sanitized = (char*)body.c_str();

    const std::string request_uri = "/apim-token-service/v2.0/token/get";
    HttpsRequest* request = new HttpsRequest(network_, SSL_CA_STORE_PEM, HTTP_POST, (api_url_ + request_uri).c_str());
    request->set_header("Content-Type", "application/json;charset=UTF-8");
 
    HttpResponse* response = request->send(body_sanitized, strlen(body_sanitized));
    if (!response) 
    {
        delete request;
        tr_warn("GetAccessToken failed (status code %d)", response->get_status_code());
        return "invalid";  
    }
    else
    {
        std::string res_string = response->get_body_as_string();
        Json::Reader reader;
        Json::Value root;
        Json::Value sub_root;
        reader.parse(res_string, root, false);
        sub_root = root.get("data", "invalid");
        std::string access_token = sub_root.get("accessToken", "invalid").asString();
        delete request;

        return access_token;
    }
}

/**
 *  @brief  RESTful call to check if this device instance has already been created in DECADA prior.
 *  @author Lau Lee Hong
 *  @return C++ string containing response code from the server
 */
std::string DecadaManagerV2::CheckDeviceCreation(void)
{
    const std::string timestamp_ms = MsPaddingIntToString(RawRtcTimeNow());
    const std::string access_token = GetAccessToken();
    const std::string http_get_frame = "actionget";

    /* Sort parameters in ASCII order */ 
    const std::string parameters = http_get_frame + "deviceKey" + GetDeviceUid() + "orgId" + decada_ou_id_ + "productKey" + decada_product_key_;
    const std::string signing_params = access_token + parameters + timestamp_ms + decada_access_secret_;
    const std::string signature = ToLowerCase(GenericSHA256Generator(signing_params));

    const std::string request_uri = "/connect-service/v2.1/devices?action=get&orgId=" + decada_ou_id_
                                    + "&productKey=" + decada_product_key_
                                    + "&deviceKey=" + GetDeviceUid();

    HttpsRequest* request = new HttpsRequest(network_, SSL_CA_STORE_PEM, HTTP_GET, (api_url_ + request_uri).c_str());
    request->set_header("apim-accesstoken", access_token);
    request->set_header("apim-signature", signature);
    request->set_header("apim-timestamp", timestamp_ms);

    HttpResponse* response = request->send();
    if (!response) 
    {
        delete request;
        tr_warn("CheckDeviceCreation failed (status code %d)", response->get_status_code());
        return "invalid"; 
    }
    else
    {
        std::string res_string = response->get_body_as_string();
        tr_debug("check creation: %s", res_string.c_str());
        Json::Reader reader;
        Json::Value root;
        Json::Value sub_root;
        reader.parse(res_string, root, false);
        sub_root = root.get("data", "invalid");
        std::string device_secret = sub_root.get("deviceSecret", "invalid").asString();

        delete request;
        return device_secret;
    }
 }

/**
 *  @brief  RESTful call to create "register" this device under the product.
 *  @author Lau Lee Hong
 *  @param  default_name  User defined name for their device
 *  @return C++ string containing of random generated characters
 */
std::string DecadaManagerV2::CreateDeviceInDecada(std::string default_name)
{   
    const std::string timestamp_ms = MsPaddingIntToString(RawRtcTimeNow());
    const std::string access_token = GetAccessToken();
    const std::string http_post_frame = "actioncreate";

    Json::Value international_name;

    Json::Value device_name;
    device_name["defaultValue"] = default_name;
    device_name["i18nValue"] = international_name;

    Json::Value message_content;
    message_content["productKey"] = decada_product_key_;
    message_content["timezone"] = "+08:00";
    message_content["deviceName"] = device_name;
    message_content["deviceKey"] = GetDeviceUid();
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    std::string body = Json::writeString(builder, message_content);
    const char* body_sanitized = (char*)body.c_str();

    /* Sort parameters in ASCII order */
    const std::string parameters = http_post_frame + "orgId" + decada_ou_id_ + body_sanitized;
    const std::string signing_params = access_token + parameters + timestamp_ms + decada_access_secret_;
    const std::string signature = ToLowerCase(GenericSHA256Generator(signing_params));

    const std::string request_uri = "/connect-service/v2.1/devices?action=create&orgId=" + decada_ou_id_;
    HttpsRequest* request = new HttpsRequest(network_, SSL_CA_STORE_PEM, HTTP_POST, (api_url_ + request_uri).c_str());
    request->set_header("Content-Type", "application/json;charset=UTF-8");
    request->set_header("apim-accesstoken", access_token);
    request->set_header("apim-signature", signature);
    request->set_header("apim-timestamp", timestamp_ms);
 
    HttpResponse* response = request->send(body_sanitized, strlen(body_sanitized));
    if (!response) 
    {
        delete request;
        tr_warn("CreateDeviceInDecada request failed (status code %d)", response->get_status_code());
        return "invalid";  
    }
    else
    {
        std::string res_string = response->get_body_as_string();
        tr_debug("create device: %s", res_string.c_str());
        Json::Reader reader;
        Json::Value root;
        Json::Value sub_root;
        reader.parse(res_string, root, false);
        sub_root = root.get("data", "invalid");
        std::string device_secret = sub_root.get("deviceSecret", "invalid").asString();

        delete request;
        return device_secret;
    }
}

/**
 *  @brief      RESTful call to do x509 exchange to attain SSL client certificate.
 *  @details    The client certificate is a key parameter required for establishing a MQTT connection with decada.
 *  @author     Lau Lee Hong
 *  @return     <ssl client certificate, ssl client certificate serial number>
 */
std::pair<std::string, std::string> DecadaManagerV2::GetClientCertificate(void)
{   
    tr_debug("GetClientCertificate called...");
    const std::string timestamp_ms = MsPaddingIntToString(RawRtcTimeNow());
    const std::string access_token = GetAccessToken();
    const std::string http_post_frame = "actionapply";
    
    Watchdog &watchdog = Watchdog::get_instance();
    watchdog.kick();

    const std::string ssl_csr = GenerateCertificateSigningRequest(timestamp_ms);

    Json::Value message_content;
    message_content["csr"] = ssl_csr;
    message_content["validDay"] = 365;
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    std::string body = Json::writeString(builder, message_content);
    const char* body_sanitized = (char*)body.c_str();

    /* Sort parameters in ASCII order */
    const std::string parameters = http_post_frame + "deviceKey" + GetDeviceUid() + "orgId" + decada_ou_id_ + "productKey" + decada_product_key_+ body_sanitized;
    const std::string signing_params = access_token + parameters + timestamp_ms + decada_access_secret_;
    const std::string signature = ToLowerCase(GenericSHA256Generator(signing_params));

    const std::string request_uri = "/connect-service/v2.0/certificates?action=apply&orgId=" + decada_ou_id_
                                    + "&productKey=" + decada_product_key_
                                    + "&deviceKey=" + GetDeviceUid();
    HttpsRequest* request = new HttpsRequest(network_, SSL_CA_STORE_PEM, HTTP_POST, (api_url_ + request_uri).c_str());
    request->set_header("Content-Type", "application/json;charset=UTF-8");
    request->set_header("apim-accesstoken", access_token);
    request->set_header("apim-signature", signature);
    request->set_header("apim-timestamp", timestamp_ms);

    HttpResponse* response = request->send(body_sanitized, strlen(body_sanitized));
    if (!response) 
    {
        delete request;
        tr_warn("GetClientCertificate request failed (status code %d)", response->get_status_code());
        return std::make_pair("invalid", "invalid");  
    }
    else
    {
        std::string res_string = response->get_body_as_string();
        tr_debug("get client cert: %s", res_string.c_str());
        Json::Reader reader;
        Json::Value root;
        Json::Value sub_root;
        tr_debug("Parsing response string");
        reader.parse(res_string, root, false);
        tr_debug("Getting root data");
        sub_root = root.get("data", "invalid");
        tr_debug("Getting decada cert");
        std::string decada_cert = sub_root.get("cert", "invalid").asString();
        tr_debug("Getting decada cert serial number");
        std::string decada_cert_serial_number = sub_root.get("certSN", "invalid").asString();

        delete request;
        tr_debug("GetClientCertificate resolved successfully");
        return std::make_pair(decada_cert, decada_cert_serial_number);
    }
    
}

/**
 *  @brief      RESTful call to renew the existing SSL client certificate with a new certificate.
 *  @details    The client certificate is a key parameter required for establishing a MQTT connection with decada.
 *  @author     Lau Lee Hong
 *  @return     Refreshed <ssl client certificate, ssl client certificate serial number>
 */
std::pair<std::string, std::string> DecadaManagerV2::RenewClientCertificate(void)
{  
    const std::string timestamp_ms = MsPaddingIntToString(RawRtcTimeNow());
    const std::string access_token = GetAccessToken();
    const std::string http_post_frame = "actionrenew";
    
    Watchdog &watchdog = Watchdog::get_instance();
    watchdog.kick();

    Json::Value message_content;
    message_content["certSn"] = StringToInt(ReadClientCertificateSerialNumber());
    message_content["validDay"] = 365;
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    std::string body = Json::writeString(builder, message_content);
    const char* body_sanitized = (char*)body.c_str();

    /* Sort parameters in ASCII order */
    const std::string parameters = http_post_frame + "deviceKey" + GetDeviceUid() + "orgId" + decada_ou_id_ + "productKey" + decada_product_key_+ body_sanitized;
    const std::string signing_params = access_token + parameters + timestamp_ms + decada_access_secret_;
    const std::string signature = ToLowerCase(GenericSHA256Generator(signing_params));

    const std::string request_uri = "/connect-service/v2.0/certificates?action=renew&orgId=" + decada_ou_id_
                                    + "&productKey=" + decada_product_key_
                                    + "&deviceKey=" + GetDeviceUid();
    HttpsRequest* request = new HttpsRequest(network_, SSL_CA_STORE_PEM, HTTP_POST, (api_url_ + request_uri).c_str());
    request->set_header("Content-Type", "application/json;charset=UTF-8");
    request->set_header("apim-accesstoken", access_token);
    request->set_header("apim-signature", signature);
    request->set_header("apim-timestamp", timestamp_ms);

    HttpResponse* response = request->send(body_sanitized, strlen(body_sanitized));
    if (!response) 
    {
        delete request;
        tr_warn("GetClientCertificate request failed (status code %d)", response->get_status_code());
        return std::make_pair("invalid", "invalid");  
    }
    else
    {
        std::string res_string = response->get_body_as_string();
        tr_debug("renew client cert: %s", res_string.c_str());
        Json::Reader reader;
        Json::Value root;
        Json::Value sub_root;
        reader.parse(res_string, root, false);
        sub_root = root.get("data", "invalid");
        std::string renewed_decada_cert = sub_root.get("cert", "invalid").asString();
        std::string renewed_decada_cert_serial_number = sub_root.get("certSN", "invalid").asString();

        delete request;
        return std::make_pair(renewed_decada_cert, renewed_decada_cert_serial_number);
    }
}

/**
 *  @brief  Connect to MQTT network (refer to MQTT_server_setting.h to set a different hostname/serverport).
 *  @author Lau Lee Hong, Goh Kok Boon
 *  @param root_ca      Root CA
 *  @param client_cert  SSL Client Certificate 
 *  @param private_key  SSL Private Key
 *  @return Success of opening socket through MQTTNetwork
 *  @note There is no way to query if there is an MQTT network behind the socket;
 *        this only checks if the socket is successfully opened.
 */
bool DecadaManagerV2::ConnectMqttNetwork(std::string root_ca, std::string client_cert, std::string private_key)
{
    if (!network_)
    {
        tr_error("NetworkInterface NULL");   
    }
    mqtt_network_ = new MQTTNetwork(network_);

    int rc = mqtt_network_->connect(broker_ip_.c_str(), mqtt_server_port_, root_ca.c_str(),
            client_cert.c_str(), private_key.c_str());

    if (rc != 0)
    {
        tr_err("Failed to set-up socket (rc = %d)", rc);
        
        return false;
    }
    else
    {
        tr_info("Opened socket on %s:%d", broker_ip_.c_str(), mqtt_server_port_);
    }
    
    return true;
}

/**
 *  @brief  Connect to MQTT broker (eg. Mosquitto).
 *  @author Lau Lee Hong
 *  @return Success of connection to MQTT broker
 */
bool DecadaManagerV2::ConnectMqttClient(void)
{
    if (!mqtt_network_)
    {
        tr_error("MQTTNetwork NULL");   
    }
    const std::string decada_device_key = device_uuid;
    const std::string decada_product_key = MBED_CONF_APP_DECADA_PRODUCT_KEY;
    int rtc_time_ms = RawRtcTimeNow();
    std::string time_now_milli_sec = MsPaddingIntToString(rtc_time_ms);

    std::string sha256_input = "clientId" + decada_device_key + "deviceKey" + decada_device_key + "productKey" + decada_product_key +
     "timestamp" + time_now_milli_sec + device_secret_;

    std::string sha256_output = GenericSHA256Generator(sha256_input);

    std::string mqtt_decada_client_id = decada_device_key + "|securemode=2,signmethod=sha256,timestamp=" + time_now_milli_sec + "|";
    std::string mqtt_decada_username = decada_device_key + "&" + decada_product_key;
    std::string mqtt_decada_password = ToLowerCase(sha256_output);
    
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = (char*)mqtt_decada_client_id.c_str();
    data.username.cstring = (char*)mqtt_decada_username.c_str();
    data.password.cstring = (char*)mqtt_decada_password.c_str();
    data.keepAliveInterval = 3600;      // keep tcp connection open for 60mins

    mqtt_client_ = new MQTT::Client<MQTTNetwork, Countdown>(*mqtt_network_);
    
    int rc = mqtt_client_->connect(data);
    if (rc != MQTT::SUCCESS)
    {
        tr_err("rc from MQTT connect is %d", rc);
     
        return false;
    }
    else
    {
        tr_info("MQTT client successfully connected to broker");
    }
    
    return true;
}

/**
 *  @brief  Disconnect the MQTT network.
 *  @author Lau Lee Hong
 *  @param  mqtt_network Reference to MQTT Network pointer
 */
void DecadaManagerV2::DisconnectMqttNetwork(void)
{
    int rc = mqtt_network_->disconnect();
    if (rc != 0)
    {
        tr_warn("Failed to disconnect from MQTT network. (rc = %d)",rc);   
    }
    delete mqtt_network_;
    mqtt_network_ = NULL;
    
    return;        
}

/**
 *  @brief  Disconnect the MQTT client.
 *  @author Lee Tze Han
 */
void DecadaManagerV2::DisconnectMqttClient(void)
{
    for (auto& sub_topic : sub_topics_)
    {
        int rc = mqtt_client_->unsubscribe(sub_topic.c_str());
        if (rc != 0)
        {
            tr_warn("Failed to unsubscribe to MQTT.(rc = %d)",rc);   
        }
        
        rc = mqtt_client_->setMessageHandler(sub_topic.c_str(), 0);
        if (rc != 0)
        {
            tr_warn("Failed to set message handler(rc = %d)",rc); 
        }
    }

    if (mqtt_client_->isConnected())
    {
        int rc = mqtt_client_->disconnect();
        if (rc != 0)
        {
            tr_warn("Failed to disconnect MQTT client(rc = %d)",rc);   
        }
    }

    delete mqtt_client_;
    mqtt_client_ = NULL;

    return;        
}

/**
 *  @brief  Reconnect MQTT network on DECADA.
 *  @author Lau Lee Hong, Ng Tze Yang
 *  @return Successful(1)/unsuccessful(0) MQTT Network Reconnection
 */
bool DecadaManagerV2::ReconnectMqttNetwork(std::string root_ca, std::string client_cert, std::string private_key)
{   
    bool rc = ConnectMqttNetwork(root_ca, client_cert, private_key);
    if (rc)
    {
        tr_info("Re-established connectivity with MQTT network");
    }
    else
    {
        tr_err("Could not established connectivity with MQTT network");
    }
    
    return rc; 
}

/**
 *  @brief  Reconnect MQTT client on DECADA.
 *  @author Lau Lee Hong
 *  @return Successful(1)/unsuccessful(0) MQTT Client Reconnection
 */
bool DecadaManagerV2::ReconnectMqttClient(void)
{
    bool connected = ConnectMqttClient();
    bool subscribed;
    for (auto& sub_topic : sub_topics_)
    {
        subscribed = Subscribe(sub_topic.c_str());
    }
    
    if (connected && subscribed)
    {
        tr_info("Re-established connectivity with MQTT client");
        return true;
    }
    else
    {
        tr_warn("Could not re-establish connectivity with MQTT client");
        return false;
    }
}

/**
 *  @brief  Disconnect and Reconnect MQTT network & client on DECADA.
 *  @author Ng Tze Yang, Lau Lee Hong
 *  @return Successful(1)/unsuccessful(0) MQTT Client Reconnection
 */
bool DecadaManagerV2::ReconnectMqttService(std::string root_ca, std::string client_cert, std::string private_key)
{   
    const uint8_t max_mqtt_failed_reconnections = 5;
    static uint8_t mqtt_failed_reconnections = 0;

    mqtt_mutex.lock();
    DisconnectMqttNetwork();
    DisconnectMqttClient();
    bool network_is_connected = ReconnectMqttNetwork(root_ca, client_cert, private_key);
    bool client_is_connected = ReconnectMqttClient();
    mqtt_mutex.unlock();
        
    while (!(network_is_connected && client_is_connected))
    {
        network_is_connected = ReconnectMqttNetwork(root_ca, client_cert, private_key);
        client_is_connected = ReconnectMqttClient();

        mqtt_failed_reconnections++;
        if (mqtt_failed_reconnections == max_mqtt_failed_reconnections)
        {
            NVIC_SystemReset();
        }
    }
    mqtt_failed_reconnections = 0;
    return true;
}

 /** @}*/ 