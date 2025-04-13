const grpc = require('@grpc/grpc-js')
const message_proto = require('./proto')
const const_module = require('./const')
const { v4:uuidv4 } = require('uuid')
const emailModule = require ('./email')

function generateSixDigitCode() {
    const randomNum = Math.floor(Math.random() * 1000000); // 0~999999
    return randomNum.toString().padStart(6, '0'); // 不足6位补零
}

// call是请求，callback是回调函数，注意async
async function GetVerifyCode(call, callback) {
    console.log("email is ", call.request.email)
    console.log("GetVerifyCode is called")

    //生成验证码
    let uniqueId = null;
    try{
        uniqueId = generateSixDigitCode();
        console.log("uniqueId is ", uniqueId)
        let text_str = `您的验证码如下:\n\n<b><font size="8">${uniqueId}</font></b>\n\n使用该验证码验证您的邮箱并完成你的注册。\n\n如果您并没有请求验证码，请无视该邮件。\n\n白久无瑕团队敬上`;
        //发送邮件
        let mailOptions = {
            from: 'baijiuwuhu@163.com',
            to: call.request.email,
            subject: '白久Chat验证码',
            text: text_str,
        };
        // 调用email.js中的SendMail函数发送邮件
        // 利用await等待异步操作发送邮件完成，发完之后再往下走程序
        // 注意await必须在async函数中使用
        let send_res = await emailModule.SendMail(mailOptions);
        console.log("send res is ", send_res)

        callback(null, { email:  call.request.email,
            error:const_module.Errors.Success
        }); 


    }catch(error){
        console.log("catch error is ", error)
        // 这里的null指的是有没有发生底层的错误
        callback(null, { email:  call.request.email,
            error:const_module.Errors.Exception
        }); 
    }

}

function main() {
    var server = new grpc.Server()
    server.addService(message_proto.VerifyService.service, { GetVerifyCode: GetVerifyCode })
    server.bindAsync('0.0.0.0:50051', grpc.ServerCredentials.createInsecure(), () => {
        // server.start() 已经不需要，bindAsync会自动调用bindAsync
        console.log('gRPC 服务已启动，监听端口 50051')        
    })
}

main()