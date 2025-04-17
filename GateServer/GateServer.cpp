#include "CServer.h"
#include "ConfigMgr.h"
#include <iostream>
#include <cstdlib>      // 用于 EXIT_FAILURE
#include <boost/asio/signal_set.hpp>
#include <cassert>
#include "RedisMgr.h"

 //void TestRedisMgr() {
	//assert(RedisMgr::GetInstance()->Connect("127.0.0.1", 6379));	// 不需要
	//assert(RedisMgr::GetInstance()->Auth("root"));				// 不需要
	/*assert(RedisMgr::GetInstance()->Set("blogwebsite", "llfc.club"));
	std::string value = "";
	assert(RedisMgr::GetInstance()->Get("blogwebsite", value));
	assert(RedisMgr::GetInstance()->Get("nonekey", value) == false);
	assert(RedisMgr::GetInstance()->HSet("bloginfo", "blogwebsite", "llfc.club"));
	assert(RedisMgr::GetInstance()->HGet("bloginfo", "blogwebsite") != "");
	assert(RedisMgr::GetInstance()->ExistsKey("bloginfo"));
	assert(RedisMgr::GetInstance()->Del("bloginfo"));
	assert(RedisMgr::GetInstance()->Del("bloginfo"));
	assert(RedisMgr::GetInstance()->ExistsKey("bloginfo") == false);
	assert(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue1"));
	assert(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue2"));
	assert(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue3"));
	assert(RedisMgr::GetInstance()->RPop("lpushkey1", value));
	assert(RedisMgr::GetInstance()->RPop("lpushkey1", value));
	assert(RedisMgr::GetInstance()->LPop("lpushkey1", value));
	assert(RedisMgr::GetInstance()->LPop("lpushkey2", value) == false);*/
	//RedisMgr::GetInstance()->Close();  // 不需要
//}

int main()
{
	SetConsoleOutputCP(CP_UTF8); // 输出编码
	SetConsoleCP(CP_UTF8); // 输入编码
	// system("chcp 65001");
	// TestRedisMgr(); // 测试RedisMgr
	auto& g_config_mgr = ConfigMgr::Inst(); // 创建ConfigMgr实例
	std::string gate_port_str = g_config_mgr["GateServer"]["port"];
	unsigned short gate_port = atoi(gate_port_str.c_str());
	try {
		unsigned short port = static_cast<unsigned short>(8080);
		net::io_context ioc{ 1 };
		// 初始化信号集，绑定到ioc，监听SIGINT和SIGTERM
		net::signal_set signals(ioc, SIGINT, SIGTERM);
		signals.async_wait([&ioc](const boost::system::error_code& error, int signal_number) {
			if (error) {
				return; // 防御性编程​​，确保只有真正的信号触发才会停止事件循环。
			}
			ioc.stop(); // 主动退出机制​
			});

		std::make_shared<CServer>(ioc, port)->Start();
		ioc.run();
		std::cout << "成功启动底层异步I/O事件调度器\n";
	}
	catch (std::exception const& e) {
		std::cerr << "Error：" << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}