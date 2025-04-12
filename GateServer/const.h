#pragma once  
// C++��׼��  
#include <iostream>  
#include "Singleton.h"  
#include <functional> // ʵ��std::function  
#include <map>  
#include <unordered_map>  
#include <memory>  //ʵ��CRTP����ݹ�  
// JSON��  
#include <json/json.h>  // ��������  
#include <json/value.h>  // �ڵ�ṹ  
#include <json/reader.h>  // ����json  
// Boost��  
#include <boost/beast/http.hpp> // http  
#include <boost/beast.hpp> // http��websocket  
#include <boost/asio.hpp> // �첽IO  
#include <boost/filesystem.hpp> // �ļ�ϵͳ  
#include <boost/property_tree/ptree.hpp> // �����ļ�
#include <boost/property_tree/ini_parser.hpp> // ����ini�����ļ�
// gRPC��  
#include <grpcpp/grpcpp.h>	// gRPC��ͷ�ļ�  

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
extern ConfigMgr g_config_mgr; // ����ȫ�ֱ���ConfigMgrʵ��