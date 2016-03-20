#include "log.h"
#include "inifile.h"
#include "tee.h"
#include <ctime>
#include <string.h>

log_level_type log_level_file = LOG_LEVEL_NULL;
log_level_type console_level_file = LOG_LEVEL_INFO;

std::ofstream file_log;
Tee tee(std::cout,file_log);

std::ostringstream strStream;
int file_log_size = 0;
int maxfilesize = 0;
int maxbackupindex = 1;
std::string fname;
logfun plogfun = NULL;
//==================================
//static char digits[] = "0123456789";
//
//inline char hiDigit(int i)
//{ return digits[i / 10]; }
//
//inline char loDigit(int i)
//{ return digits[i % 10]; }


void strtime(std::ostream& out)
{
    // current date/time based on current system
    time_t now = time(0);
    tm ltm;
    localtime_r(&now, &ltm);

    // print various components of tm structure.
    out<<1900 + ltm.tm_year<<"/"<<1 + ltm.tm_mon<<"/"<<ltm.tm_mday<<" ";
    out<<ltm.tm_hour << ":"<< 1 + ltm.tm_min << ":"<< 1 + ltm.tm_sec<<" ";
}

//==================================

bool log_init(log_level_type level)
{
    console_level_file = level;
    return true;
}


void log_setotherfun(logfun fun)
{
    plogfun = fun;
}


log_level_type conver(const char* str){

    char c = std::toupper(str[0]);
    return c == 'T' ? LOG_LEVEL_TRACE
        : c == 'D'  ? LOG_LEVEL_DEBUG
        : c == 'I'  ? LOG_LEVEL_INFO
        : c == 'W'  ? LOG_LEVEL_WARN
        : c == 'E'  ? LOG_LEVEL_ERROR
        : c == 'N'  ? LOG_LEVEL_NULL
        : LOG_LEVEL_FATAL;
}

void log_flushToFile(const char* content)
{
    if (file_log) {
        file_log<<content;
    }
}



bool log_init(const std::string& ininame,const std::string& documentPath)
{
    IniFile inifile(ininame);
    console_level_file = conver(inifile.getValue("root","std_level","I").c_str());
    log_level_file = conver(inifile.getValue("root","file_level","N").c_str());

    fname = inifile.getValue("root","filename","log.log");
    maxfilesize = inifile.getValueT("root","maxfilesize",1024);
    maxbackupindex = inifile.getValueT("root","maxbackupindex",3);
    if(log_level_file!=LOG_LEVEL_NULL)
    {
        file_log.clear();
        file_log.close();
        std::string filePath = documentPath + "/" + fname;
        file_log.open(filePath.c_str(), std::ios::app);
    }
    return true;
}

std::ostream* getlevellog(log_level_type level)
{
    if ((log_level_file >= level) && (console_level_file >= level))
        return &strStream;
    if (log_level_file >= level)
        return &strStream;
    if (console_level_file >= level)
        return &strStream;
    return NULL;
}

log_level_type str2loglevel(const char* slogtype)
{
    if(strcmp(slogtype, "TRACE") == 0)
        return LOG_LEVEL_TRACE;
    if(strcmp(slogtype, "DEBUG") == 0)
        return LOG_LEVEL_DEBUG;
    if(strcmp(slogtype, "INFO") == 0)
        return LOG_LEVEL_INFO;
    if(strcmp(slogtype, "WARN") == 0)
        return LOG_LEVEL_WARN;
    if(strcmp(slogtype, "ERROR") == 0)
        return LOG_LEVEL_ERROR;
    return LOG_LEVEL_FATAL;
}


//==========================================================
LogMessage::LogMessage(std::ostream& logger, const char* level)
: logger_(logger), level_(level)
{
    strtime(logger);
    logger<<"["<<level<<"] ";
}

LogMessage::LogMessage(std::ostream& logger, log_level_type level)
: logger_(logger),level_(
                          level >= ::LOG_LEVEL_TRACE ? "TRACE"
                          : level >= ::LOG_LEVEL_DEBUG ? "DEBUG"
                          : level >= ::LOG_LEVEL_INFO ? "INFO"
                          : level >= ::LOG_LEVEL_WARN ? "WARN"
                          : level >= ::LOG_LEVEL_ERROR ? "ERROR"
                          : "FATAL")
{
    strtime(logger);
    logger<<"["<<level<<"] ";
}

void LogMessage::flush()
{
    logger_<<"\r\n";//std::endl; //修改支持windows系统换行格式 - by 麦
    //logger_.flush();
    std::string str = strStream.str();
    strStream.str("");
    log_level_type level = str2loglevel(level_);
    if ((log_level_file >= level) && (console_level_file >= level))
    {
        file_log<<str;
        file_log.flush();
        std::cout<<str;
    }
    else if (log_level_file >= level)
    {
        file_log<<str;
        file_log.flush();
    }
    else if (console_level_file >= level)
    {
        std::cout<<str;
    }
    if (plogfun) {
        plogfun(str.c_str());
    }
}

LogMessage::~LogMessage()
{
}

std::ostream& LogMessage::out()
{ return logger_; }

//========================================
