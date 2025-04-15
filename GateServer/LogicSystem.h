#pragma once
#include <map>
#include <memory>      // 用于 std::shared_ptr
#include <functional>  // 用于 std::function
#include "Singleton.h"

class HttpConnection; //前置声明HttpConnection，避免头文件的循环引用，一定不要在头文件include HttpConnection
typedef std::function<void(std::shared_ptr<HttpConnection>)>HttpHandler; // 声明HttpHandler函数
class LogicSystem : public Singleton<LogicSystem> {
	friend class Singleton<LogicSystem>;
public:
	// ~LogicSystem(){};  析构没啥用，不管
	bool HandleGet(std::string, std::shared_ptr<HttpConnection>);
	void RegGet(std::string, HttpHandler);
	void RegPost(std::string, HttpHandler);
	bool HandlePost(std::string, std::shared_ptr<HttpConnection>);
private:
	LogicSystem();
	std::map<std::string, HttpHandler> _post_handlers;
	std::map<std::string, HttpHandler> _get_handlers;
};

