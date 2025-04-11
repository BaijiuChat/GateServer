#pragma once

#include <iostream>
#include "Singleton.h"
#include <functional>
#include <map>
#include <unordered_map>

#include <json/json.h>  // ��������
#include <json/value.h>  // �ڵ�ṹ
#include <json/reader.h>  // ����json

#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <memory>  //ʵ��CRTP����ݹ�
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