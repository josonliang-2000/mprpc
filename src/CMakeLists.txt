
# aux_source_directory(. SRC_LIST)
set(SRC_LIST 
    mprpcapplication.cc 
    mprpcconfig.cc 
    rpcheader.pb.cc 
    rpcprovider.cc 
    mprpcchannel.cc 
    mprpccontroller.cc
    logger.cc
    zookeeperutil.cc)
# 生成框架的名为mprpc的静态库（因为muduo提供的是静态库，动态库要自己另外编）
add_library(mprpc ${SRC_LIST})
target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt)