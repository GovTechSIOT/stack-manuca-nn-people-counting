#include "mbed.h"
#include "utest/utest.h"
#include "unity/unity.h"
#include "greentea-client/test_env.h"
#include "sensor_profile.h"
#include "global_params.h"

using namespace utest::v1;

// Test for update of member variable, and getting json packet - temperature
static control_t update_value_test_1(const size_t call_count) 
{
    const std::string uuid = "0x" + device_uuid;
    std::string expected_packet, actual_packet;
    SensorProfile temperature_profile;
    temperature_profile.UpdateValue("temperature", "1.00", 5);

    // {"id":"<replace with actual uuid>","method":"thing.measurepoint.post","params":{"measurepoints":{"temperature":1.0}},"version":"1.0"}
    expected_packet = "{\"id\":\"" + device_uuid + "\",\"method\":\"thing.measurepoint.post\",\"params\":{\"measurepoints\":{\"temperature\":1.0}},\"version\":\"1.0\"}";
    actual_packet = temperature_profile.GetNewDecadaPacket();

    TEST_ASSERT_EQUAL_STRING(expected_packet.c_str(), actual_packet.c_str());

    return CaseNext;
}

// Test for update of member variable, and getting json packet - humidity
static control_t update_value_test_2(const size_t call_count) 
{
    std::string uuid = "0x" + device_uuid;
    std::string expected_packet, actual_packet;
    SensorProfile humidity_profile;
    humidity_profile.UpdateValue("humidity", "3.4567", 5);

    // {"id":"<replace with actual uuid>","method":"thing.measurepoint.post","params":{"measurepoints":{"humidity":3.4567}},"version":"1.0"}
    expected_packet = "{\"id\":\"" + device_uuid + "\",\"method\":\"thing.measurepoint.post\",\"params\":{\"measurepoints\":{\"humidity\":3.4567}},\"version\":\"1.0\"}";
    actual_packet = humidity_profile.GetNewDecadaPacket();

    TEST_ASSERT_EQUAL_STRING(expected_packet.c_str(), actual_packet.c_str());

    return CaseNext;
}

// Test for update of member variable, and getting json packet - CO2
static control_t update_value_test_3(const size_t call_count) 
{
    std::string uuid = "0x" + device_uuid;
    std::string expected_packet, actual_packet;
    SensorProfile co2_profile;
    co2_profile.UpdateValue("co2", "999.99", 5);

    // {"id":"<replace with actual uuid>","method":"thing.measurepoint.post","params":{"measurepoints":{"CO2":999.99}},"version":"1.0"}
    expected_packet = "{\"id\":\"" + device_uuid + "\",\"method\":\"thing.measurepoint.post\",\"params\":{\"measurepoints\":{\"co2\":999.99}},\"version\":\"1.0\"}";
    actual_packet = co2_profile.GetNewDecadaPacket();

    TEST_ASSERT_EQUAL_STRING(expected_packet.c_str(), actual_packet.c_str());

    return CaseNext;
}

// Test for update of member variable, and getting json packet - PM2.5_mass
static control_t update_value_test_4(const size_t call_count) 
{
    std::string uuid = "0x" + device_uuid;
    std::string expected_packet, actual_packet;
    SensorProfile pm25_profile;
    pm25_profile.UpdateValue("PM2.5_mass", "0.12", 5);

    // {"id":"<replace with actual uuid>","method":"thing.measurepoint.post","params":{"measurepoints":{"PM2.5_mass":0.12}},"version":"1.0"}
    expected_packet = "{\"id\":\"" + device_uuid + "\",\"method\":\"thing.measurepoint.post\",\"params\":{\"measurepoints\":{\"PM2.5_mass\":0.12}},\"version\":\"1.0\"}";
    actual_packet = pm25_profile.GetNewDecadaPacket();

    TEST_ASSERT_EQUAL_STRING(expected_packet.c_str(), actual_packet.c_str());

    return CaseNext;
}

// Test for update of member variable, and getting json packet - PM10_mass
static control_t update_value_test_5(const size_t call_count) 
{
    std::string uuid = "0x" + device_uuid;
    std::string expected_packet, actual_packet;
    SensorProfile pm10_profile;
    pm10_profile.UpdateValue("PM10_mass", "0.69", 5);

    // {"id":"<replace with actual uuid>","method":"thing.measurepoint.post","params":{"measurepoints":{"PM10_mass":0.69}},"version":"1.0"}
    expected_packet = "{\"id\":\"" + device_uuid + "\",\"method\":\"thing.measurepoint.post\",\"params\":{\"measurepoints\":{\"PM10_mass\":0.69}},\"version\":\"1.0\"}";
    actual_packet = pm10_profile.GetNewDecadaPacket();

    TEST_ASSERT_EQUAL_STRING(expected_packet.c_str(), actual_packet.c_str());

    return CaseNext;
}

