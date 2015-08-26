//
//  timer_file.cpp
//  mqtt_server
//
//  Created by davad.di on 8/25/15.
//
//

#include "common/timer_file.hpp"

struct tm time_to_tm(time_t t)
{
#ifdef WIN32
#if _MSC_VER < 1400 //VS2003
    return *localtime(&t);
#else //vs2005->vs2013->
    struct tm tt = { 0 };
    localtime_s(&tt, &t);
    return tt;
#endif
#else //linux
    struct tm tt;
    memset(&tt, 0, sizeof(tt));
    localtime_r(&t, &tt);
    return tt;
#endif
}

bool is_same_hour(time_t t1, time_t t2)
{
    tm tm1 = time_to_tm(t1);
    tm tm2 = time_to_tm(t2);
    if ( tm1.tm_year == tm2.tm_year
        && tm1.tm_yday == tm2.tm_yday
        && tm1.tm_hour == tm2.tm_hour)
    {
        return true;
    }
    
    return false;
}



CTimerFileInfo::CTimerFileInfo()
{
    m_file_prefix = "rec";
    m_file_suffix = "log";
    m_file_dir    = "./";
    m_file_max_line = 100000; // 10w
    m_file_max_size = 10*1024*1024; // 10M
    m_file_start_seq = 0;
}

void CTimerFileInfo::file_prefix(const char *prefix)
{
    m_file_prefix = prefix;
}

void CTimerFileInfo::file_suffix(const char *suffix)
{
    m_file_suffix = suffix;
}

void CTimerFileInfo::file_dir(const char *dir)
{
    m_file_dir = dir;
}

std::string CTimerFileInfo::format_date(std::time_t tm_value)
{
    char date_buf[256];
    struct tm tm_time = time_to_tm(tm_value);
    
    // dir/prefix_year_month_day_hour_seq.suffix
    snprintf(date_buf, 256, "%04d_%02d_%02d_%02d_%02d_%02d",
             tm_time.tm_year + 1900,
             tm_time.tm_mon + 1,
             tm_time.tm_mday,
             tm_time.tm_hour,
             tm_time.tm_min,
             tm_time.tm_sec
             );
    
    std::string str = date_buf;
    
    return str;
}

bool CTimerFileInfo::check_tm(std::time_t t1, std::time_t t2)
{
    // one hour
    return is_same_hour(t1,t2);
}


CTimerFile::CTimerFile(CTimerFileInfo &file_info)
: m_timer_file_info(file_info), m_cur_file_line(0), m_last_write_time(std::time(nullptr))
{
    m_file_seq = file_info.file_start_seq();
}

CTimerFile::~CTimerFile()
{
    if (m_file_stream != nullptr)
    {
        fflush(m_file_stream);
        ::fclose(m_file_stream);
        m_file_stream = nullptr;
    }
}


int CTimerFile::write_data(const void *data, uint32_t len)
{
    if (check_file() == -1)
    {
        return -1;
    }
    
    size_t res = fwrite(data, 1, len, m_file_stream);
    if (res != len)
    {
        LOG_DEBUG("Write to file failed. %d, %s", errno, strerror(errno));
        return -1;
    }
    
    fflush(m_file_stream);
    
    m_cur_file_line++;
    m_last_write_time = std::time(nullptr);
    
    return 0;
}

int CTimerFile::create_file()
{
    gen_file_name();
    m_file_stream = fopen(m_file_name.c_str(), "a"); // can see man fopen, know flag
    
    if (m_file_stream == nullptr)
    {
        LOG_ERROR("Open file [%s] failed. errno %d, %s", m_file_name.c_str(), errno, strerror(errno));
        return -1;
    }
    
    LOG_DEBUG("Open file [%s] succed.", m_file_name.c_str());
    
    return 0;
}

int CTimerFile::check_file()
{
    /* check file_ size, file line, timeout */
    if (m_file_stream == nullptr)
    {
        if (this->create_file() == -1)
        {
            return -1;
        }
    }
    
    bool create_file = false;
    
    do
    {
        // check file_line
        if (m_cur_file_line >= m_timer_file_info.file_max_line())
        {
            //LOG_DEBUG("Cur file line [%d] max_file_line [%d]",
            //                    m_cur_file_line, m_timer_file_info.file_max_line());
            
            create_file = true;
            break;
        }
        
        // check file size
        (void) fseek(m_file_stream, 0L, SEEK_END);
        long file_size = ftell(m_file_stream);
        if (file_size >= m_timer_file_info.file_max_size())
        {
            //LOG_DEBUG("file size [%d], file_max_size [%d]",
            //                    file_size, m_timer_file_info.file_max_size());
            
            create_file = true;
            break;
        }
        
        // check timeout  hour
        if (!m_timer_file_info.check_tm(std::time(nullptr), m_last_write_time)) // one hour
        {
            // LOG_DEBUG("Not same hour, change file now");
            create_file = true;
            break;
        }
        
    } while(0);
    
    if (create_file)
    {
        fflush(m_file_stream);
        fclose(m_file_stream);
        m_file_stream = nullptr;
        
        m_cur_file_line = 0;
        
        if (this->create_file() == -1)
        {
            return -1;
        }
    }
    
    return 0;
}

void CTimerFile::gen_file_name()
{
    
    char file_name[256];
    //struct tm cur_time = time_to_tm(std::time(nullptr));
    
    // dir/prefix_year_month_day_hour_seq.suffix
    snprintf(file_name, 256, "%s/%s_%s_%04d.%s",
             m_timer_file_info.file_dir().c_str(),
             m_timer_file_info.file_prefix().c_str(),
             m_timer_file_info.format_date().c_str(),
             m_file_seq++%10000,
             m_timer_file_info.file_suffix().c_str());
    
    m_file_name = file_name;
}

