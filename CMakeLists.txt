cmake_minimum_required(VERSION 3.15)
# 设置vcpkg的工具链文件（如果使用vcpkg）
set(CMAKE_TOOLCHAIN_FILE "D:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

project(BC_Server)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 自动添加CMAKE_CURRENT_SOURCE_DIR到每个源文件
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 添加源文件
set(GATE_SERVER_SOURCES
    GateServer/GateServer.cpp
    GateServer/CServer.cpp
    GateServer/HttpConnection.cpp
    GateServer/LogicSystem.cpp
    GateServer/message.grpc.pb.cc
    GateServer/message.pb.cc
    GateServer/VerifyGrpcClient.cpp
    GateServer/ConfigMgr.cpp
    GateServer/AsioIOServicePool.cpp
)

# 添加头文件
set(GATE_SERVER_HEADERS
    GateServer/CServer.h
    GateServer/const.h
    GateServer/HttpConnection.h
    GateServer/Singleton.h
    GateServer/LogicSystem.h
    GateServer/message.grpc.pb.h
    GateServer/message.pb.h
    GateServer/VerifyGrpcClient.h
    GateServer/ConfigMgr.h
    GateServer/AsioIOServicePool.h
)

# 创建可执行文件
add_executable(GateServer ${GATE_SERVER_SOURCES} ${GATE_SERVER_HEADERS})

# 设置UTF-8编码
if(MSVC)
    target_compile_options(GateServer PRIVATE "/utf-8")
endif()

# 查找Boost库和组件
find_package(Boost REQUIRED COMPONENTS system filesystem)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(GateServer PRIVATE ${Boost_LIBRARIES})

# 查找gRPC和Protobuf
find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
target_link_libraries(GateServer PRIVATE gRPC::grpc++ gRPC::grpc++_reflection protobuf::libprotobuf)

# 添加jsoncpp库
set(JSONCPP_INCLUDE_DIR "D:/jsoncpp-0.y.z/include")
set(JSONCPP_LIBRARY_RELEASE "D:/jsoncpp-0.y.z/makefiles/vs71/x64/libjson/json_vc71_libmt.lib")
set(JSONCPP_LIBRARY_DEBUG "D:/jsoncpp-0.y.z/makefiles/vs71/x64/libjson/json_vc71_libmtd.lib")

include_directories(${JSONCPP_INCLUDE_DIR})
target_link_libraries(GateServer PRIVATE 
    debug ${JSONCPP_LIBRARY_DEBUG} 
    optimized ${JSONCPP_LIBRARY_RELEASE}
)

# 配置文件复制到输出目录
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/GateServer/config.ini ${CMAKE_BINARY_DIR}/config.ini COPYONLY)

# 多线程支持
if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()