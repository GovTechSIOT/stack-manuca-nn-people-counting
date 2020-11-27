/**
 * @defgroup boot_manager Boot Manager
 * @{
 */

#include "boot_manager.h"
#include "device_uid.h"
#include "global_params.h"
#include "persist_store.h"

RawSerial pc(USBTX, USBRX, 115200);

const uint8_t boot_timeout_sec = 5;    
const std::string sdk_ver = "2.1.0";
const uint8_t max_login_attempts = 3;
const std::string poll_rate_ms = "10000";
const std::string boot_login_pw = "stack2020";
const std::string uuid = GetDeviceUid();

std::string wifi_ssid = ReadWifiSsid();
std::string wifi_pass = ReadWifiPass();

bool reformat_SD = false;
std::string user_input = "";

/**
 *  @brief  Prints the SDK Header.
 *  @author Lau Lee Hong, Ng Tze Yang
 */
void PrintHeader(void)
{
    pc.printf("\r-----------------------------------------------------\r\n");
    pc.printf("\r  ____   ____   _____         _       ____  _             _    \r\n");
    pc.printf("\r / ___| / ___| |_   _|__  ___| |__   / ___|| |_ __ _  ___| | __\r\n");
    pc.printf("\r \\___ \\| |  _    | |/ _ \\/ __| '_ \\  \\___ \\| __/ _` |/ __| |/ /\r\n");
    pc.printf("\r  ___) | |_| |   | |  __/ (__| | | |  ___) | || (_| | (__|   < \r\n");
    pc.printf("\r |____/ \\____|   |_|\\___|\\___|_| |_| |____/ \\__\\__,_|\\___|_|\\_\\\r\n");
    pc.printf("\r\nsdk v%s \r\n\r\n", sdk_ver.c_str());
}

/**
 *  @brief  Prints the Bootmanager Menu
 *  @author Ng Tze Yang
 */
void PrintMenu(void)
{
    PrintHeader();

    /* Censor Wifi Password */
    std::string censored_wifi_pass = ReadWifiPass();
    censored_wifi_pass.replace(1, censored_wifi_pass.length()-2, censored_wifi_pass.length()-2, '*');
    
    pc.printf("\ruuid %s \r\n", uuid.c_str());
    pc.printf("\r-----------------------------------------------------\r\n");
    pc.printf("(1) WIFI SSID \t\t\t\t %s\r\n", ReadWifiSsid().c_str());
    pc.printf("\r-----------------------------------------------------\r\n");
    pc.printf("(2) WIFI Password: \t\t\t %s\r\n", censored_wifi_pass.c_str());
    pc.printf("\r-----------------------------------------------------\r\n");
    pc.printf("(3) Clear DECADA MQTT Certificate & Key\r\n");
    pc.printf("\r-----------------------------------------------------\r\n");
    pc.printf("(-1) Reset All to Defaults\r\n");
    pc.printf("\r-----------------------------------------------------\r\n");
    pc.printf("(-2) Save & Quit Bootmanager\r\n");
    pc.printf("\r-----------------------------------------------------\r\n");
}

/**
 *  @brief  Enter the Boot Manager.
 *  @author Lau Lee Hong
 *  @return Whether to enter boot manager (1) or not (0)
 */
bool EnterBootManager(void)
{
    bool boot = false;
    
    Timer t;
    t.start();
    while (1)
    {
        /* Launch BootManager*/
        if (pc.readable())
        {
            char c = pc.getc();
            boot = true;
            break;
        }
        /* Normal Operation */
        if (t.read() > boot_timeout_sec)
        {
            break;    
        }
    }
    t.stop();
    
    return boot;
}

/**
 *  @brief  Main bootloader logic. Configures MANUCA OS based on user inputs
 *  @author Lau Lee Hong, Ng Tze Yang
 */
void RunBootManager(void)
{
    if(ReadInitFlag() != "true")
    {
        SetDefaultConfig();
    }

    /* Boot Initialisation */
    WriteSwVer(sdk_ver);
    PrintHeader();
    BootManagerLogin();
    InitAfterLogin();

    /* Main Bootmanager Logic */
    while (true)
    {
        PrintMenu();
        user_input = GetUserInputString();
        /* Quit Bootmanager */
        if (user_input == "-2")
        {
            pc.printf("End of configuration. MANUCA OS will restart.\r\n\r\n");
            NVIC_SystemReset();
        }
        /* Reset Defaults */
        else if (user_input == "-1")
        {
            SetDefaultConfig();
        }
        else if (user_input == "1") 
        {
            pc.printf("Choose new WIFI SSID:\r\n");
            WriteWifiSsid(GetUserInputString());
        }
        else if (user_input == "2") 
        {
            pc.printf("Choose new WIFI Password:\r\n");
            WriteWifiPass(GetUserInputString(true));
        }
        else if (user_input == "3") 
        {
            WriteSSLPrivateKey("");
            WriteClientCertificate("");
            WriteClientCertificateSerialNumber("");
            pc.printf("DECADA MQTT Certificate & Key Cleared.\r\n");
        }
        else
        {
            pc.printf("Invalid choice \"%s\". Try again...\r\n",user_input.c_str());
        }
    }
}

/**
 *  @brief  Initalisation tasks after successful login.
 *  @author Lau Lee Hong, Ng Tze Yang
 */
void InitAfterLogin(void)
{
    WriteCycleInterval(poll_rate_ms);
    WriteInitFlag("true");
}

/**
 *  @brief  Login function to authenticate user.
 *  @author Lau Lee Hong, Ng Tze Yang
 */
void BootManagerLogin(void) 
{
    for (int i=max_login_attempts; i>0; i--)
    {
        pc.printf("Enter passphrase (%d attempts left): \r\n", i);
        user_input = GetUserInputString(true);
        if (user_input == boot_login_pw)
        {
            pc.printf("\r\nLogin successful.\r\n");
            return;
        }
        pc.printf("\r\nWrong Passphrase.\r\n");
    }
    pc.printf("\r\nOS Locked\r\n");
    while(1);
}

/**
 *  @brief  Obtains user inputs and returns it as a std::string.
 *  @author Lau Lee Hong, Ng Tze Yang
 *  @return std::string containing user input
 */
std::string GetUserInputString(bool is_hidden)
{
    std::string input = "";
    while (true)
    {
        char c = pc.getc();
        /* Do not show passwords typed on screen */
        if (is_hidden)
        {
            if (c == '\b') 
            {
                pc.printf("%c",c);
            }
            else
            {
                pc.printf("*");
            }
        }
        else
        {
            pc.printf("%c", c);
        }
        if (c == '\r') // User presses 'Enter'
        {
            return input;   
        }
        else if (c == '\b')    // Backspace      
        {
            input = input.substr(0, input.size()-1);
        }
        else
        {
            input.push_back(c);
        }
    }
}

/**
 *  @brief  Sets configuration parameters to default values
 *  @author Ng Tze Yang, Lau Lee Hong
 */
void SetDefaultConfig(void)
{
    WriteWifiSsid("WIFI_SSID");
    WriteWifiPass("WIFI_PW");
    WriteSSLPrivateKey("");
    WriteClientCertificate("");
    WriteClientCertificateSerialNumber("");
}

/**
 *  @brief  Reset the wireless module
 *  @author Goh Kok Boon
 */
void WirelessModuleReset(void)
{
    DigitalOut wifireset(PF_11);
    wifireset = 0;
    wait(1);
    wifireset = 1;
    wait(1);
}
/** @}*/
