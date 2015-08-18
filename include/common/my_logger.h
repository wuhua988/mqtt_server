#ifdef _HAS_LOG4CPLUSH_LOG_

#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/loggingmacros.h>



// redefine macros, make them both supoort orgianl and vars
// LOG_DEBUG("Its a test\n"); or LOG_DEBUG("Its a test %s", "hello");

#if defined (LOG4CPLUS_HAVE_C99_VARIADIC_MACROS)
#define LOG_MACRO_FMT_BODY(logger, logLevel, logFmt, ...)         \
LOG4CPLUS_SUPPRESS_DOWHILE_WARNING()                                \
do {                                                                \
log4cplus::Logger const & _l                                    \
= log4cplus::detail::macros_get_logger (logger);            \
if (LOG4CPLUS_MACRO_LOGLEVEL_PRED (                             \
_l.isEnabledFor (log4cplus::logLevel), logLevel)) {     \
LOG4CPLUS_MACRO_INSTANTIATE_SNPRINTF_BUF (_snpbuf);         \
log4cplus::tchar const * _logEvent                          \
= _snpbuf.print (logFmt, ##__VA_ARGS__);                  \
log4cplus::detail::macro_forced_log (_l,                    \
log4cplus::logLevel, _logEvent,                         \
__FILE__, __LINE__, LOG4CPLUS_MACRO_FUNCTION ());       \
}                                                               \
} while(0)                                                          \
LOG4CPLUS_RESTORE_DOWHILE_WARNING()

#elif defined (LOG4CPLUS_HAVE_GNU_VARIADIC_MACROS)
#define LOG_MACRO_FMT_BODY(logger, logLevel, logFmt, logArgs...)  \
LOG4CPLUS_SUPPRESS_DOWHILE_WARNING()                                \
do {                                                                \
log4cplus::Logger const & _l                                    \
= log4cplus::detail::macros_get_logger (logger);            \
if (LOG4CPLUS_MACRO_LOGLEVEL_PRED (                             \
_l.isEnabledFor (log4cplus::logLevel), logLevel)) {     \
LOG4CPLUS_MACRO_INSTANTIATE_SNPRINTF_BUF (_snpbuf);         \
log4cplus::tchar const * _logEvent                          \
= _snpbuf.print (logFmt, ##logArgs);                      \
log4cplus::detail::macro_forced_log (_l,                    \
log4cplus::logLevel, _logEvent,                         \
__FILE__, __LINE__, LOG4CPLUS_MACRO_FUNCTION ());       \
}                                                               \
} while(0)                                                          \
LOG4CPLUS_RESTORE_DOWHILE_WARNING()

#endif



#if defined (LOG4CPLUS_HAVE_C99_VARIADIC_MACROS)
#define LOG_DEBUG(logFmt, ...)                            \
LOG_MACRO_FMT_BODY (log4cplus::Logger::getRoot(), DEBUG_LOG_LEVEL, logFmt, ##__VA_ARGS__)
#elif defined (LOG4CPLUS_HAVE_GNU_VARIADIC_MACROS)
#define LOG_DEBUG(logFmt, logArgs...)          \
LOG_MACRO_FMT_BODY(log4cplus::Logger::getRoot(), DEBUG_LOG_LEVEL, logFmt, ##logArgs)
#endif

#if defined (LOG4CPLUS_HAVE_C99_VARIADIC_MACROS)
#define LOG_INFO(logFmt, ...)                             \
LOG_MACRO_FMT_BODY (log4cplus::Logger::getRoot(), INFO_LOG_LEVEL, logFmt, ##__VA_ARGS__)
#elif defined (LOG4CPLUS_HAVE_GNU_VARIADIC_MACROS)
#define LOG_INFO(logFmt, logArgs...)                      \
LOG_MACRO_FMT_BODY(log4cplus::Logger::getRoot(), INFO_LOG_LEVEL, logFmt, ##logArgs)
#endif

#if defined (LOG4CPLUS_HAVE_C99_VARIADIC_MACROS)
#define LOG_WARN(logFmt, ...)                             \
LOG_MACRO_FMT_BODY (log4cplus::Logger::getRoot(), WARN_LOG_LEVEL, logFmt, ##__VA_ARGS__)
#elif defined (LOG4CPLUS_HAVE_GNU_VARIADIC_MACROS)
#define LOG_WARN(logFmt, logArgs...)                      \
LOG_MACRO_FMT_BODY(log4cplus::Logger::getRoot(), WARN_LOG_LEVEL, logFmt, ##logArgs)
#endif


#if defined (LOG4CPLUS_HAVE_C99_VARIADIC_MACROS)
#define LOG_ERROR(logFmt, ...)                            \
LOG_MACRO_FMT_BODY (log4cplus::Logger::getRoot(), ERROR_LOG_LEVEL, logFmt, ##__VA_ARGS__)
#elif defined (LOG4CPLUS_HAVE_GNU_VARIADIC_MACROS)
#define LOG_ERROR(logFmt, logArgs...)                     \
LOG_MACRO_FMT_BODY(log4cplus::Logger::getRoot(), ERROR_LOG_LEVEL, logFmt, ##logArgs)
#endif

#if defined (LOG4CPLUS_HAVE_C99_VARIADIC_MACROS)
#define LOG_FATAL(logFmt, ...)                            \
LOG_MACRO_FMT_BODY (log4cplus::Logger::getRoot(), FATAL_LOG_LEVEL, logFmt, ##__VA_ARGS__)
#elif defined (LOG4CPLUS_HAVE_GNU_VARIADIC_MACROS)
#define LOG_FATAL(logFmt, logArgs...)                     \
LOG_MACRO_FMT_BODY(log4cplus::Logger::getRoot(), FATAL_LOG_LEVEL, logFmt, ##logArgs)
#endif

#define LOG_TRACE_METHOD(logEvent)                        \
log4cplus::TraceLogger _log4cplus_trace_logger(log4cplus::Logger::getRoot(), logEvent,    \
__FILE__, __LINE__); \

#if defined (LOG4CPLUS_HAVE_C99_VARIADIC_MACROS)
#define LOG_TRACE(logFmt, ...)                            \
LOG_MACRO_FMT_BODY (log4cplus::Logger::getRoot(), TRACE_LOG_LEVEL, logFmt,##__VA_ARGS__)
#elif defined (LOG4CPLUS_HAVE_GNU_VARIADIC_MACROS)
#define LOG_TRACE(logFmt, logArgs...)                     \
LOG_MACRO_FMT_BODY(log4cplus::Logger::getRoot(), TRACE_LOG_LEVEL, logFmt, ##logArgs)
#endif


using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;

class CLoggerMgr
{
public:
    
    CLoggerMgr(const char *confFileName)
    {
        log4cplus::initialize (); // important
        // LogLog::getLogLog()->setInternalDebugging(true);
        open(confFileName);
    }
    
    int open(const char *confFileName)
    {
        try
        {
            PropertyConfigurator::doConfigure(confFileName);
            return 0;
        }
        catch(...)
        {
            tcout << LOG4CPLUS_TEXT("Exception...") << endl;
            return -1;
        }
    }
};
#define LOGGER(confFileName) \
CLoggerMgr logger_mgr(confFileName);

#else

#define LOG_DEBUG(x)
#define LOG_INFO(x)
#define LOG_WARN(x)
#define LOG_ERROR(x)
#define LOG_FATAL(x)

#define LOGGER(confFileName)
#endif
