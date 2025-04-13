#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "Singleton.h"
#include "const.h"  // ������ ErrorCodes ö�٣����Կ��ǵ�����ȡ

using grpc::Channel; // �ͻ���������֮���ͨ��ͨ����
using grpc::ClientContext; // �������ú͹����� RPC ���õ������ġ�
using grpc::Status; //  RPC ���õĽ��״̬��

using message::GetVerifyReq;
using message::GetVerifyRsp;
using message::VerifyService;

// ������һ������ģʽ��ʵ�֣�ȷ��������������ֻ��һ�� VerifyGrpcClient ʵ����
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
		std::shared_ptr<Channel> channel = grpc::CreateChannel(
			"localhost:50051", grpc::InsecureChannelCredentials());
		stub_ = VerifyService::NewStub(channel); // ����һ���µĴ��������ʹ��
	}
	std::unique_ptr <VerifyService::Stub> stub_; // ����������������˽���ͨ��
};

