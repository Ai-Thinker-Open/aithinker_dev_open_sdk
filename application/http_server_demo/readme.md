# 简介

这个demo是用来演示http server的demo

支持芯片：BL602、BL616、BL618、LN882H

# 环境搭建

参考主工程的readme，搭建开发环境后可以正常 demo 例程表示环境搭建完毕

# 编译

1. 进入sdk根目录

2. 执行 ./build.sh <platform> http_server_demo menuconfig ,不用修改直接退出，生成默认config

   eg：./build.sh bl602 http_server_demo menuconfig

3. 执行 ./build.sh <platform> http_server_demo cn debug 开始编译

   eg：./build.sh bl602 http_server_demo cn debug

# 测试

## 使用

连接模组AP,浏览器输入模组IP进行测试

/hello      GET方法

/echo       POST方法

/ctrl       PUT方法

## /hello
浏览器输入IP+URI,如
```C
192.168.100.1/hello
```
浏览器会返回"Hello World!"

## /echo
浏览器输入模组IP访问,按下键盘F12打开控制台,在控制台输入
```
fetch(new Request('/echo',{
    method:'POST', 
    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
    body:"param1=value1&param2=value2"
})).then((resp)=>{console.log(resp)})
```
点击回车,观察LOG口输出信息

## /ctrl
浏览器输入模组IP访问,按下键盘F12打开控制台,在控制台输入
```
fetch(new Request('/ctrl',{
    method:'PUT', 
    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
    body:"0"
})).then((resp)=>{console.log(resp)})
```
此指令会注销/hello,再次访问/hello会报错

控制台重新输入下述内容,可重新注册/hello
```
fetch(new Request('/ctrl',{
    method:'PUT', 
    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
    body:"1"
})).then((resp)=>{console.log(resp)})
```