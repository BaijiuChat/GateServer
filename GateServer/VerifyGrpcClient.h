#pragma once
#include "const.h"
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "Singleton.h"

using grpc::Channel; // 客户端与服务端之间的通信通道。
using grpc::ClientContext; // 用于配置和管理单个 RPC 调用的上下文。
using grpc::Status; //  RPC 调用的结果状态。

using message::GetVerifyReq;
using message::GetVerifyRsp;
using message::VerifyService;

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
	std::unique_ptr <VerifyService::Stub> stub_;
};

