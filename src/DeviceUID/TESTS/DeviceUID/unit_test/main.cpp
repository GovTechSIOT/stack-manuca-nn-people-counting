#include "mbed.h"
#include "utest/utest.h"
#include "unity/unity.h"
#include "greentea-client/test_env.h"
#include "device_uid.h"

using namespace utest::v1;


// Test for UID uniqueness
static control_t UID_static_test(const size_t call_count) {
    //printf("HERE: %s", GetDeviceUid().c_str()); // Use minicom to look at the log if need to debug
    std::string UID = GetDeviceUid();
    TEST_ASSERT_TRUE(GetDeviceUid() == UID);

    return CaseNext;
}

// Test for UID size
static control_t UID_size_test(const size_t call_count) {
    int UID_Count = 24;
    std::string UID = GetDeviceUid();
    TEST_ASSERT_TRUE(UID.size() == UID_Count);

    return CaseNext;
}

utest::v1::status_t greentea_setup(const size_t number_of_cases) {
    // Here, we specify the timeout (60s) and the host test (a built-in host test or the name of our Python file)
    GREENTEA_SETUP(60, "default_auto");

    return greentea_test_setup_handler(number_of_cases);
}

// List of test cases in this file
Case cases[] = {
    Case("Check static of UID", UID_static_test),
    Case("Check size of UID", UID_size_test)
};

Specification specification(greentea_setup, cases);

int main() {
    return !Harness::run(specification);
}