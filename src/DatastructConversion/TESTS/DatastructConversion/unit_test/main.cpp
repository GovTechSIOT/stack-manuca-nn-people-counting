#include "mbed.h"
#include "utest/utest.h"
#include "unity/unity.h"
#include "greentea-client/test_env.h"
#include "conversions.h"

using namespace utest::v1;

// Test for integrity of pointer after free-ing neighbouring pointers 
static control_t convert_string_to_char_test_1(const size_t call_count) 
{
    std::string dummy_str_1 = "foolushou";
    std::string actual_str = "hellosensorpod";
    std::string dummy_str_2 = "barbarblacksheep";
    
    char* dummy_ptr_1 = StringToChar(dummy_str_1);
    char* actual_ptr = StringToChar(actual_str);
    char* dummy_ptr_2 = StringToChar(dummy_str_2);
    
    char* expected_ptr = (char*) "hellosensorpod";

    free(dummy_ptr_1);
    free(dummy_ptr_2);

    TEST_ASSERT_EQUAL_STRING_MESSAGE(expected_ptr, actual_ptr, "Mem ptr allocated was unintentionally deallocated");
    free(actual_ptr);
    return CaseNext;
}

// Test for failure on additional whitespace
static control_t convert_string_to_char_test_2(const size_t call_count) 
{
    std::string actual_str = "hellosensorpod";
    char* actual_ptr = StringToChar(actual_str);
    char* wrong_ptr = (char*) "hellosensorpod ";

    TEST_ASSERT_NOT_EQUAL_MESSAGE(wrong_ptr, actual_ptr, "String object conversion to c-string failed");
    free(actual_ptr);
    return CaseNext;
}

// Test for large decimal value
static control_t convert_int_to_hex_test_1(const size_t call_count) 
{
    uint32_t actual = 1234567890;
    std::string actual_hex = IntToHex(actual);
    std::string expected_hex = "499602d2";

    TEST_ASSERT_EQUAL_STRING(expected_hex.c_str(), actual_hex.c_str());
    return CaseNext;
}

// Test for decimal 0
static control_t convert_int_to_hex_test_2(const size_t call_count) 
{
    uint32_t actual = 0;
    std::string actual_hex = IntToHex(actual);
    std::string expected_hex = "0";

    TEST_ASSERT_EQUAL_STRING(expected_hex.c_str(), actual_hex.c_str());
    return CaseNext;
}

// Test for wrong conversion
static control_t convert_int_to_hex_test_3(const size_t call_count) 
{
    uint32_t actual = 987654;
    std::string actual_hex = IntToHex(actual);
    std::string wrong_hex = "e1206";    // expected 0xf1206

    TEST_ASSERT_NOT_EQUAL_MESSAGE(wrong_hex , actual_hex, "Actual hex is wrongly intepreted");
    return CaseNext;
}

// Test for functionality - positive int
static control_t convert_int_to_string_test_1(const size_t call_count) 
{
    int actual = 1234567890;
    std::string actual_str = IntToString(actual);
    std::string expected_str = "1234567890";

    TEST_ASSERT_EQUAL_STRING_MESSAGE(expected_str.c_str(), actual_str.c_str(), "Positive-Int to String conversion failed");

    return CaseNext;
}

// Test for functionality - negative int
static control_t convert_int_to_string_test_2(const size_t call_count) 
{
    int actual = -54321;
    std::string actual_str = IntToString(actual);
    std::string expected_str = "-54321";

    TEST_ASSERT_EQUAL_STRING_MESSAGE(expected_str.c_str(), actual_str.c_str(), "Negative-Int to String conversion failed");

    return CaseNext;
}

// Test for functionality - failure by attenuation
static control_t convert_int_to_string_test_3(const size_t call_count) 
{
    int actual = 1234567890;
    std::string actual_str = IntToString(actual);
    std::string wrong_str = "123456789";    // expected: "1234567890"

    TEST_ASSERT_NOT_EQUAL_MESSAGE(wrong_str , actual_str, "Actual number is attenuated on conversion");

    return CaseNext;
}

