#include "CServer.h"
#include "HttpConnection.h"
#include <iostream>
#include <boost/beast.hpp>

namespace beast = boost::beast;

// _ioc��_acceptor��_socket����˽�б����������캯����û���ṩ�����Գ�������Ĭ�Ϲ���
// Ϊ�������������죬��Ҫʹ�ÿ������죬Ҳ���� ð�� �������������
CServer::CServer(boost::asio::io_context& ioc, unsigned short& port) 
	:_ioc(ioc), // �ײ��첽I/O������
	 _acceptor(ioc, tcp::endpoint(tcp::v4(), port)),  // ����ioc����������������IPv4:port�˿�
	 _socket(ioc) // ���ں����������Ӻ������ͨѶ
{
	std::cout << "�������ڼ��� 0.0.0.0 �� 8080 �˿�" << std::endl;
}

void CServer::Start() {
	auto self = shared_from_this(); // ����ָ�룬��ֹ����������ǰ�ͷŵ�
	// ����selfָ��
	// �����Ӵ���socket�����۳ɹ�����ʧ�ܶ����лص����ص���Ҫ����ErrorCode
	// ���û�д�����ErrorCodeΪ��
	// async_accpet �����ڼ����첽���ӵ�
	_acceptor.async_accept(_socket, [self](beast::error_code ec) {
		try {
			if (ec) {
				std::cout << "�첽���ܷ��������������¼���" << std::endl;
				self->Start(); // ���¼���
				return;
			}

			// ���������ӣ�����HttpConnection���������������
			std::make_shared<HttpConnection>(std::move(self->_socket))->Start();

			// ��������
			self->Start();
		}
		catch (std::exception exp) {
			std::cout << "������" << exp.what() << std::endl;
			self->Start();
		}
		});

}