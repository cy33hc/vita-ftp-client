#ifndef UTIL_H
#define UTIL_H

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

}
#endif
