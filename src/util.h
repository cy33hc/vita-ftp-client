#ifndef UTIL_H
#define UTIL_H

#include <vitasdk.h>
#include <string>
#include <vector>
#include <algorithm>

namespace Util {

    static inline std::string& Ltrim(std::string& str, std::string chars)
    {
        str.erase(0, str.find_first_not_of(chars));
        return str;
    }

    static inline std::string& Rtrim(std::string& str, std::string chars)
    {
        str.erase(str.find_last_not_of(chars) + 1);
        return str;
    }

    // trim from both ends (in place)
    static inline std::string& Trim(std::string& str, std::string chars)
    {
        return Ltrim(Rtrim(str, chars), chars);
    }

    static inline void ReplaceAll(std::string & data, std::string toSearch, std::string replaceStr)
    {
        size_t pos = data.find(toSearch);
        while( pos != std::string::npos)
        {
            data.replace(pos, toSearch.size(), replaceStr);
            pos = data.find(toSearch, pos + replaceStr.size());
        }
    }

    static inline std::string ToLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), 
                [](unsigned char c){ return std::tolower(c); });
        return s;
    }

    static inline void convertUtcToLocalTime(SceDateTime *time_local, DateTime *time_utc) {
        // sceRtcGetTick and other sceRtc functions fails with year > 9999
        SceDateTime sce_utc;
        sce_utc.year = time_utc->year;
        sce_utc.month = time_utc->month;
        sce_utc.day = time_utc->day;
        sce_utc.hour = time_utc->hours;
        sce_utc.minute = time_utc->minutes;
        sce_utc.second = time_utc->seconds;
        sce_utc.microsecond = time_utc->microsecond;

        int year_utc = time_utc->year;
        int year_delta = year_utc < 9999 ? 0 : year_utc - 9998;
        time_utc->year -= year_delta;

        SceRtcTick tick;
        sceRtcGetTick(&sce_utc, &tick);
        time_utc->year = year_utc;

        sceRtcConvertUtcToLocalTime(&tick, &tick);
        sceRtcSetTick(time_local, &tick);
        time_local->year += year_delta;
    }
}
#endif
