#include "LogicSystem.h"
#include "HttpConnection.h"
#include "VerifyGrpcClient.h"
#include <iostream>
#include <boost/beast/core/ostream.hpp>
#include <json/json.h>
#include <json/reader.h>
//#include "RedisMgr.h"

void LogicSystem::RegGet(std::string url, HttpHandler handler)
{
	_get_handlers.insert(make_pair(url, handler));
}

void LogicSystem::RegPost(std::string url, HttpHandler handler)
{
	_post_handlers.insert(make_pair(url, handler));
}

LogicSystem::LogicSystem()
{
	RegGet("/get_test", [](std::shared_ptr<HttpConnection> connection) {
		std::cout << "正在处理 get_test 测试业务。\n";
		beast::ostream(connection->_response.body()) << "received get_test request\n";
		int i = 0;
		for (auto& elem : connection->_get_params) {
			i++;
			beast::ostream(connection->_response.body()) << "param" << i << " key is " << elem.first;
			beast::ostream(connection->_response.body()) << ", " << " value is " << elem.second << std::endl;
		}
		});

	RegPost("/get_verifycode", [](std::shared_ptr<HttpConnection> connection) {
		std::cout << "正在处理 get_verifycode 测试业务。\n";
		auto body_str = beast::buffers_to_string(connection->_request.body().data());
		std::cout << "receive body is\n" << body_str;
		connection->_response.set(http::field::content_type, "text/json");
		// 使用Json解析
		Json::Value root;
		Json::Reader reader;
		Json::Value src_root;
		bool parse_success = reader.parse(body_str, src_root);
		if (!parse_success || !src_root.isMember("email")) {
			std::cout << "Json参数不正确，无法解析" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}
		auto email = src_root["email"].asString();
		GetVerifyRsp rsp = VerifyGrpcClient::GetInstance()->GetVerifyCode(email);
		std::cout << "\nemail is " << email << "\n\n";
		root["error"] = rsp.error();
		root["email"] = src_root["email"];
		std::string jsonstr = root.toStyledString(); // TCP是面向字节流的，需要转成字符串
		beast::ostream(connection->_response.body()) << jsonstr;
		return true;
		});

	//RegPost("/user_register", [](std::shared_ptr<HttpConnection> connection) {
	//	auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
	//	std::cout << "receive body is " << body_str << std::endl;
	//	connection->_response.set(http::field::content_type, "text/json");
	//	Json::Value root;
	//	Json::Reader reader;
	//	Json::Value src_root;
	//	bool parse_success = reader.parse(body_str, src_root);
	//	if (!parse_success) {
	//		std::cout << "Failed to parse JSON data!" << std::endl;
	//		root["error"] = ErrorCodes::Error_Json;
	//		std::string jsonstr = root.toStyledString();
	//		beast::ostream(connection->_response.body()) << jsonstr;
	//		return true;
	//	}
	//	//先查找redis中email对应的验证码是否合理
	//	std::string  verify_code;
	//	bool b_get_verify = RedisMgr::GetInstance()->Get(src_root["email"].asString(), verify_code);
	//	if (!b_get_verify) {
	//		std::cout << " get verify code expired" << std::endl;
	//		root["error"] = ErrorCodes::verifyExpired;
	//		std::string jsonstr = root.toStyledString();
	//		beast::ostream(connection->_response.body()) << jsonstr;
	//		return true;
	//	}

	//	if (verify_code != src_root["verifycode"].asString()) {
	//		std::cout << " verify code error" << std::endl;
	//		root["error"] = ErrorCodes::verifyCodeErr;
	//		std::string jsonstr = root.toStyledString();
	//		beast::ostream(connection->_response.body()) << jsonstr;
	//		return true;
	//	}

	//	//访问redis查找
	//	bool b_usr_exist = RedisMgr::GetInstance()->ExistsKey(src_root["user"].asString());
	//	if (b_usr_exist) {
	//		std::cout << " user exist" << std::endl;
	//		root["error"] = ErrorCodes::UserExist;
	//		std::string jsonstr = root.toStyledString();
	//		beast::ostream(connection->_response.body()) << jsonstr;
	//		return true;
	//	}

	//	//查找数据库判断用户是否存在

	//	root["error"] = 0;
	//	root["email"] = src_root["email"];
	//	root["user"] = src_root["user"].asString();
	//	root["passwd"] = src_root["passwd"].asString();
	//	root["confirm"] = src_root["confirm"].asString();
	//	root["verifycode"] = src_root["verifycode"].asString();
	//	std::string jsonstr = root.toStyledString();
	//	beast::ostream(connection->_response.body()) << jsonstr;
	//	return true;
	//	});
}

bool LogicSystem::HandleGet(std::string path, std::shared_ptr<HttpConnection> con)
{
	// 在map中寻找是否有path的url
	if (_get_handlers.find(path) == _get_handlers.end()) {
		return false;
	}
	_get_handlers[path](con);
	return true;
}

bool LogicSystem::HandlePost(std::string path, std::shared_ptr<HttpConnection> con) {
	// 在map中寻找是否有path的url
	if (_post_handlers.find(path) == _post_handlers.end()) {
		return false;
	}
	_post_handlers[path](con);
	return true;
}

