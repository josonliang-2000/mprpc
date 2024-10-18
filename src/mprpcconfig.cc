#include "mprpcconfig.h"
#include <iostream>
#include <string>

// 负责解析加载配置文件
void MprpcConfig::LoadConfigFile(const char *config_file)
{
    FILE *pf = fopen(config_file, "r");
    if (nullptr == pf) {
        std::cout << config_file << " is not exist!" << std::endl;
        exit(EXIT_FAILURE);
    } 

    // 1.注释 2.正确的配置项= 3.去掉开头多余的空格
    while(!feof(pf)) {
        char buf[512] = {0};
        fgets(buf, 512, pf); // 读取一行

        std::string rdbuf(buf);
        Trim(rdbuf);

        // 判断#的注释
        if (rdbuf.empty() || rdbuf[0] == '#') {
            continue;
        }

        // 解析配置项
        int idx = rdbuf.find('=');
        if (idx == std::string::npos) {
            // 配置项不合法
            continue;
        }

        // eg: rpcserverip=127.0.0.1\n
        std::string key(rdbuf.substr(0, idx));
        Trim(key);
        std::string value(rdbuf.substr(idx + 1));
        Trim(value);
        
        m_configMap[key] = value;
    }
}
// 查询配置项信息
std::string MprpcConfig::Load(const std::string &key)
{
    auto it = m_configMap.find(key);
    if (it == m_configMap.end()) {
        std::cout << "Cannot find " << key << std::endl;
        return " ";
    }
    return it->second;
}

void MprpcConfig::Trim(std::string &str) 
{
    // 去掉字符串前面多余的空格
    int idx = str.find_first_not_of(' ');
    if (idx != std::string::npos) {
        str = str.substr(idx);
    }

    // 去掉空格后的换行符
    if (str.back() == '\n') str.pop_back();

    // 去掉字符串后面多余的空格
    idx = str.find_last_not_of(' ');
    if (idx != std::string::npos) {
        str = str.substr(0, idx + 1);
    }

    // 去掉尾部的换行符
    if (str.back() == '\n') str.pop_back();
}