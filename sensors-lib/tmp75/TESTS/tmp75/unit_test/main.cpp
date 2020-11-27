#include "mbed.h"
#include "utest/utest.h"
#include "unity/unity.h"
#include "greentea-client/test_env.h"
#include "tmp75.h"

#define TMP75_SDA   PB_9
#define TMP75_SCL   PB_6
#define ALRT_PIN    PD_10

using namespace utest::v1;

static control_t tmp_get_name_test_1(const size_t call_count) 
{
    
    Tmp75 tmp_test(TMP75_SDA, TMP75_SCL, ALRT_PIN, 400000);
    std::string expected_str = "tmp75";
    std::string actual_str = tmp_test.GetName();

    TEST_ASSERT_EQUAL_STRING(expected_str.c_str(), actual_str.c_str());

    return CaseNext;
}

static control_t tmp_get_data_test_1(const size_t call_count) 
{
    
    Tmp75 tmp_test(TMP75_SDA, TMP75_SCL, ALRT_PIN, 400000);
    std::vector<std::pair<std::string, std::string>> data_list;
    std::string actual_str;

    tmp_test.Enable();
    int ret = tmp_test.GetData(data_list);
    if (ret == Tmp75::DATA_OK)
    {
        actual_str = data_list[0].first;
    }
    else actual_str = "disconnected";

    std::string expected_str = "ambient_temp";
    TEST_ASSERT_EQUAL_STRING(expected_str.c_str(), actual_str.c_str());

    return CaseNext;
}

static control_t tmp_get_data_test_2(const size_t call_count) 
{
    
    Tmp75 tmp_test(TMP75_SDA, TMP75_SCL, ALRT_PIN, 400000);
    std::vector<std::pair<std::string, std::string>> data_list;
    std::string data_str;

    tmp_test.Enable();
    int ret = tmp_test.GetData(data_list);

    if (ret == Tmp75::DATA_OK)
    {
        data_str = data_list[0].second;
    }
    else data_str = "9999.99";

    float data = std::stof(data_str);
    float expected_data = 50.00;    // test range between 0 to 100C
    float delta = 50.00;

    TEST_ASSERT_FLOAT_WITHIN(delta, expected_data, data);

    return CaseNext;
}

static control_t tmp_enable_test_1(const size_t call_count) 
{
    
    Tmp75 tmp_test(TMP75_SDA, TMP75_SCL, ALRT_PIN, 400000);
    std::vector<std::pair<std::string, std::string>> data_list;

    tmp_test.Enable();
    int actual_ret = tmp_test.GetData(data_list);
    int expected_ret = Tmp75::DATA_OK;

    TEST_ASSERT_EQUAL_INT(expected_ret, actual_ret);

    return CaseNext;
}

static control_t tmp_disable_test_1(const size_t call_count) 
{
    
    Tmp75 tmp_test(TMP75_SDA, TMP75_SCL, ALRT_PIN, 400000);
    std::vector<std::pair<std::string, std::string>> data_list;

    tmp_test.Disable();
    int actual_ret = tmp_test.GetData(data_list);
    int expected_ret = Tmp75::DISCONNECT;

    TEST_ASSERT_EQUAL_INT(expected_ret, actual_ret);

    return CaseNext;
}

static control_t tmp_reset_test_1(const size_t call_count) 
{
    
    Tmp75 tmp_test(TMP75_SDA, TMP75_SCL, ALRT_PIN, 400000);
    std::vector<std::pair<std::string, std::string>> data_list;

    tmp_test.Enable();
    tmp_test.Reset();
    int actual_ret = tmp_test.GetData(data_list);
    int expected_ret = Tmp75::DATA_OK;

    TEST_ASSERT_EQUAL_INT(expected_ret, actual_ret);

    return CaseNext;
}

static control_t tmp_reset_test_2(const size_t call_count) 
{
    
    Tmp75 tmp_test(TMP75_SDA, TMP75_SCL, ALRT_PIN, 400000);
    std::vector<std::pair<std::string, std::string>> data_list;
    std::string data_str;

    tmp_test.Enable();
    tmp_test.Reset();
    int ret = tmp_test.GetData(data_list);
    if (ret == Tmp75::DATA_OK)
    {
        data_str = data_list[0].second;
    }
    else data_str = "9999.99";

    float data = std::stof(data_str);
    float expected_data = 50.00;    // test range between 0 to 100C
    float delta = 50.00;

    TEST_ASSERT_FLOAT_WITHIN(delta, expected_data, data);

    return CaseNext;
}