// Test for update of member variable, and getting json packet - invalid
static control_t update_value_test_6(const size_t call_count) 
{
    std::string uuid = "0x" + device_uuid;
    std::string expected_packet, actual_packet;
    SensorProfile foo_profile;
    foo_profile.UpdateValue("bar", "1234.567", 5);

    // {"id":"<replace with actual uuid>","method":"thing.measurepoint.post","params":{"measurepoints":{"bar":1234.567}},"version":"1.0"}
    expected_packet = "{\"id\":\"" + device_uuid + "\",\"method\":\"thing.measurepoint.post\",\"params\":{\"measurepoints\":{\"bar\":1234.567}},\"version\":\"1.0\"}";
    actual_packet = foo_profile.GetNewDecadaPacket();

    TEST_ASSERT_EQUAL_STRING(expected_packet.c_str(), actual_packet.c_str());

    return CaseNext;
}

// Test for update of member variable, and getting json packet after updating entity list with new timestamp
static control_t update_value_test_7(const size_t call_count) 
{
    const std::string uuid = "0x" + device_uuid;
    std::string expected_packet, actual_packet;
    SensorProfile temperature_profile;
    temperature_profile.UpdateValue("temperature", "1.00", 5);
    temperature_profile.UpdateEntityList(10);
    
    // {"id":"<replace with actual uuid>","method":"thing.measurepoint.post","params":{"measurepoints":null},"version":"1.0"}
    expected_packet = "{\"id\":\"" + device_uuid + "\",\"method\":\"thing.measurepoint.post\",\"params\":{\"measurepoints\":null},\"version\":\"1.0\"}";
    actual_packet = temperature_profile.GetNewDecadaPacket();

    TEST_ASSERT_EQUAL_STRING(expected_packet.c_str(), actual_packet.c_str());

    return CaseNext;
}

// Test for checking for data availability after update of member variable
static control_t update_value_test_8(const size_t call_count) 
{
    const std::string uuid = "0x" + device_uuid;
    bool expected_ret, actual_ret;
    SensorProfile temperature_profile;
    temperature_profile.UpdateValue("temperature", "1.00", 5);
    
    expected_ret = true;
    actual_ret = temperature_profile.CheckEntityAvailability();

    TEST_ASSERT_EQUAL(expected_ret, actual_ret);

    return CaseNext;
}

// Test for checking for data availability after update of entity list with new timestamp, without new data
static control_t update_value_test_9(const size_t call_count) 
{
    const std::string uuid = "0x" + device_uuid;
    bool expected_ret, actual_ret;
    SensorProfile temperature_profile;
    temperature_profile.UpdateValue("temperature", "1.00", 5);
    temperature_profile.UpdateEntityList(10);
    
    expected_ret = false;
    actual_ret = temperature_profile.CheckEntityAvailability();

    TEST_ASSERT_EQUAL(expected_ret, actual_ret);
 
    return CaseNext;
}

utest::v1::status_t greentea_setup(const size_t number_of_cases) 
{
    // Here, we specify the timeout (60s) and the host test (a built-in host test or the name of our Python file)
    GREENTEA_SETUP(60, "default_auto");

    return greentea_test_setup_handler(number_of_cases);
}

// List of test cases in this file
Case cases[] = 
{
    Case("Test for update of member variable, and getting snon-style json packet - temperature", update_value_test_1),
    Case("Test for update of member variable, and getting snon-style json packet - humidity", update_value_test_2),
    Case("Test for update of member variable, and getting snon-style json packet - CO2", update_value_test_3),
    Case("Test for update of member variable, and getting snon-style json packet - PM2.5_mass", update_value_test_4),
    Case("Test for update of member variable, and getting snon-style json packet - PM10_mass", update_value_test_5),
    Case("Test for update of member variable, and getting snon-style json packet - invalid", update_value_test_6),
    Case("Test for update of member variable, and getting snon-style json packet after updating entity list with new timestamp", update_value_test_7),
    Case("Test for checking of data availability, after update of member variable", update_value_test_8),
    Case("Test for checking of data availability, after update of entity list with new timestamp without new data", update_value_test_9)
};

Specification specification(greentea_setup, cases);

int main() 
{
    return !Harness::run(specification);
}