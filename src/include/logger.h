#pragma once
#include "lockqueue.h"
#include <string>

enum LogLevel {
    INFO, // 普通信息
    ERROR, // 错误信息
};

//Mprpc框架提供的日志系统
class Logger {
public:
    static Logger& GetInstance();
    void SetLoglevel(LogLevel level);
    void Log(const std::string &msg);
private:
    int m_loglevel; // 记录日志级别
    LockQueue<std::string> m_lckQue; // 日志缓冲队列

    Logger();
    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;
};

// 定义宏供用户使用 ("xxx %d %s", 20, "xxxx"...)
#define LOG_INFO(logmsgformat, ...) \
    do \
    {  \
        Logger &logger = Logger::GetInstance(); \
        logger.SetLoglevel(INFO); \
        char c[1024] = {0}; \
        snprintf(c, sizeof(c), logmsgformat, ##__VA_ARGS__); \
        logger.Log(c); \
    } while (0); 

#define LOG_ERR(logmsgformat, ...) \
    do \
    {  \
        Logger &logger = Logger::GetInstance(); \
        logger.SetLoglevel(ERROR); \
        char c[1024] = {0}; \
        snprintf(c, sizeof(c), logmsgformat, ##__VA_ARGS__); \
        logger.Log(c); \
    } while (0);