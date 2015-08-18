#ifndef _str_tools_common_h__
#define _str_tools_common_h__

#include <iostream>
#include <algorithm>
#include <vector>
#include <iterator>

namespace str_tools
{
    void split(std::string str, std::string separator, int max, std::vector<std::string>* results);
    std::string & trim(std::string &str);
    void remove_space(std::string& str);
    
    template <class T> std::string to_string(const T& t);
    template <class out_type,class in_value> out_type convert(const in_value & t);
    
    // IS0 8601 time format
    std::string format_time(uint32_t time);
}



#endif

