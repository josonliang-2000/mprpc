set(SRC_LIST callfriendservice.cc ../friend.pb.cc)
add_executable(consumer ${SRC_LIST})
# 链接框架的动态库
target_link_libraries(consumer mprpc protobuf)