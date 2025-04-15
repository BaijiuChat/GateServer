#include "AsioIOServicePool.h"
#include <iostream>
using namespace std;

AsioIOServicePool::AsioIOServicePool(std::size_t size) :
    _ioServices(size),
    _works(size),
    _nextIOService(0) {
    // io_context池，多个银行窗口（比如2个窗口），每个窗口独立处理业务，避免顾客挤在一个窗口排队。
    for (std::size_t i = 0; i < size; ++i) {
        // 使用emplace创建Work对象，而不是错误的赋值方式
        _works[i] = std::make_unique<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(
            _ioServices[i].get_executor());
    }
    // 窗口的“营业中”牌子（只要挂着，窗口就保持工作状态，即使暂时没顾客）。
    // 遍历多个ioservice，创建多个线程，每个线程内部启动ioservice
    for (std::size_t i = 0; i < _ioServices.size(); ++i) {
        _threads.emplace_back([this, i]() {
            _ioServices[i].run();
            });
    }
}

AsioIOServicePool::~AsioIOServicePool() {
    Stop();
    std::cout << "AsioIOServicePool destruct" << endl;
}

// 叫号机按轮询分配窗口（顾客1去窗口A，顾客2去窗口B，顾客3又回到窗口A，公平分配）。
boost::asio::io_context& AsioIOServicePool::GetIOService() {
    auto& service = _ioServices[_nextIOService++];
    if (_nextIOService == _ioServices.size()) {
        _nextIOService = 0;
    }
    return service;
}

void AsioIOServicePool::Stop() {
    // 因为仅仅执行work.reset并不能让iocontext从run的状态中退出
    // 当iocontext已经绑定了读或写的监听事件后，还需要手动stop该服务。

    // 停止当前的所有io_service
    for (auto& io_service : _ioServices) {
        io_service.stop();
    }
    // 通知不再接受新的工作
    for (auto& work : _works) {
        work.reset();
    }
    // 回收线程
    for (auto& t : _threads) {
        t.join();
    }
}