static control_t tmp_reset_test_3(const size_t call_count) 
{
    
    Tmp75 tmp_test(TMP75_SDA, TMP75_SCL, ALRT_PIN, 400000);
    std::vector<std::pair<std::string, std::string>> data_list;

    tmp_test.Disable();
    tmp_test.Reset();
    int actual_ret = tmp_test.GetData(data_list);
    int expected_ret = Tmp75::DISCONNECT;

    TEST_ASSERT_EQUAL_INT(expected_ret, actual_ret);

    return CaseNext;
}

static control_t tmp_config_test_1(const size_t call_count) 
{
    
    Tmp75 tmp_test(TMP75_SDA, TMP75_SCL, ALRT_PIN, 400000);

    int actual_ret = tmp_test.Configure();
    int expected_ret = Tmp75::TMPOK;

    TEST_ASSERT_EQUAL_INT(expected_ret, actual_ret);

    return CaseNext;
}

static control_t tmp_config_test_2(const size_t call_count) 
{
    
    Tmp75 tmp_test(TMP75_SDA, TMP75_SCL, ALRT_PIN, 400000);
    std::vector<std::pair<std::string, std::string>> data_list;
    std::string actual_str, expected_str, data_str;
    float data_val;

    float t_low = -5;
    float t_high = 0;

    tmp_test.Enable();
    int na = tmp_test.Configure(t_low, t_high);

    wait(1);
    int ret = tmp_test.GetData(data_list);
    if (ret == Tmp75::DATA_OK)
    {
        for (int i=0; i<data_list.size(); i++)
        {
            if (data_list[i].first == "ambient_temp")
            {
                data_str = data_list[i].second;
                data_val = std::stof(data_str);
                if (data_val >= t_high)
                {
                    expected_str = "ambient_temp_alert";     // expected if temp is normal > 5C
                }
                else expected_str = "no alert";

                break;
            }
        }

        for (int i=0; i<data_list.size(); i++)
        {    
            if (data_list[i].first == "ambient_temp_alert") 
            {
                actual_str = data_list[i].first;
                break;
            }
            else actual_str = "no alert";
        }
    }
    else actual_str = "disconnected";

    TEST_ASSERT_EQUAL_STRING(expected_str.c_str(), actual_str.c_str());

    return CaseNext;
}

static control_t tmp_config_test_3(const size_t call_count) 
{
    
    Tmp75 tmp_test(TMP75_SDA, TMP75_SCL, ALRT_PIN, 400000);
    std::vector<std::pair<std::string, std::string>> data_list;
    std::string actual_str, expected_str, data_str;
    float data_val;

    float t_low = 95;
    float t_high = 100;

    tmp_test.Enable();
    int na = tmp_test.Configure(t_low, t_high);

    wait(1);
    int ret = tmp_test.GetData(data_list);
    if (ret == Tmp75::DATA_OK)
    {
        for (int i=0; i<data_list.size(); i++)
        {
            if (data_list[i].first == "ambient_temp")
            {
                data_str = data_list[i].second;
                data_val = std::stof(data_str);
                if (data_val >= t_high)
                {
                    expected_str = "ambient_temp_alert";
                }
                else expected_str = "no alert";     // expected if temp is normal < 100C

                break;
            }
        }

        for (int i=0; i<data_list.size(); i++)
        {    
            if (data_list[i].first == "ambient_temp_alert") 
            {
                actual_str = data_list[i].first;
                break;
            }
            else actual_str = "no alert";
        }
    }
    else actual_str = "disconnected";

    TEST_ASSERT_EQUAL_STRING(expected_str.c_str(), actual_str.c_str());

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
    Case("Check Tmp75 GetName sensor name", tmp_get_name_test_1),
    Case("Check Tmp75 GetData data name", tmp_get_data_test_1),
    Case("Check Tmp75 GetData data value range between 0 to 100", tmp_get_data_test_2),
    Case("Check Tmp75 Enable", tmp_enable_test_1),
    Case("Check Tmp75 Disable", tmp_disable_test_1),
    Case("Check Tmp75 Reset when enabled", tmp_reset_test_1),
    Case("Check Tmp75 data after reset when enabled", tmp_reset_test_2),
    Case("Check Tmp75 Reset when disabled", tmp_reset_test_3),
    Case("Check Tmp75 Config", tmp_config_test_1),
    Case("Check Tmp75 Config low temp threshold", tmp_config_test_2),
    Case("Check Tmp75 Config high temp threshold", tmp_config_test_3),

};

Specification specification(greentea_setup, cases);

int main() 
{
    return !Harness::run(specification);
}