#include "HttpConnection.h"
#include "LogicSystem.h"

// ��סsocketֻ���ƶ����죬���ܿ�������
HttpConnection::HttpConnection(tcp::socket socket)
	: _socket(std::move(socket))
{

}

void HttpConnection::Start()
{
	std::cout << "HttpConnection �ѣ��ٴΣ�������\n";
	auto self = shared_from_this();
	// async_read������socket��������������ͻص�������lambda��
	http::async_read(_socket, _buffer, _request, [self](beast::error_code ec, ::std::size_t bytes_transferred) {
		// try-catch��˫�н�����ʱ�������catch����������
		try {
			if (ec) {
				std::cout << "http������" << ec.what() << std::endl;
				return;
			}

			boost::ignore_unused(bytes_transferred);
			self->HandleReq();
			self->CheckDeadline();
		}
		catch (std::exception& exp) {
			std::cout << "��������" << exp.what() << std::endl;
		}
		});
}

void HttpConnection::CheckDeadline()
{
	auto self = shared_from_this();
	_deadline.async_wait([self](beast::error_code ec) {
		// ecΪ�մ����ʱ�������������ر�socket
		// ec�ǿգ��类����_deadline.cancel()����ʲôҲ����
		if (!ec) {
			self->_socket.close();
			std::cout << "���ӳ�ʱ�ˣ�" << std::endl;
		}
		});
}

unsigned char ToHex(unsigned char x)
{
	return  x > 9 ? x + 55 : x + 48;
}

unsigned char FromHex(unsigned char x)
{
	unsigned char y;
	if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
	else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
	else if (x >= '0' && x <= '9') y = x - '0';
	else assert(0);
	return y;
}

std::string UrlEncode(const std::string& str)
{
	std::string strTemp = "";
	size_t length = str.length();
	for (size_t i = 0; i < length; i++)
	{
		//�ж��Ƿ�������ֺ���ĸ����
		if (isalnum((unsigned char)str[i]) ||
			(str[i] == '-') ||
			(str[i] == '_') ||
			(str[i] == '.') ||
			(str[i] == '~'))
			strTemp += str[i];
		else if (str[i] == ' ') //Ϊ���ַ�
			strTemp += "+";
		else
		{
			//�����ַ���Ҫ��ǰ��%���Ҹ���λ�͵���λ�ֱ�תΪ16����
			strTemp += '%';
			strTemp += ToHex((unsigned char)str[i] >> 4);
			strTemp += ToHex((unsigned char)str[i] & 0x0F);
		}
	}
	return strTemp;
}

std::string UrlDecode(const std::string& str)
{
	std::string strTemp = "";
	size_t length = str.length();
	for (size_t i = 0; i < length; i++)
	{
		//��ԭ+Ϊ��
		if (str[i] == '+') strTemp += ' ';
		//����%������������ַ���16����תΪchar��ƴ��
		else if (str[i] == '%')
		{
			assert(i + 2 < length);
			unsigned char high = FromHex((unsigned char)str[++i]);
			unsigned char low = FromHex((unsigned char)str[++i]);
			strTemp += high * 16 + low;
		}
		else strTemp += str[i];
	}
	return strTemp;
}

void HttpConnection::PreParseGetParam() {
	// ��ȡ URI  
	auto uri = _request.target();
	// ���Ҳ�ѯ�ַ����Ŀ�ʼλ�ã��� '?' ��λ�ã�  
	auto query_pos = uri.find('?');
	if (query_pos == std::string::npos) {
		_get_url = uri;
		return;
	}

	_get_url = uri.substr(0, query_pos);
	std::string query_string = uri.substr(query_pos + 1);
	std::string key;
	std::string value;
	size_t pos = 0;
	while ((pos = query_string.find('&')) != std::string::npos) {
		auto pair = query_string.substr(0, pos);
		size_t eq_pos = pair.find('=');
		if (eq_pos != std::string::npos) {
			key = UrlDecode(pair.substr(0, eq_pos)); // ������ url_decode ����������URL����  
			value = UrlDecode(pair.substr(eq_pos + 1));
			_get_params[key] = value;
		}
		query_string.erase(0, pos + 1);
	}
	// �������һ�������ԣ����û�� & �ָ�����  
	if (!query_string.empty()) {
		size_t eq_pos = query_string.find('=');
		if (eq_pos != std::string::npos) {
			key = UrlDecode(query_string.substr(0, eq_pos));
			value = UrlDecode(query_string.substr(eq_pos + 1));
			_get_params[key] = value;
		}
	}
}

void HttpConnection::HandleReq()
{
	std::cout << "��ת�� HandleReq ҵ�񣬲���ʼ����Ƿ�ʱ" << std::endl;
	// ���ð汾��
	_response.version(_request.version());
	// Http���ǳ����ӣ����豣��
	_response.keep_alive(false);
	// ���������� get ����
	if (_request.method() == http::verb::get) {
		PreParseGetParam();
		bool success = LogicSystem::GetInstance()->HandleGet(_get_url, shared_from_this());
		if (!success) {
			_response.result(http::status::not_found);
			_response.set(http::field::content_type, "text/plain");
			beast::ostream(_response.body()) << "δ�ҵ� URL\r\n";
			WriteResponse(); // ����Ϣȫ��д��socket��ͬʱ��������Ӧ���ݵ�socket����ȥ
			return;
		}
		// �ɹ���
		_response.set(http::field::content_type, "text/plain");
		_response.result(http::status::ok);
		_response.set(http::field::server, "GateServer");
		WriteResponse();
	}
	else if (_request.method() == http::verb::post) {
		// �жϳɲ��ɹ���.target()��ȡ����url
		bool success = LogicSystem::GetInstance()->HandlePost(_request.target(), shared_from_this());
		if (!success) {
			_response.result(http::status::not_found);
			_response.set(http::field::content_type, "text/plain");
			beast::ostream(_response.body()) << "δ�ҵ� URL\r\n";
			WriteResponse(); // ����Ϣȫ��д��socket��ͬʱ��������Ӧ���ݵ�socket����ȥ
			return;
		}
		// �ɹ���
		_response.set(http::field::content_type, "text/plain");
		_response.result(http::status::ok);
		_response.set(http::field::server, "GateServer");
		WriteResponse();
	}
}

void HttpConnection::WriteResponse()
{
	auto self = shared_from_this();
	_response.content_length(_response.body().size()); // ���ó���
	http::async_write(_socket, _response, [self](beast::error_code ec, ::std::size_t bytes_transferred) {
		self->_socket.shutdown(tcp::socket::shutdown_send, ec); // �رշ������˵�socket����
		self->_deadline.cancel();
		});
}