// Test for decimal place attenuations
static control_t convert_double_to_char_test_1(const size_t call_count) 
{
    char buf[32];
    double actual_data = 321.234567890;
    
    int decimal_places = 0;
    std::string actual_data_str = DoubleToChar(buf, actual_data, decimal_places);
    std::string expected_data_str = "321.0";
    TEST_ASSERT_EQUAL_STRING(expected_data_str.c_str(), actual_data_str.c_str());
    int dp = actual_data_str.substr(actual_data_str.find(".")+1).length();
    TEST_ASSERT_EQUAL_INT_MESSAGE(decimal_places+1, dp, "Decimal places were not attenuated");

    memset(buf, 0, sizeof buf);
    decimal_places = 2;
    actual_data_str = DoubleToChar(buf, actual_data, decimal_places);
    expected_data_str = "321.23";
    TEST_ASSERT_EQUAL_STRING(expected_data_str.c_str(), actual_data_str.c_str());
    dp = actual_data_str.substr(actual_data_str.find(".")+1).length();
    TEST_ASSERT_EQUAL_INT_MESSAGE(decimal_places, dp, "Data was not attenuated at 2 decimal places");

    return CaseNext;
}

// Test for attenuation without rounding
static control_t convert_double_to_char_test_2(const size_t call_count) 
{
    char buf[32];
    double actual_data = 321.987654321;
    
    int decimal_places = 2;
    std::string actual_data_str = DoubleToChar(buf, actual_data, decimal_places);
    std::string wrong_data_str = "321.99";
    TEST_ASSERT_NOT_EQUAL(wrong_data_str, actual_data_str);     // Expected: 321.98

    return CaseNext;
}

// Test for negative input
static control_t convert_double_to_char_test_3(const size_t call_count) 
{
    char buf[32];
    double actual_data = -321.9876;
    
    int decimal_places = 2;
    std::string actual_data_str = DoubleToChar(buf, actual_data, decimal_places);
    std::string expected_data_str = "-321.98";
    TEST_ASSERT_EQUAL_STRING(expected_data_str.c_str(), actual_data_str.c_str());     // Expected: ''

    return CaseNext;
}

// Test for large string
static control_t convert_string_to_int_test_1(const size_t call_count) 
{
    std::string actual_str = "1234567890";
    int actual_int = StringToInt(actual_str);
    int expected_int = 1234567890;

    TEST_ASSERT_EQUAL_INT(expected_int, actual_int);
    return CaseNext;
}

// Test for corner case - zero
static control_t convert_string_to_int_test_2(const size_t call_count) 
{
    std::string actual_str = "0";
    int actual_int = StringToInt(actual_str);
    int expected_int = 0;

    TEST_ASSERT_EQUAL_INT(expected_int, actual_int);

    return CaseNext;
}

// Test for negative input
static control_t convert_string_to_int_test_3(const size_t call_count) 
{
    std::string actual_str = "-1";
    int actual_int = StringToInt(actual_str);
    int expected_int = -1;

    TEST_ASSERT_EQUAL_INT(expected_int, actual_int);

    return CaseNext;
}

// Test for non-decimal value
static control_t convert_string_to_int_test_4(const size_t call_count) 
{
    std::string actual_str = "abc";
    int actual_int = StringToInt(actual_str);
    int expected_int = 0;

    TEST_ASSERT_EQUAL_INT(expected_int, actual_int);

    return CaseNext;
}

// Test for attenuating leading zero
static control_t convert_string_to_int_test_5(const size_t call_count) 
{
    std::string actual_str = "012345";
    int actual_int = StringToInt(actual_str);
    int expected_int = 12345;

    TEST_ASSERT_EQUAL_INT(expected_int, actual_int);

    return CaseNext;
}

// Test for functionality - positive time
static control_t convert_timet_to_string_test_1(const size_t call_count) 
{
    std::time_t actual = 1234567890;
    std::string actual_str = TimeToString(actual);
    std::string expected_str = "1234567890";

    TEST_ASSERT_EQUAL_STRING(expected_str.c_str(), actual_str.c_str());

    return CaseNext;
}

// Test for functionality - negative time (thrown during error)
static control_t convert_timet_to_string_test_2(const size_t call_count) 
{
    std::time_t actual = -1;
    std::string actual_str = TimeToString(actual);
    std::string expected_str = "-1";

    TEST_ASSERT_EQUAL_STRING(expected_str.c_str(), actual_str.c_str());

    return CaseNext;
}

// Test for failure by attenuation
static control_t convert_timet_to_string_test_3(const size_t call_count) 
{
    std::time_t actual = 1234567890;
    std::string actual_str = TimeToString(actual);
    std::string wrong_str = "123456789";    // expected: "1234567890"

    TEST_ASSERT_NOT_EQUAL_MESSAGE(wrong_str , actual_str, "Actual time is attenuated on conversion");

    return CaseNext;
}

