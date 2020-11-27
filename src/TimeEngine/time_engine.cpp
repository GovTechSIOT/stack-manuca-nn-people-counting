/**
 * @defgroup time_engine Time Engine
 * @{
 */

#include "time_engine.h"
#include "mbed_trace.h"
#include "conversions.h"

#define TRACE_GROUP  "TimeEngine"

/**
 *  @brief  Formats the time component by appending a leading zero if the input string has less than two characters.
 *  @author Lau Lee Hong
 *  @param  time_component Month, day, hour, min, sec
 *  @return Minimally two-digit formatted string
 */
std::string FormatTime(std::string time_component)
{
    if (time_component.length() < 2)
    {
        time_component = "0" + time_component;   
    }
    return time_component;
    
}

/**
 *  @brief  Updates the onboard Real-time Clock.
 *  @author Lau Lee Hong, Lee Tze Han
 *  @param  ntp Address of the Network Time Protocol client object
 *  @return success(1) / failure (0)
 */
bool UpdateRtc(NTPClient& ntp)
{
    time_t raw_time = ntp.get_timestamp();
    
    /* NTPClient returns a negative error code if unsuccessful */
    if (raw_time < 0)
    {
        tr_warn("NTP update unsuccessful (rc = %d)", raw_time);
        return false;
    }
    else
    {
        set_time(raw_time);
        return true;
    }
}

/**
 *  @brief  Current raw time (in seconds) with reference to the onboard Real-time Clock.
 *  @author Lau Lee Hong
 *  @return Seconds elapsed since 00:00:00 UTC, January 1, 1970
 */
int RawRtcTimeNow(void)
{
    time_t raw_rtc_time = time(NULL);
    
    return raw_rtc_time;
}

/**
 *  @brief  Converts raw time to ISO8601-formatted time.
 *  @author Lau Lee Hong
 *  @param raw_time Seconds elapsed since 00:00:00 UTC, January 1, 1970
 *  @return Current ISO8601-formatted time with reference to the onboard Real-time Clock
 */
std::string ConvertRawTimeToIso8601Time(time_t raw_time)
{
    struct tm *ptm;
        
    ptm = localtime(&raw_time);
    std::string year = IntToString(ptm->tm_year + 1900);
    std::string month = IntToString(ptm->tm_mon + 1);
    month = FormatTime(month);
    std::string day = IntToString(ptm->tm_mday);
    day = FormatTime(day);
    std::string hour = IntToString(ptm->tm_hour);
    hour = FormatTime(hour);
    std::string min = IntToString(ptm->tm_min);
    min = FormatTime(min);
    std::string sec = IntToString(ptm->tm_sec);
    sec = FormatTime(sec);

    std::string iso8601_timestamp_now = year + "-" + month + "-" + day + 'T' + hour + ":" + min + ":" + sec + "Z";
    
    return iso8601_timestamp_now;
}

/** @}*/