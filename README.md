# GateServer
不要看！害羞！
我还在学习中！
### ** 配置环境 **
1. 下载jsoncpp的0.y.z版本，通过编译获取两个库（release和debug），注意release需要在项目属性中调成无分页。
2. 在项目属性C++常规和链接器常规中包含jsoncpp的include、makefiles\vs71\x64\libjson中的两个lib（复制到新建文件夹libjson）
   D:\jsoncpp-0.y.z\include;$(IncludePath)
   D:\jsoncpp-0.y.z\makefiles\vs71\x64\libjson;$(LibraryPath)
4. debug和release分别调整链接库，debug是第四个，release是第一个。
5. 链接器->输入->附加依赖项填写json_vc71_libmt.lib/json_vc71_libmtd.lib
6. 通过vcpkg安装boost、grpc、hiredis和redis-plus-plus，并且集成至环境，项目属性代码生成调整C++标准为17。
7. 下载node.js、执行npm install ioredis。
8. 下载MySQL Connector C++的ZIP ARCHIVE（debug release都下，分别放两个文件夹）。
9. release版直接参照：https://blog.csdn.net/weixin_74027669/article/details/137203874
10. debug版的库文件夹注意，是在lib64\debug\vs14中，其他和上文教程没啥区别，注意复制要复制到.exe文件在的地方。


### ** 测试 **
使用Apifox测试、NaviCat操作数据库
