# GateServer
不要看！害羞！
我还在学习中！
### ** VS配置环境 **
1. 下载jsoncpp的0.y.z版本，通过编译获取两个库（release和debug），注意release需要在项目属性中调成无分页。其他参见但不完全正确（详见下一条）：https://blog.csdn.net/m0_52143936/article/details/124815509

2. debug和release的常规里配置类型都是静态库lib不要动，Debug的代码生成运行库选多线程调试DLL、RELEASE选多线程DLL。

3. 在项目属性C++常规和链接器常规中包含jsoncpp的include、makefiles\vs71\x64\libjson中的两个lib（复制到新建文件夹libjson）
   D:\jsoncpp-0.y.z\include;$(IncludePath)
   D:\jsoncpp-0.y.z\makefiles\vs71\x64\libjson;$(LibraryPath)

4. debug和release分别调整链接库，debug是第四个，release是第三个。（和第二条一样）

5. 链接器->输入->附加依赖项填写json_vc71_libmt.lib/json_vc71_libmtd.lib

6. 通过vcpkg安装boost、grpc、hiredis和redis-plus-plus，并且集成至环境，项目属性代码生成调整C++标准为17。

7. 下载node.js、执行npm install ioredis。

8. 下载MySQL Connector C++的ZIP ARCHIVE（debug release都下，分别放两个文件夹）。

9. release版直接参照：https://blog.csdn.net/weixin_74027669/article/details/137203874

10. debug版的库文件夹注意，是在lib64\debug\vs14中，其他和上文教程没啥区别，注意复制要复制到.exe文件在的地方。

    

# CLion 工程配置

## 首次安装配置

如果这是第一次安装，建议直接在 Backup and Sync 中同步云端。手动配置时需要调整以下内容：

### 1. 编辑器设置

- 路径：编辑器 ➡️ 常规 ➡️ 编辑器标签页
- 设置：勾选“标记已修改”

### 2. 字体设置

- 字体选择：Consolas
- 字体大小：16
- 版式设置：调整为微软雅黑

### 3. 工具链设置

- 路径：构建执行部署 ➡️ 工具链
- 操作：将 Visual Studio 拉到最上方，工具集选择到 `D:\Visual Studio\Visual Studio 2022`

### 4. CMake 配置

- 操作：在 CMake 中添加配置（Release）
- 选项：输入 `-DCMAKE_TOOLCHAIN_FILE=D:\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows`

### 5. 插件与版本控制设置

- 插件：
  - 下载插件：Visual Studio Theme、GitHub Copilot
  - 设置：勾选“自动更新”，注意 VS Theme 只替换编辑器样式！
- 版本控制：
  - 在版本控制中登录 GitHub（记得授权并设置为默认，勾选）
  - 编辑右侧聊天，登录到 GitHub

### 6. 更新设置

- 路径：设置里搜索“更新”
- 设置：勾选“自动更新”，然后检查更新

## 工程创建与配置

### 文件夹创建

- 在 CLion 中创建工程
- 创建 `include` 和 `src` 文件夹，分别存放头文件和源文件

### 编译配置

- （如果上面已完成，此步可跳过）
- 操作：在编译配置中添加 Release
- CMake 额外选项：输入 `-DCMAKE_TOOLCHAIN_FILE=D:\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows`

### 控制台设置

- 操作：在项目设置中勾选“额外打开控制台”

### vcpkg 设置

1. （必做！CLion 可能自动完成，需挂机一会）
   - 在 CLion 左侧点击“更多”，选择 vcpkg
   - 定位到自己的 vcpkg 目录，确定
2. （同上）
   - 将 vcpkg 改为清单模式，确定，此时应自动生成 `vcpkg.json`

### CMakeLists.txt 设置

- 操作：将保存的 CMakeLists.txt 文本复制过去（注意是文本而非文件）
- 修改：更改项目名称，检查复制事件是否需要调整
- 注意：完成后一定要重新加载 CMakeLists！

## 注

- 如果使用 MSVC，需在设置中配置 Visual Studio 并将其设为首位

  


### ** 测试 **
使用Apifox测试、NaviCat操作数据库
