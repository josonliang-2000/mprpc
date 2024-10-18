#include "rpcprovider.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"



/*
service_name => serviceDescriptor* => service* => method_name => method对象 


*/
// 这里是框架提供给外部使用的，可以发布rpc方法的api
// 其实就是存好服务的方法及其索引
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;

    // 获取了服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pserveiceDesc = service->GetDescriptor();
    // 获取服务的名字
    std::string service_name = pserveiceDesc->name();
    // 获取服务对象service的方法的数量
    int methodCnt = pserveiceDesc->method_count();
    
    std::cout << "service_name:" << service_name << std::endl;
    for (int i = 0; i < methodCnt; ++i) {
        // 获取服务对象指定下表的服务方法描述()
        const google::protobuf::MethodDescriptor* pmethodDesc = pserveiceDesc->method(i);  
        std::string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name, pmethodDesc});

        std::cout << "method_name:" << method_name << std::endl;
    }
    service_info.m_service = service;
    m_serviceMap.insert({service_name, service_info});
     
}

// 启动rpc服务节点，开始提供rpc远程网络调用服务
void RpcProvider::Run()
{
    // 加载配置文件
    std::string ip = MprpcApplication::GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip, port);

    // 创建TcpServer对象
    muduo::net::TcpServer server(&m_eventLoop, address, "RpcProvider"); 
    // 绑定连接回调和消息读写回调方法，分离了网络代码和业务代码
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1,
            std::placeholders::_2, std::placeholders::_3));

    // 设置muduo库的线程数量
    server.setThreadNum(4);

    // log
    std::cout << "RpcProvider start service at ip:" << ip << " port:" << port << std::endl;

    // 启动网络服务
    server.start();
    m_eventLoop.loop();
}

// 连接状态发生变化时的回调（连接、断开等）
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn) 
{
    if (!conn->connected()) {
        // 和rpc client的连接断开了
        conn->shutdown(); //优雅关闭
    }


}


/*
在框架内部，RpcProvider和RpcConsumer协商好之间通信用的protobuf数据类型
service_name method_name args   定义proto的message类型，进行数据的序列化和反序列化
16UserServiceLoginzhangsan123456

定义的数据格式
header_size（4字节） + rpcheader + content
其中的rpcheader格式：
message RpcHeader
{
    bytes service_name = 1;
    bytes method_name = 2;
    uint32 args_size = 3;
}
std::string insert和copy方法
*/
// 已建立连接的用户读写事件回调：完成请求的反序列化
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn,  
                            muduo::net::Buffer *buffer, 
                            muduo::Timestamp) 
{
    // 网络上接收的远程rpc调用请求的字符流 Login args
    std::string recv_buf = buffer->retrieveAllAsString();

    // 从字符流中读取前4个字节的内容
    uint32_t header_size = 0;
    recv_buf.copy((char*)&header_size, 4, 0);

    // 根据header_size读取数据头的原始字符流
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if (rpcHeader.ParseFromString(rpc_header_str)) {
        // 数据反序列化成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();

    } else {
        // 数据头反序列化失败
        std::cout << "rpc_header_str:" << rpc_header_str << " parse error!" << std::endl;
        return;
    }

    // 获取rpc方法参数的字符流数据
    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    // 调试信息
    std::cout << "=======================receive rpcrequest=====================" << std::endl;
    std::cout << "head_size:" << header_size << std::endl;
    std::cout << "rpc_header_str:" << rpc_header_str << std::endl;
    std::cout << "service_name:" << service_name << std::endl;
    std::cout << "method_name:" << method_name << std::endl;
    std::cout << "args_str:" << args_str << std::endl;
    std::cout << "===============================================================" << std::endl;

    // 获取service对象和method对象
    auto it = m_serviceMap.find(service_name); // <服务名，ServiceInfo>
    if (it == m_serviceMap.end()) {
        std::cout << service_name << " is not exist!" << std::endl;
        return;
    }

    auto mit = it->second.m_methodMap.find(method_name); // <方法名，MethodDescriptor*>
    if (mit == it->second.m_methodMap.end()) {
        std::cout << method_name << " is not exist!" << std::endl;
        return;
    }

    google::protobuf::Service *service = it->second.m_service; // 获取service对象
    const google::protobuf::MethodDescriptor *method = mit->second; // 获取method对象

    // 生成rpc方法调用的请求request和响应response参数
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str)) {
        std::cout << "request parse error, content:" << args_str << std::endl;
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    // 给下面的method方法的调用，绑定一个Closure的回调函数
    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider, 
                                                                    const muduo::net::TcpConnectionPtr&, 
                                                                    google::protobuf::Message*>
                                                                    (this, &RpcProvider::SendRpcResponse, conn, response);

    // 在框架上根据远端rpc请求，调用当前rpc节点上发布的方法
    // new UserService().Login(controller, request, response, done)
    service->CallMethod(method, nullptr, request, response, done);    
}

void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message *response)
{
    std::string response_str;
    if (response->SerializeToString(&response_str)) { // response进行序列化
        conn->send(response_str);
        std::cout << "response after serializing: " << response_str << std::endl;
    } else {
        std::cout << "serialize response_str error!" << std::endl;
    }

    conn->shutdown(); // 模拟http的短连接服务，由rpcprovider主动断开连接
}