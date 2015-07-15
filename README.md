# MQTT Server on reactor mode

simple reactor base on c++0x11 and accept4
use log4cplus-1.1.2, use static lib link. Can see include/reactor/define.hpp and move logger define from main.cpp

-----------------------------------------
#define _HAS_LOG4CPLUSH_LOG_                                                                        
                                                                                                   
#ifdef _HAS_LOG4CPLUSH_LOG_                                                                         
     #include "common/my_logger.h"                                                                   
#elif                                                                                               
     int my_printf(const char *fmt, ...);                                                            
     #define LOG_DEBUG my_printf                                                                         
     #define LOG_WARN  my_printf                                                                         
     #define LOG_ERROR my_printf                                                                         
     #define LOG_TRACE_METHORD my_printf                                                                 
#endif
-------------------------------------------



