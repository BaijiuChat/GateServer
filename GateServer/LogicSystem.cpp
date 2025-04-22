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

// 统一处理错误并返回响应
void handleErrorResponse(Json::Value& root, int errorCode, const std::string& customMessage = "") {
	root["error"] = errorCode;

	if (!customMessage.empty()) {
		root["message"] = customMessage;
	}
	else {
		// 为常见错误提供标准消息
		switch (errorCode) {
		case ErrorCodes::Error_Json:
			root["message"] = "JSON格式错误或参数不完整";
			break;
		case ErrorCodes::VerifyExpired:
			root["message"] = "验证码已过期";
			break;
		case ErrorCodes::VerifyCodeErr:
			root["message"] = "验证码错误";
			break;
		case ErrorCodes::PasswdErr:
			root["message"] = "两次输入的密码不一致";
			break;
		case ErrorCodes::UserEmailNotExists:
			root["message"] = "用户名或邮箱不存在";
			break;
		case ErrorCodes::UserEmailExists:
			root["message"] = "用户名或邮箱已存在";
			break;
		case ErrorCodes::SQLFailed:
			root["message"] = "数据库操作异常";
			break;
		case ErrorCodes::DatabaseConnectionFailed:
			root["message"] = "数据库连接失败";
			break;
		case ErrorCodes::DatabaseProcedureError:
			root["message"] = "存储过程执行异常";
			break;
		case ErrorCodes::GeneralException:
			root["message"] = "服务器发生异常";
			break;
		case ErrorCodes::UnknownException:
			root["message"] = "服务器发生未知异常";
			break;
		case ErrorCodes::UnknownError:
			root["message"] = "未定义的错误";
			break;
		default:
			root["message"] = "系统错误";
			break;
		}
	}
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
				handleErrorResponse(root, ErrorCodes::UserEmailExists);
				break;
			case -1:
				std::cout << "SQL异常" << std::endl;
				handleErrorResponse(root, ErrorCodes::SQLFailed);
				break;
			case -2:
				std::cout << "无法获取数据库连接" << std::endl;
				handleErrorResponse(root, ErrorCodes::DatabaseConnectionFailed);
				break;
			case -3:
				std::cout << "存储过程未返回结果" << std::endl;
				handleErrorResponse(root, ErrorCodes::DatabaseProcedureError);
				break;
			case -4:
				std::cout << "发生标准异常" << std::endl;
				handleErrorResponse(root, ErrorCodes::GeneralException);
				break;
			case -5:
				std::cout << "发生未知异常" << std::endl;
				handleErrorResponse(root, ErrorCodes::UnknownException);
				break;
			default:
				std::cout << "未预期的错误码: " << uid << std::endl;
				handleErrorResponse(root, ErrorCodes::UnknownError, "未预期的错误码: " + std::to_string(uid));
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
				std::cout << "确认密码错误" << std::endl;
				root["error"] = ErrorCodes::PasswdErr;
				std::string jsonstr = root.toStyledString();
				beast::ostream(connection->_response.body()) << jsonstr;
				return true;
			}

			// 先查找redis中email对应的验证码是否合理
			std::string verify_code;
			bool b_get_verifycode = RedisMgr::GetInstance()->Get(CODEPREFIX + email, verify_code);
			if (!b_get_verifycode) {
				std::cout << "验证码过期了！" << std::endl;
				root["error"] = ErrorCodes::VerifyExpired;
				std::string jsonstr = root.toStyledString();
				beast::ostream(connection->_response.body()) << jsonstr;
				return true;
			}

			if (verify_code != verifycode) {
				std::cout << "验证码错误！" << std::endl;
				root["error"] = ErrorCodes::VerifyCodeErr;
				std::string jsonstr = root.toStyledString();
				beast::ostream(connection->_response.body()) << jsonstr;
				return true;
			}

			// 检查用户名和邮箱是否匹配
			int checkResult = MySqlMgr::GetInstance()->CheckEmail(user, email);
			if (checkResult != 0) {
				std::cout << "用户名或邮箱错误！错误码: " << checkResult << std::endl;
				switch (checkResult) {
				case -2:
					handleErrorResponse(root, ErrorCodes::ERR_NETWORK);
					break;
				case -6:
					handleErrorResponse(root, ErrorCodes::UserEmailNotExists);
					break;
				case -7:
					handleErrorResponse(root, ErrorCodes::UserMailNotMatch);
					break;
				}
				std::string jsonstr = root.toStyledString();
				beast::ostream(connection->_response.body()) << jsonstr;
				return true;
			}
			// 更新密码
			int updateResult = MySqlMgr::GetInstance()->UpdatePwd(user, pwd);
			if (updateResult != 0) {
				std::cout << "更新密码失败！错误码: " << updateResult << std::endl;

				// 根据错误码设置不同的错误信息
				switch (updateResult) {
				case -1:
					handleErrorResponse(root, ErrorCodes::SQLFailed);
					break;
				case -2:
					handleErrorResponse(root, ErrorCodes::DatabaseConnectionFailed);
					break;
				case -4:
					handleErrorResponse(root, ErrorCodes::GeneralException);
					break;
				case -5:
					handleErrorResponse(root, ErrorCodes::UnknownException);
					break;
				case -6:
					handleErrorResponse(root, ErrorCodes::UserEmailNotExists);
					break;
				case -7:
					handleErrorResponse(root, ErrorCodes::UserMailNotMatch);
					break;
				default:
					handleErrorResponse(root, ErrorCodes::PasswdUpFailed, "密码更新失败，错误码: " + std::to_string(updateResult));
					break;
				}

				std::string jsonstr = root.toStyledString();
				beast::ostream(connection->_response.body()) << jsonstr;
				return true;
			}

			std::cout << "更新密码成功！新密码为" << pwd << std::endl;
			root["error"] = ErrorCodes::SUCCESS;
			root["email"] = email;
			root["user"] = user;
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