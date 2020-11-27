#include <cstring>
#include <cctype>
#include "cmsis_os.h"
#include "mbed.h"
#include "utest/utest.h"
#include "unity/unity.h"
#include "greentea-client/test_env.h"
#include "trace_macro.h"
#include "trace_manager.h"
#include "global_params.h"

using namespace utest::v1;

// Test decada service response message structure using raw string
static control_t create_decada_response_test_1(const size_t call_count) 
{
    // {"code":200,"data":{"poll_rate_updated":"true"},"id":"abc"}
    const std::string expected_msg = "{\"code\":200,\"data\":{\"poll_rate_updated\":\"true\"},\"id\":\"abc\"}";
    std::string actual_msg = CreateDecadaResponse("poll_rate_updated", "abc");
    
    TEST_ASSERT_EQUAL_STRING(expected_msg.c_str(), actual_msg.c_str());

    return CaseNext;
}

// Test decada service response message structure using x-macro
static control_t create_decada_response_test_2(const size_t call_count) 
{
    // {"code":200,"data":{"poll_rate_updated":"true"},"id":"abc"}
    const std::string expected_msg = "{\"code\":200,\"data\":{\"poll_rate_updated\":\"true\"},\"id\":\"abc\"}";
    std::string actual_msg = CreateDecadaResponse(trace_name[POLL_RATE_UPDATE], "abc");
    
    TEST_ASSERT_EQUAL_STRING(expected_msg.c_str(), actual_msg.c_str());

    return CaseNext;
}

// Test decada event trace message for c++ whitespace characters
static control_t create_decada_response_test_3(const size_t call_count) 
{
    std::string actual_snon = CreateDecadaResponse("mqtt_connect_success", "123");
    uint16_t expected_whitespaces = 0;

    int actual_whitespace = 0; 
    int length = actual_snon.length(); 
    for (int i = 0; i < length; i++) 
    { 
        int c = actual_snon[i]; 
        if (isspace(c))
        {
            actual_whitespace++;
        }  
    } 
    
    TEST_ASSERT_EQUAL_INT(expected_whitespaces, actual_whitespace);
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
    Case("Test decada service response message structure using raw string", create_decada_response_test_1),
    Case("Test decada service response message structure using x-macro", create_decada_response_test_2),
    Case("Test decada service response message for c++ whitespace characters", create_decada_response_test_3)
};

Specification specification(greentea_setup, cases);

int main() 
{
    return !Harness::run(specification);
}