#pragma once  
#include <iostream>
#include <memory> // unique_ptr
#include <cstdint> // int64_t  
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>
#include <queue>
// Defer头文件实现一个简单的RAII类，用于在作用域结束时自动调用指定的函数
#include "Defer.h"
// MySQL Connector/C++ JDBC 接口头文件
#include <jdbc/mysql_driver.h>
#include <jdbc/cppconn/connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/exception.h>


class SqlConnection 
{
public:
    SqlConnection(sql::Connection* con, int64_t lasttime);
    std::unique_ptr<sql::Connection> _con;
    int64_t _last_oper_time;
};

class MySqlPool
{
public:
    MySqlPool(
        const std::string& url,
        const std::string& user,
        const std::string& pass,
        const std::string& schema,
        int poolSize);
    void checkConnection();
	~MySqlPool();

private:
	std::string url_;
	std::string user_;
	std::string pass_;
	std::string schema_;
	int poolSize_;
	std::queue<std::unique_ptr<SqlConnection>> pool_;
	std::mutex mutex_;
    std::condition_variable cond_; // 池子为空时让线程等待
    std::atomic<bool> b_stop_{ false }; // 池子停止时让线程退出
	std::thread _check_thread;
};
