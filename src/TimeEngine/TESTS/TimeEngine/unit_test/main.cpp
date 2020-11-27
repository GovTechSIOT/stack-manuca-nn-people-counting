#include "mbed.h"
#include "utest/utest.h"
#include "unity/unity.h"
#include "greentea-client/test_env.h"
#include "time_engine.h"
#include "communications_network.h"
#include "conversions.h"


using namespace utest::v1;

void UpdateRtcToDefault(NTPClient& ntp)
{
    time_t raw_time = 0;
    set_time(raw_time);
        
    return;
}

void Sleep(unsigned int mseconds)
{
    clock_t goal = mseconds + clock();
    while (goal > clock());
}

// Test for not updating of rtc
static control_t update_rtc_test(const size_t call_count) 
{
    NetworkInterface* network = NULL;
    ConfigNetworkInterface(network);
    NTPClient ntp(network);

    UpdateRtcToDefault(ntp); // Default time, set to 0 which is 1970-01-01T00:00:00
    std::string default_time = ConvertRawTimeToIso8601Time(RawRtcTimeNow());
    UpdateRtc(ntp); // Current time
    std::string current_time = ConvertRawTimeToIso8601Time(RawRtcTimeNow());

    TEST_ASSERT_FALSE(default_time == current_time);
    
    return CaseNext;
}

// Test for retrieving of rtc
static control_t get_rtc_test(const size_t call_count) 
{
    int interval = 3;
    int seconds;

    std::string current_time = ConvertRawTimeToIso8601Time(RawRtcTimeNow());
    std::size_t start = current_time.find("T");
    int current_seconds = stoi(current_time.substr(start+7,2));

    Sleep(interval * 100); // 3 seconds

    std::string time_after_three_seconds = ConvertRawTimeToIso8601Time(RawRtcTimeNow());
    std::size_t start2 = time_after_three_seconds.find("T");
    int after_seconds = stoi(time_after_three_seconds.substr(start2+7,2));
    if (after_seconds < interval) 
    {
      after_seconds += 60;
    }

    seconds = abs(current_seconds - after_seconds);

    TEST_ASSERT_TRUE(seconds == interval);

  return CaseNext;
}

// Test for time formatting
static control_t format_time_test(const size_t call_count) 
{
    std::string single_char = "1";
    single_char = FormatTime(single_char);
    TEST_ASSERT_TRUE(single_char.size() == 2);

    return CaseNext;
}

// Test for converting raw to iso time - arbitrary time
static control_t convert_raw_to_iso_time_test_1(const size_t call_count) 
{
    const time_t raw_time = 1561519234;
    const std::string expected_result = "2019-06-26T03:20:34Z";

    std::string t = ConvertRawTimeToIso8601Time(raw_time);
    TEST_ASSERT_EQUAL_STRING(expected_result.c_str(), t.c_str());

    return CaseNext;
}

// Test for converting raw to iso time - zero time
static control_t convert_raw_to_iso_time_test_2(const size_t call_count) 
{
    const time_t raw_time = 0;
    const std::string expected_result = "1970-01-01T00:00:00Z";

    std::string t = ConvertRawTimeToIso8601Time(raw_time);
    TEST_ASSERT_EQUAL_STRING(expected_result.c_str(), t.c_str());

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
    Case("Check update real-time clock function", update_rtc_test),
    Case("Check get real-time clock function", get_rtc_test),
    Case("Check format time function", format_time_test),
    Case("Check for converting raw to iso time - arbitrary time", convert_raw_to_iso_time_test_1),
    Case("Check for converting raw to iso time - zero time", convert_raw_to_iso_time_test_2)
};

Specification specification(greentea_setup, cases);

int main() 
{
    return !Harness::run(specification);
}
