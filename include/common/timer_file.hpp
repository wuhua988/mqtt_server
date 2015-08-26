//
//  timer_file.hpp
//  mqtt_server
//
//  Created by davad.di on 8/20/15.
//
//

#ifndef mqtt_server_timer_file_hpp
#define mqtt_server_timer_file_hpp

// #include <stdio.h> for fwrite

#include "reactor/define.hpp"
#include <time.h>

struct tm time_to_tm(time_t t);
bool is_same_hour(time_t t1, time_t t2);



class CTimerFileInfo
{
public:
    CTimerFileInfo();
    
    void file_prefix(const char *prefix);
    
    void file_suffix(const char *suffix);
    void file_dir(const char *dir);
    
    std::string format_date(std::time_t tm_value = std::time(nullptr));
    
    bool check_tm(std::time_t t1, std::time_t t2);
    
    CLS_VAR_REF(std::string, file_prefix);
    CLS_VAR_REF(std::string, file_suffix);
    CLS_VAR_REF(std::string, file_dir);
    
    CLS_VAR_NO_REF_CONST(uint32_t, file_max_size);
    CLS_VAR_NO_REF_CONST(uint32_t, file_max_line);
    CLS_VAR_NO_REF_CONST(uint32_t, file_max_timeout);
    
    CLS_VAR_NO_REF_CONST(uint32_t, file_start_seq);
};

// time base on hour
class CTimerFile
{
public:
    CTimerFile(CTimerFileInfo &file_info);
    
    ~CTimerFile();
    int write_data(const void *data, uint32_t len);
    int create_file();
    int check_file();
    void gen_file_name();
    
    
protected:
    CTimerFileInfo m_timer_file_info;
    
    // cur file line
    uint32_t m_cur_file_line;
    
    // cur_file_seq
    uint32_t m_file_seq;
    
    // last write time
    time_t m_last_write_time;
    
    std::string m_file_name;
    FILE *m_file_stream = nullptr;
};

#endif
