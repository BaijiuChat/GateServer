#pragma once
#include "const.h"

class HttpConnection; //ǰ������HttpConnection������ͷ�ļ���ѭ�����ã�һ����Ҫ��ͷ�ļ�include HttpConnection
typedef std::function<void(std::shared_ptr<HttpConnection>)>HttpHandler; // ����HttpHandler����
class LogicSystem : public Singleton<LogicSystem> {
	friend class Singleton<LogicSystem>;
public:
	// ~LogicSystem(){};  ����ûɶ�ã�����
	bool HandleGet(std::string, std::shared_ptr<HttpConnection>);
	void RegGet(std::string, HttpHandler);
	void RegPost(std::string, HttpHandler);
	bool HandlePost(std::string, std::shared_ptr<HttpConnection>);
private:
	LogicSystem();
	std::map<std::string, HttpHandler> _post_handlers;
	std::map<std::string, HttpHandler> _get_handlers;
};

