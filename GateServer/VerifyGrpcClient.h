#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "Singleton.h"
#include "const.h"  // 仅用于 ErrorCodes 枚举，可以考虑单独提取

using grpc::Channel; // 客户端与服务端之间的通信通道。
using grpc::ClientContext; // 用于配置和管理单个 RPC 调用的上下文。
using grpc::Status; //  RPC 调用的结果状态。

using message::GetVerifyReq;
using message::GetVerifyRsp;
using message::VerifyService;

class RPConPool {
public:
	RPConPool(size_t poolsize, std::string host, std::string port) :
		poolSize(poolsize), host_(host), port_(port), b_stop(false)
	{
		for (size_t i = 0; i < poolSize; ++i) {
			auto channel = grpc::CreateChannel(host_ + ":" + port_, grpc::InsecureChannelCredentials());
			connections_.push(std::move(VerifyService::NewStub(channel)));
		}
	}

	~RPConPool(){
		std::lock_guard<std::mutex> lock(mutex_);
		while (!connections_.empty()) {
			connections_.pop();
		}
	}

	void Close(){
		std::lock_guard<std::mutex> lock(mutex_);
		b_stop = true;
		cond_.notify_all(); // 通知所有等待的线程
	}
	
	
private:
	std::atomic<bool> b_stop;
	size_t poolSize;
	std::string host_;
	std::string port_;
	std::queue<std::unique_ptr<VerifyService::Stub>> connections_;
	std::condition_variable cond_;
	std::mutex mutex_;
};

// 这里是一个单例模式的实现，确保在整个程序中只有一个 VerifyGrpcClient 实例。
class VerifyGrpcClient : public Singleton<VerifyGrpcClient>
{
	friend class Singleton<VerifyGrpcClient>;
public:
	GetVerifyRsp GetVerifyCode(const std::string& email){
		GetVerifyReq request;
		GetVerifyRsp reply;
		ClientContext context;;

		request.set_email(email);
		Status status = stub_->GetVerifyCode(&context, request, &reply);
		if (status.ok()) {
			return reply;
		}
		else {
			reply.set_error(ErrorCodes::RPCFailed);
			std::cout << "RPC failed" << std::endl;
			return reply;
		}
	}
private:
	VerifyGrpcClient() {

	}
};

