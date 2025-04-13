#include "CServer.h"
#include "HttpConnection.h"
#include <iostream>
#include <boost/beast.hpp>

namespace beast = boost::beast;

// _ioc、_acceptor和_socket都是私有变量，而构造函数中没有提供，所以程序会调用默认构造
// 为了让其正常构造，需要使用拷贝构造，也就是 冒号 后面的三个东西
CServer::CServer(boost::asio::io_context& ioc, unsigned short& port) 
	:_ioc(ioc), // 底层异步I/O调度器
	 _acceptor(ioc, tcp::endpoint(tcp::v4(), port)),  // 接入ioc，并监听本机所有IPv4:port端口
	 _socket(ioc) // 用于后续接受链接后的数据通讯
{
	std::cout << "现在正在监听 0.0.0.0 的 8080 端口" << std::endl;
}

void CServer::Start() {
	auto self = shared_from_this(); // 智能指针，防止被析构后提前释放掉
	// 捕获self指针
	// 将连接传给socket，无论成功还是失败都会有回调，回调需要传入ErrorCode
	// 如果没有错误，则ErrorCode为空
	// async_accpet 是用于监听异步连接的
	_acceptor.async_accept(_socket, [self](beast::error_code ec) {
		try {
			if (ec) {
				std::cout << "异步接受发生错误，正在重新监听" << std::endl;
				self->Start(); // 重新监听
				return;
			}

			// 创建新链接，创建HttpConnection类来管理这个连接
			std::make_shared<HttpConnection>(std::move(self->_socket))->Start();

			// 继续监听
			self->Start();
		}
		catch (std::exception exp) {
			std::cout << "错误是" << exp.what() << std::endl;
			self->Start();
		}
		});

}