// Test for large string
static control_t convert_string_to_timet_test_1(const size_t call_count) 
{
    std::string actual_str = "1234567890";
    std::time_t actual_timet = StringToTime(actual_str);
    int expected_timet = 1234567890;

    TEST_ASSERT_EQUAL_INT(expected_timet, actual_timet);
    return CaseNext;
}

// Test for corner case - zero
static control_t convert_string_to_timet_test_2(const size_t call_count) 
{
    std::string actual_str = "0";
    std::time_t actual_timet = StringToTime(actual_str);
    int expected_timet = 0;

    TEST_ASSERT_EQUAL_INT(expected_timet, actual_timet);
    return CaseNext;
}


// Test for negative input
static control_t convert_string_to_timet_test_3(const size_t call_count) 
{
    std::string actual_str = "-1";
    std::time_t actual_timet = StringToTime(actual_str);
    int expected_timet = -1;

    TEST_ASSERT_EQUAL_INT(expected_timet, actual_timet);
    return CaseNext;
}

// Test for non-decimal value
static control_t convert_string_to_timet_test_4(const size_t call_count) 
{
    std::string actual_str = "abc";
    std::time_t actual_timet = StringToTime(actual_str);
    int expected_timet = 0;

    TEST_ASSERT_EQUAL_INT(expected_timet, actual_timet);
    return CaseNext;
}

// Test for attenuating leading zero
static control_t convert_string_to_timet_test_5(const size_t call_count) 
{
    std::string actual_str = "012345";
    std::time_t actual_timet = StringToTime(actual_str);
    int expected_timet = 12345;

    TEST_ASSERT_EQUAL_INT(expected_timet, actual_timet);
    return CaseNext;
}

// Test for converting lowercase alphabets to uppercase - pure lowercase
static control_t convert_lowercase_to_uppercase_alphabets_test_1(const size_t call_count) 
{
    std::string expected_result = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string input = "abcdefghijklmnopqrstuvwxyz";
    std::string actual_result = ToUpperCase(input);

    TEST_ASSERT_EQUAL_STRING(expected_result.c_str(), actual_result.c_str());
    return CaseNext;
}

// Test for converting lowercase alphabets to uppercase - mix of uppercase, lowercase, numbers, special symbols
static control_t convert_lowercase_to_uppercase_alphabets_test_2(const size_t call_count) 
{
    std::string expected_result = "1AA2BB3CC4DD5EE#FF%GG&HH IIJJKKLLMMNNOOPPQQRRSSTTUUVVWWXXYYZZ";
    std::string input = "1aA2bB3cC4dD5eE#fF%gG&hH iIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ";
    std::string actual_result = ToUpperCase(input);

    TEST_ASSERT_EQUAL_STRING(expected_result.c_str(), actual_result.c_str());
    return CaseNext;
}

// Test for converting lowercase alphabets to uppercase - null input string
static control_t convert_lowercase_to_uppercase_alphabets_test_3(const size_t call_count) 
{
    std::string expected_result = "";
    std::string input = "";
    std::string actual_result = ToUpperCase(input);

    TEST_ASSERT_EQUAL_STRING(expected_result.c_str(), actual_result.c_str());
    return CaseNext;
}

// Test for converting uppercase alphabets to lowcase - pure lowercase
static control_t convert_uppercase_to_lowercase_alphabets_test_1(const size_t call_count) 
{
    std::string expected_result = "abcdefghijklmnopqrstuvwxyz";
    std::string input = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string actual_result = ToLowerCase(input);

    TEST_ASSERT_EQUAL_STRING(expected_result.c_str(), actual_result.c_str());
    return CaseNext;
}

// Test for converting uppercase alphabets to lowercase - mix of uppercase, lowercase, numbers, special symbols
static control_t convert_uppercase_to_lowercase_alphabets_test_2(const size_t call_count) 
{
    std::string expected_result = "1aa2bb3cc4dd5ee#ff%gg&hh iijjkkllmmnnooppqqrrssttuuvvwwxxyyzz";
    std::string input = "1aA2bB3cC4dD5eE#fF%gG&hH iIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ";
    std::string actual_result = ToLowerCase(input);

    TEST_ASSERT_EQUAL_STRING(expected_result.c_str(), actual_result.c_str());
    return CaseNext;
}

// Test for converting uppercase alphabets to lowercase - null input string
static control_t convert_uppercase_to_lowercase_alphabets_test_3(const size_t call_count) 
{
    std::string expected_result = "";
    std::string input = "";
    std::string actual_result = ToLowerCase(input);

    TEST_ASSERT_EQUAL_STRING(expected_result.c_str(), actual_result.c_str());
    return CaseNext;
}

