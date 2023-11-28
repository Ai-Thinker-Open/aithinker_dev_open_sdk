#http_demo


## 基础命令：

帮助命令：

```
$ ./build.sh help					//用于打印SDK工具支持的命令行

```

工具版本查询命令：

```
$ ./build.sh version				//查询工具版本编号

```

中间件接口文档更新命令：

```
$ ./build.sh docs					//更新中间接口文档

```

编译清空命令：

```
$ ./build.sh clean					//清空全部编译结果
$ ./build.sh 芯片型号 应用名称 clean   //清空具体应用结果

```

仓库恢复出厂命令：

```
$ ./build.sh reset					//清空所有改动，将仓库恢复到出厂模式

```

子仓库更新方法：

```
$ ./build.sh update					//一般在初始化SDK阶段使用，这个指令会更新/覆盖子仓库内容，如果对子仓库有更新未进行提交，不建议执行该指令

```

仓库环境部署命令：

```
$ ./build.sh environment			//自动下载编译依赖包，如发现依赖包不满足，请发送邮件到 <chenzf@aithinker.com>

```

应用开发配置方法：

```
$ ./build.sh 芯片型号 应用名称 menuconfig		 举例：./build.sh bl602 gpio_demo menuconfig
```

应用开发编译方法：

```
$ ./build.sh 芯片型号 应用名称 国家编码 调试等级	举例：./build.sh bl602 gpio_demo cn debug

```

demo 示例
```
// 目前支持bl618 bl602 ln882h

$ ./build.sh bl602 http_demo menuconfig // 配置使用的平台IO信息以及log等级等

$ ./build.sh bl602 http_demo cn debug // 编译对应的平台demo
```

demo 简介
```
固件路径:aithinker_dev_sdk/out/bl602(对应平台命名)/http_demo/bin/dev_project.bin

功能:测试HTTP请求，联网后主动发送HTTP请求连接指定测试网址，返回code：200 即为连接成功。
根据menuconfig的配置，见生成的"aiio_autoconf.h"文件内容。
注意：需要在menuconfig配置待连接的路由器账号密码，连接路由器成功后开始发送http请求，请确保对应测试网址可以ping通。
```

