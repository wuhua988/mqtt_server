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
}

#endif

