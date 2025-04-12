#include "LogicSystem.h"
#include "HttpConnection.h"
#include "VerifyGrpcClient.h" // ����post��grpc

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
		std::cout << "���ڴ��� get_test ����ҵ��\n";
		beast::ostream(connection->_response.body()) << "received get_test request\n";
		int i = 0;
		for (auto& elem : connection->_get_params) {
			i++;
			beast::ostream(connection->_response.body()) << "param" << i << " key is " << elem.first;
			beast::ostream(connection->_response.body()) << ", " << " value is " << elem.second << std::endl;
		}
		});

	RegPost("/get_verifycode", [](std::shared_ptr<HttpConnection> connection) {
		std::cout << "���ڴ��� get_verifycode ����ҵ��\n";
		auto body_str = beast::buffers_to_string(connection->_request.body().data());
		std::cout << "receive body is\n" << body_str;
		connection->_response.set(http::field::content_type, "text/json");
		// ʹ��Json����
		Json::Value root;
		Json::Reader reader;
		Json::Value src_root;
		bool parse_success = reader.parse(body_str, src_root);
		if (!parse_success || !src_root.isMember("email")) {
			std::cout << "Json��������ȷ���޷�����" << std::endl;
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
		std::string jsonstr = root.toStyledString(); // TCP�������ֽ����ģ���Ҫת���ַ���
		beast::ostream(connection->_response.body()) << jsonstr;
		return true;
		});
}

bool LogicSystem::HandleGet(std::string path, std::shared_ptr<HttpConnection> con)
{
	// ��map��Ѱ���Ƿ���path��url
	if (_get_handlers.find(path) == _get_handlers.end()) {
		return false;
	}
	_get_handlers[path](con);
	return true;
}

bool LogicSystem::HandlePost(std::string path, std::shared_ptr<HttpConnection> con) {
	// ��map��Ѱ���Ƿ���path��url
	if (_post_handlers.find(path) == _post_handlers.end()) {
		return false;
	}
	_post_handlers[path](con);
	return true;
}

