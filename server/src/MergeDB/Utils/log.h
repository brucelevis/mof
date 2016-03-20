#ifndef __LOG_H__
#define __LOG_H__

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

enum log_level_type {
    LOG_LEVEL_NULL = 0,
    LOG_LEVEL_FATAL = 1<<0,
    LOG_LEVEL_ERROR = 1<<1,
    LOG_LEVEL_WARN  = 1<<2,
    LOG_LEVEL_INFO  = 1<<3,
    LOG_LEVEL_DEBUG = 1<<4,
    LOG_LEVEL_TRACE = 1<<5
};

//日志初始化
bool log_init(const std::string& ininame="log.ini", const std::string& documentPath="");
bool log_init(log_level_type level);
typedef void (*logfun)(const char*);
void log_setotherfun(logfun fun);
void log_flushToFile(const char* content);


class LogMessage
{
    std::ostream& logger_;
    const char* level_;
public:
    LogMessage(std::ostream& logger, const char* level);
    LogMessage(std::ostream& logger, log_level_type level);
    ~LogMessage();

    std::ostream& out();
    void flush();
};

std::ostream* getlevellog(log_level_type level);

#define _log_log(level, expr)   \
    do { \
    std::ostream* logger = getlevellog(LOG_LEVEL_ ## level);\
    if (logger) \
    { \
        ::LogMessage __logMessage(*logger, #level); \
        __logMessage.out() << expr; \
        __logMessage.flush(); \
    } \
    } while (false)

//日志输出
#define log_fatal(expr)     _log_log(FATAL, expr)
#define log_error(expr)     _log_log(ERROR, expr)
#define log_warn(expr)      _log_log(WARN, expr)
#define log_info(expr)      _log_log(INFO, expr)
#define log_debug(expr)     _log_log(DEBUG, expr)

#endif // __LOG_H__
