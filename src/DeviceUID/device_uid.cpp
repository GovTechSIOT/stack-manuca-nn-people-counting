/**
 * @defgroup device_uuid Device UUID
 * @{
 */

#include <stdint.h>
#include <sstream>
#include <iomanip>
#include "device_uid.h"

/**
 *  @brief  Returns a unique device ID
 *  @author Lee Tze Han
 *  @return C++ string containing 24-character hexadecimal representation of UID
 */
std::string GetDeviceUid(void)
{
    using namespace std;

    stringstream ss;
    ss << hex;
    
    /* Format each byte to 8 char hexadecimal representation */
    ss << setw(8) << setfill('0');   
    
    /* DEVICE_UID_ADDR contains the address pointing to the 96-bit factory flashed UID
    this macro is configured for each target in mbed_app.json */
    ss << *(uint32_t*) (DEVICE_UID_ADDR);           // bits  0 - 31 (offset 0x00)
    ss << *(uint32_t*) (DEVICE_UID_ADDR + 0x04);    // bits 32 - 63 (offset 0x04)
    ss << *(uint32_t*) (DEVICE_UID_ADDR + 0x08);    // bits 64 - 95 (offset 0x08)
    
    return ss.str();
}

/** @}*/