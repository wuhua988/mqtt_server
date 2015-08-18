#include "common/str_tools.hpp"
#include <sstream>

namespace str_tools
{
    void split(std::string str, std::string separator, int max, std::vector<std::string>* results)
    {
        int i = 0;
        size_t found = str.find_first_of(separator);
        
        while (found != std::string::npos)
        {
            if (found > 0)
            {
                results->push_back(str.substr(0, found));
            }
            
            str = str.substr(found + 1);
            found = str.find_first_of(separator);
            
            if (max > -1 && ++i == max) break;
        }
        
        if (str.length() > 0) {
            results->push_back(str);
        }
    }
    
    /* isspace()
     ' '	(0x20)	space (SPC)
     '\t'	(0x09)	horizontal tab (TAB)
     '\n'	(0x0a)	newline (LF)
     '\v'	(0x0b)	vertical tab (VT)
     '\f'	(0x0c)	feed (FF)
     '\r'	(0x0d)	carriage return (CR)
     */
    std::string & trim(std::string &s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(isspace))));
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(isspace))).base(), s.end());
        
        return s;
    }
    
    void remove_space(std::string& str)
    {
        std::string buff(str);
        char space = ' ';
        str.assign(buff.begin() + buff.find_first_not_of(space),
                   buff.begin() + buff.find_last_not_of(space) + 1);
    }
    
    template <class T>
    std::string to_string(const T& t)
    {
        std::ostringstream oss;
        oss << t;
        return oss.str();
    }
    
    template <class out_type,class in_value>
    out_type convert(const in_value & t)
    {
        std::stringstream stream;
        stream << t;
        out_type result;
        stream >> result;    //向result中写入值
        return result;
    }
}
