#pragma once  
// C++标准库  
#include <iostream>  
#include "Singleton.h"  
#include <functional> // 实现std::function  
#include <map>  
#include <unordered_map>  
#include <memory>  //实现CRTP奇异递归  
// JSON库  
#include <json/json.h>  // 基础功能  
#include <json/value.h>  // 节点结构  
#include <json/reader.h>  // 解析json  
// Boost库  
#include <boost/beast/http.hpp> // http  
#include <boost/beast.hpp> // http和websocket  
#include <boost/asio.hpp> // 异步IO  
#include <boost/filesystem.hpp> // 文件系统  
#include <boost/property_tree/ptree.hpp> // 配置文件
#include <boost/property_tree/ini_parser.hpp> // 解析ini配置文件
// gRPC库  
#include <grpcpp/grpcpp.h>	// gRPC的头文件  

namespace beast = boost::beast;         // from <boost/beast.hpp>  
namespace http = beast::http;           // from <boost/beast/http.hpp>  
namespace net = boost::asio;            // from <boost/asio.hpp>  
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>  

enum ErrorCodes {  
SUCCESS = 0,  
Error_Json = 1001,  
RPCFailed = 1002,  
};

class ConfigMgr;
extern ConfigMgr g_config_mgr; // 声明全局变量ConfigMgr实例