#include "CServer.h"
#include "ConfigMgr.h"
int main()
{
	ConfigMgr g_config_mgr; // 创建ConfigMgr实例
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