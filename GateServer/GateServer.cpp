#include "CServer.h"
#include "ConfigMgr.h"
#include <iostream>
#include <cstdlib>      // 用于 EXIT_FAILURE
#include <boost/asio/signal_set.hpp>

int main()
{
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