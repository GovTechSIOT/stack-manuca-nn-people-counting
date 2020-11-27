#include "cmsis_os.h"
#include "mbed.h"
#include "utest/utest.h"
#include "unity/unity.h"
#include "greentea-client/test_env.h"
#include "param_control.h"
#include "global_params.h"

using namespace utest::v1;

// Test receive of control message - sensor thread endpoint
static control_t distribute_control_message_test_1(const size_t call_count) 
{
    const std::string expected_param = "sensor_poll_rate";
    const int expected_value = 1000;
    const std::string expected_msg_id = "foo123";
    const std::string expected_endpoint_id = "123";

    std::string actual_param;
    int actual_value;
    std::string actual_msg_id;
    std::string actual_endpoint_id;

    DistributeControlMessage(expected_param, expected_value, expected_msg_id, expected_endpoint_id);

    osEvent evt = sensor_control_mail_box.get(1000);
    if (evt.status == osEventMail)
    {
        sensor_control_mail_t *sensor_control_mail = (sensor_control_mail_t *)evt.value.p;
        actual_param = sensor_control_mail->param;
        free(sensor_control_mail->param);
        actual_value = sensor_control_mail->value;
        sensor_control_mail_box.free(sensor_control_mail);
        actual_msg_id = sensor_control_mail->msg_id;
        free(sensor_control_mail->msg_id);
        actual_endpoint_id = sensor_control_mail->endpoint_id;
        free(sensor_control_mail->endpoint_id);
    }

    TEST_ASSERT_EQUAL_STRING(expected_param.c_str(), actual_param.c_str());
    TEST_ASSERT_EQUAL_INT(expected_value, actual_value);
    TEST_ASSERT_EQUAL_STRING(expected_msg_id.c_str(), actual_msg_id.c_str());
    TEST_ASSERT_EQUAL_STRING(expected_endpoint_id.c_str(), actual_endpoint_id.c_str());

    return CaseNext;
}

// Test receive of control message - invalid endpoint
static control_t distribute_control_message_test_2(const size_t call_count) 
{
    const std::string expected_param = "foo_bar";
    const int expected_value = 1000;
    const std::string expected_msg_id = "foo123";
    const std::string expected_endpoint_id = "123";

    std::string actual_param;
    int actual_value;
    std::string actual_msg_id;
    std::string actual_endpoint_id;

    DistributeControlMessage(expected_param, expected_value, expected_msg_id, expected_endpoint_id);

    osEvent evt = sensor_control_mail_box.get(1000);
    if (evt.status == osEventMail)
    {
        sensor_control_mail_t *sensor_control_mail = (sensor_control_mail_t *)evt.value.p;
        actual_param = sensor_control_mail->param;
        free(sensor_control_mail->param);
        actual_value = sensor_control_mail->value;
        sensor_control_mail_box.free(sensor_control_mail);
        actual_msg_id = sensor_control_mail->msg_id;
        free(sensor_control_mail->msg_id);
        actual_endpoint_id = sensor_control_mail->endpoint_id;
        free(sensor_control_mail->endpoint_id);
    }

    TEST_ASSERT_NOT_EQUAL(expected_param.c_str(), actual_param.c_str());
    TEST_ASSERT_NOT_EQUAL(expected_value, actual_value);
    TEST_ASSERT_NOT_EQUAL(expected_msg_id.c_str(), actual_msg_id.c_str());
    TEST_ASSERT_NOT_EQUAL(expected_endpoint_id.c_str(), actual_endpoint_id.c_str());

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
    Case("Test distribution of control message - sensor thread", distribute_control_message_test_1),
    Case("Test receive of control message - invalid endpoint", distribute_control_message_test_2)
};

Specification specification(greentea_setup, cases);

int main() 
{
    return !Harness::run(specification);
}