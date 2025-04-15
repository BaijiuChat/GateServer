#pragma once
#include <memory>        // 用于 enable_shared_from_this
#include <boost/asio.hpp>

namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class CServer:public std::enable_shared_from_this<CServer>
{
public:
	CServer(boost::asio::io_context& ioc, unsigned short& port);  // 上面的namespace
	void Start();
private:
	net::io_context& _ioc;
	tcp::acceptor _acceptor;
};

