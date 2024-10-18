#include "zookeeperutil.h"
#include "mprpcapplication.h"
#include <iostream>

// 全局的watcher线程回调函数
void global_watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx) {

}


ZkClient::ZkClient() : m_zhandle(nullptr)
{
}

ZkClient::~ZkClient()
{
    if (m_zhandle != nullptr) {
        zookeeper_close(m_zhandle);
    }
}

void ZkClient::Start()
{

}

void ZkClient::Create(const char *path, const char *data, int datalen, int state)
{
}

std::string ZkClient::GetData(const char *path)
{
    return std::string();
}
