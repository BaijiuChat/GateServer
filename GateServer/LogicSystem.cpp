#include "LogicSystem.h"
#include "HttpConnection.h"
#include "VerifyGrpcClient.h"
#include <iostream>
#include <boost/beast/core/ostream.hpp>
#include <json/json.h>
#include <json/reader.h>
#include "RedisMgr.h"
#include "MySqlMgr.h"

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
	// get测试模块
	RegGet("/get_test", [](std::shared_ptr<HttpConnection> connection) {
		std::cout << "正在处理 get_test 测试业务。\n";
		beast::ostream(connection->_response.body()) << "received get_test request\n";
		int i = 0;
		for (auto& elem : connection->_get_params) {
			i++;
			beast::ostream(connection->_response.body()) << "param" << i << " key is " << elem.first;
			beast::ostream(connection->_response.body()) << ", " << " value is " << elem.second << std::endl;
		}
		// 不需要手动关闭连接，WriteResponse()会处理
		});

	// 发送验证码模块
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

		// 添加RPC调用的异常处理
		try {
			GetVerifyRsp rsp = VerifyGrpcClient::GetInstance()->GetVerifyCode(email);
			std::cout << "\nemail is " << email << "\n\n";
			root["error"] = rsp.error();
			root["email"] = src_root["email"];
		}
		catch (const std::exception& e) {
			std::cout << "RPC调用异常: " << e.what() << std::endl;
			root["error"] = ErrorCodes::RPCFailed;
			root["message"] = "RPC服务暂时不可用，请稍后重试";
		}
		catch (...) {
			std::cout << "RPC调用未知异常" << std::endl;
			root["error"] = ErrorCodes::RPCFailed;
			root["message"] = "RPC服务异常，请稍后重试";
		}

		std::string jsonstr = root.toStyledString();
		beast::ostream(connection->_response.body()) << jsonstr;
		return true;
		});

	// 用户注册模块
	RegPost("/user_register", [](std::shared_ptr<HttpConnection> connection) {
		auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
		std::cout << "receive body is " << body_str << std::endl;
		connection->_response.set(http::field::content_type, "text/json");

		Json::Value root;
		Json::Reader reader;
		Json::Value src_root;
		bool parse_success = reader.parse(body_str, src_root);

		if (!parse_success) {
			std::cout << "Failed to parse JSON data!" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		// 完善参数验证
		if (!src_root.isMember("email") ||
			!src_root.isMember("user") ||
			!src_root.isMember("passwd") ||
			!src_root.isMember("confirm") ||
			!src_root.isMember("verifycode")) {
			std::cout << "Json参数不完整" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			root["message"] = "参数不完整";
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		// 检查各字段是否为空
		auto email = src_root["email"].asString();
		auto user = src_root["user"].asString();
		auto pwd = src_root["passwd"].asString();
		auto confirm = src_root["confirm"].asString();
		auto verifycode = src_root["verifycode"].asString();

		if (email.empty() || user.empty() || pwd.empty() || confirm.empty() || verifycode.empty()) {
			std::cout << "参数不能为空" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			root["message"] = "参数不能为空";
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		if (pwd != confirm) {
			std::cout << "确认密码错误 " << std::endl;
			root["error"] = ErrorCodes::PasswdErr;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		// 先查找redis中email对应的验证码是否合理
		std::string  verify_code;
		bool b_get_verify = RedisMgr::GetInstance()->Get(CODEPREFIX + email, verify_code);
		if (!b_get_verify) {
			std::cout << " 验证码已过期 " << std::endl;
			root["error"] = ErrorCodes::VerifyExpired;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		if (verify_code != verifycode) {
			std::cout << " 验证码错误 " << std::endl;
			root["error"] = ErrorCodes::VerifyCodeErr;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		// 访问MySQL查找用户是否存在
		int uid = MySqlMgr::GetInstance()->RegUser(user, email, pwd);
		if (uid <= 0) {  // 捕获所有错误情况
			// 根据不同错误码给出不同的错误信息
			switch (uid) {
			case 0:
				std::cout << "用户名或邮箱已存在" << std::endl;
				root["error"] = ErrorCodes::UserEmailExists;
				break;
			case -1:
				std::cout << "SQL异常" << std::endl;
				root["error"] = ErrorCodes::SQLFailed;
				break;
			case -2:
				std::cout << "无法获取数据库连接" << std::endl;
				root["error"] = ErrorCodes::DatabaseConnectionFailed;
				break;
			case -3:
				std::cout << "存储过程未返回结果" << std::endl;
				root["error"] = ErrorCodes::DatabaseProcedureError;
				break;
			case -4:
				std::cout << "发生标准异常" << std::endl;
				root["error"] = ErrorCodes::GeneralException;
				break;
			case -5:
				std::cout << "发生未知异常" << std::endl;
				root["error"] = ErrorCodes::UnknownException;
				break;
			default:
				std::cout << "未预期的错误码: " << uid << std::endl;
				root["error"] = ErrorCodes::UnknownError;
				break;
			}
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		// 正常流程的响应
		root["error"] = 0;
		root["email"] = email;
		root["user"] = user;
		root["passwd"] = pwd;
		root["confirm"] = confirm;
		root["verifycode"] = verifycode;
		std::string jsonstr = root.toStyledString();
		beast::ostream(connection->_response.body()) << jsonstr;
		return true;
		});

		RegPost("/reset_pwd", [](std::shared_ptr<HttpConnection> connection) {
			auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
			std::cout << "请求头为：" << body_str << std::endl;
			connection->_response.set(http::field::content_type, "text/json");
			Json::Value root;
			Json::Reader reader;
			Json::Value src_root;
			bool parse_success = reader.parse(body_str, src_root);
			if (!parse_success) {
				std::cout << "解析Json失败！" << std::endl;
				root["error"] = ErrorCodes::Error_Json;
				std::string jsonstr = root.toStyledString();
				beast::ostream(connection->_response.body()) << jsonstr;
			}

			auto email = src_root["email"].asString();
			auto user = src_root["user"].asString();
			auto pwd = src_root["passwd"].asString();

			std::string verify_code;
			bool b_get_verifycode = RedisMgr::GetInstance()->Get(CODEPREFIX + src_root["email"].asString(), verify_code);
			if (!b_get_verifycode) {
				std::cout << "验证码过期了！" << std::endl;
				root["error"] = ErrorCodes::VerifyExpired;
				std::string jsonstr = root.toStyledString();
				beast::ostream(connection->_response.body()) << jsonstr;
				return true;
			}
			if (verify_code != src_root["verifycode"].asString()) {
				std::cout << "验证码错误！" << std::endl;
				root["error"] = ErrorCodes::VerifyCodeErr;
				std::string jsonstr = root.toStyledString();
				beast::ostream(connection->_response.body()) << jsonstr;
				return true;
			}

			bool isMatch = MySqlMgr::GetInstance()->CheckEmail(user, email);
			if (!isMatch) {
				std::cout << "用户名或邮箱错误！" << std::endl;
				root["error"] = ErrorCodes::UserMailNotMatch;
				std::string jsonstr = root.toStyledString();
				beast::ostream(connection->_response.body()) << jsonstr;
				return true;
			}
			bool b_up = MySqlMgr::GetInstance()->UpdatePwd(user, pwd);
			if (!b_up) {
				std::cout << "更新密码失败！" << std::endl;
				root["error"] = ErrorCodes::PasswdUpFailed;
				std::string jsonstr = root.toStyledString();
				beast::ostream(connection->_response.body()) << jsonstr;
				return true;
			}

			std::cout << "更新密码成功！新密码为" << pwd << std::endl;
			root["error"] = ErrorCodes::SUCCESS;
			root["email"] = email;
			root["user"] = user;
			root["passwd"] = pwd;
			root["verifycode"] = src_root["verify_code"].asString();
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
			});
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