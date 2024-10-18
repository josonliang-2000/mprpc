#include "logger.h"
#include <time.h>
#include <iostream>

Logger &Logger::GetInstance()
{
    static Logger logger;
    return logger;
}

void Logger::SetLoglevel(LogLevel level)
{
    m_loglevel = level;
}

// 写日志，把日志信息写入lockqueue
void Logger::Log(const std::string &msg)
{
    m_lckQue.push(msg);
}

Logger::Logger()
{
    // 启动专门的写日志线程
    std::thread writeLogTask([&]() {
        for (;;) {
            // 获取当前的日期，然后取日志信息，写入相应的日志文件当中 a+
            time_t now = time(nullptr); //表示自1970年1月1日以来的秒数（Unix 时间戳）
            tm *nowtm = localtime(&now); // 解析

            char file_name[128];
            sprintf(file_name, "%d-%d-%d.txt", nowtm->tm_year + 1900, nowtm->tm_mon + 1, nowtm->tm_mday);

            FILE *pf = fopen(file_name, "a+");
            if (pf == nullptr) {
                std::cout << "logger file :" << file_name << " open error!" << std::endl;
                exit(EXIT_FAILURE);
            }

            std::string msg = m_lckQue.pop();
            char time_buf[128] = {0};
            sprintf(time_buf, "[%d:%d:%d]: ", nowtm->tm_hour, nowtm->tm_min, nowtm->tm_sec);
            msg.insert(0, time_buf);
            msg.append("\n");
            fputs(msg.c_str(), pf);

            // 如果队列不为空才关闭文件描述符
            // if (m_lckQue.isEmpty()) {
                fclose(pf);
            // }
        }
    });
    // 设置分离线程，守护线程
    writeLogTask.detach();
}
