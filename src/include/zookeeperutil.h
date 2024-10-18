#pragma once

#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>

class ZkClient {
public:
    ZkClient();
    ~ZkClient();
    // zkClient启动连接server
    void Start();
    // 在zkServer上根据指定的<路径，数据，是否永久节点>创建znode
    void Create(const char *path, const char *data, int datalen, int state = 0);
    // 根据路径索引节点数据
    std::string GetData(const char *path);
private:
    // zk的客户端句柄
    zhandle_t *m_zhandle;
};