// Test for converting string to double - long (8) decimal places
static control_t convert_string_to_double_test_1(const size_t call_count) 
{
    double expected_result = 3.14159265;
    std::string input = "3.14159265";
    double actual_result = StringToDouble(input);

    TEST_ASSERT_EQUAL_DOUBLE(expected_result, actual_result);
    return CaseNext;
}
// Test for converting string to double - integer input
static control_t convert_string_to_double_test_2(const size_t call_count) 
{
    double expected_result = 9.00;
    std::string input = "9";
    double actual_result = StringToDouble(input);

    TEST_ASSERT_EQUAL_DOUBLE(expected_result, actual_result);
    return CaseNext;
}

// Test for converting string to double - negative number
static control_t convert_string_to_double_test_3(const size_t call_count) 
{
    double expected_result = -3.14159265;
    std::string input = "-3.14159265";
    double actual_result = StringToDouble(input);

    TEST_ASSERT_EQUAL_DOUBLE(expected_result, actual_result);
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
    Case("Check StringToChar Integrity of pointer after free-ing neighbouring pointers", convert_string_to_char_test_1),
    Case("Check StringToChar Failure on additional whitespace", convert_string_to_char_test_2),
    Case("Check IntToHex Large decimal value", convert_int_to_hex_test_1),
    Case("Check IntToHex Decimal 0", convert_int_to_hex_test_2),
    Case("Check IntToHex Wrong conversion", convert_int_to_hex_test_3),
    Case("Check IntToString Functionality - positive int", convert_int_to_string_test_1),
    Case("Check IntToString Functionality - negative int", convert_int_to_string_test_2),
    Case("Check IntToString Functionality - failure by attenuation", convert_int_to_string_test_3),
    Case("Check DoubleToChar Decimal place attenuations", convert_double_to_char_test_1),
    Case("Check DoubleToChar Attenuation without rounding", convert_double_to_char_test_2),
    Case("Check DoubleToChar Negative input", convert_double_to_char_test_3),
    Case("Check StringToInt Large string", convert_string_to_int_test_1),
    Case("Check StringToInt Corner case - zero", convert_string_to_int_test_2),
    Case("Check StringToInt Negative input", convert_string_to_int_test_3),
    Case("Check StringToInt Non-decimal value", convert_string_to_int_test_4),
    Case("Check StringToInt Attenuating leading zero", convert_string_to_int_test_5),
    Case("Check TimeToString Functionality - positive time", convert_timet_to_string_test_1),
    Case("Check TimeToString Functionality - negative time (thrown during error)", convert_timet_to_string_test_2),
    Case("Check TimeToString Failure by attenuation", convert_timet_to_string_test_3),
    Case("Check StringToTime Large string", convert_string_to_timet_test_1),
    Case("Check StringToTime Corner case - zero", convert_string_to_timet_test_2),
    Case("Check StringToTime Negative input", convert_string_to_timet_test_3),
    Case("Check StringToTime Non-decimal value", convert_string_to_timet_test_4),
    Case("Check StringToTime Attenuating leading zero", convert_string_to_timet_test_5),
    Case("Check ToUpperCase converting lowercase alphabets to uppercase - pure lowercase", convert_lowercase_to_uppercase_alphabets_test_1),
    Case("Check ToUpperCase converting lowercase alphabets to uppercase - mix of uppercase, lowercase, numbers, special symbols", convert_lowercase_to_uppercase_alphabets_test_2),
    Case("Check ToUpperCase converting lowercase alphabets to uppercase - null input string", convert_lowercase_to_uppercase_alphabets_test_3),
    Case("Check ToLowerCase converting uppercase alphabets to lowercase - pure uppercase", convert_uppercase_to_lowercase_alphabets_test_1),
    Case("Check ToLowerCase converting uppercase alphabets to lowercase - mix of uppercase, lowercase, numbers, special symbols", convert_uppercase_to_lowercase_alphabets_test_2),
    Case("Check ToLowerCase converting uppercase alphabets to lowercase - null input string", convert_uppercase_to_lowercase_alphabets_test_3),
    Case("Check StringToDouble converting string to double - long (8) decimal places", convert_string_to_double_test_1),
    Case("Check StringToDouble converting string to double - integer input", convert_string_to_double_test_2),
    Case("Check StringToDouble converting string to double - negative number", convert_string_to_double_test_3)

};

Specification specification(greentea_setup, cases);

int main() 
{
    return !Harness::run(specification);
}