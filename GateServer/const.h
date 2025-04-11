#pragma once

#include <iostream>
#include "Singleton.h"
#include <functional>
#include <map>
#include <unordered_map>

#include <json/json.h>  // 基础功能
#include <json/value.h>  // 节点结构
#include <json/reader.h>  // 解析json

#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <memory>  //实现CRTP奇异递归
#include <grpcpp/grpcpp.h>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

enum ErrorCodes {
	SUCCESS = 0,
	Error_Json = 1001,
	RPCFailed = 1002,
};