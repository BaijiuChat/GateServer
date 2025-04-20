#include "MySqlDao.h"

SqlConnection::SqlConnection(sql::Connection* con, int64_t lasttime)
	:_con(con), _last_oper_time(lasttime)
{
	
}

MySqlPool::MySqlPool(const std::string& url, const std::string& user, const std::string& pass, const std::string& schema, int poolSize)
	:url_(url), user_(user), pass_(pass), schema_(schema), poolSize_(poolSize)
{
	try {
		for (int i = 0; i < poolSize_; ++i) {
			// Connector C++ 使用单例模式来创建驱动实例，所以需要用指针
			sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance(); 
			sql::Connection* con = driver->connect(url_, user_, pass_); // 
			con->setSchema(schema_);
			auto CurrentTime = std::chrono::system_clock::now().time_since_epoch();
			long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(CurrentTime).count();
			// 记录连接和最后操作时间
			pool_.push(std::make_unique<SqlConnection>(con, timestamp));
		}

		_check_thread = std::thread([this]() {
			while (!b_stop_) {
				checkConnection();
				std::this_thread::sleep_for(std::chrono::seconds(60)); // 每60秒检查一次
			}
			});
		_check_thread.detach(); // 分离线程，交给系统管理
	}
	catch (sql::SQLException& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		Close(); // 发生异常时关闭连接池
		throw; // 重新抛出异常
	}
	catch (std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		Close(); // 发生异常时关闭连接池
		throw; // 重新抛出异常
	}
	catch (...) {
		std::cerr << "Unknown error" << std::endl;
		Close(); // 发生异常时关闭连接池
		throw; // 重新抛出异常
	}
}

void MySqlPool::checkConnection()
{
	std::lock_guard<std::mutex> guard(mutex_);
	int poolSize = pool_.size();
	auto currentTime = std::chrono::system_clock::now().time_since_epoch();
	long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
	for (int i = 0; i < poolSize; ++i) {
		auto con = std::move(pool_.front());
		pool_.pop();
		Defer defer([this, &con]() {
			pool_.push(std::move(con));
			});

		if (con->_last_oper_time + 60 > timestamp)  // 超过60秒没有操作
			continue;

		try {
			std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("SELECT 1"));
			pstmt->execute();
			con->_last_oper_time = timestamp; // 更新最后操作时间
			std::cout << "MySQL链接存活，操作时间为：" << timestamp << std::endl;
		}
		catch (sql::SQLException& e) {
			std::cerr << "MySQL链接失效，重新创建连接，错误为：" << e.what() << std::endl;
			sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();
			sql::Connection* newCon = driver->connect(url_, user_, pass_);
			newCon->setSchema(schema_);
			con->_con.reset(newCon); // 替换旧连接
			auto CurrentTime = std::chrono::system_clock::now().time_since_epoch();
			long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(CurrentTime).count();
			con->_last_oper_time = timestamp; // 更新最后操作时间
		}
		catch (std::exception& e) {
			std::cerr << "Error: " << e.what() << std::endl;
		}
		catch (...) {
			std::cerr << "Unknown error" << std::endl;
		}
	}
}

std::unique_ptr<SqlConnection> MySqlPool::getConnection() 
{
	std::unique_lock<std::mutex> lock(mutex_);
	cond_.wait(lock, [this] {
		if (b_stop_) {
			return true;
		}
		return !pool_.empty();
		});
	if (b_stop_) {
		return nullptr; // 确保线程持有锁的时候检查新的b_stop_值
	}
	std::unique_ptr<SqlConnection> con(std::move(pool_.front()));
	pool_.pop(); // 移除空指针
	return con;
}

void MySqlPool::returnConnection(std::unique_ptr<SqlConnection> con)
{
	std::unique_lock<std::mutex> lock(mutex_);
	if (b_stop_) {
		return; // 如果池子已经停止，就不用返回连接了
	}
	pool_.push(std::move(con)); // 将连接放回池中
	cond_.notify_one(); // 通知一个等待的线程
}

void MySqlPool::Close()
{
	b_stop_ = true; // 设置停止标志
	cond_.notify_all(); // 通知所有等待的线程
}

MySqlPool::~MySqlPool()
{
	std::unique_lock<std::mutex> lock(mutex_);
	while (!pool_.empty()) {
		pool_.pop();
	}
}
