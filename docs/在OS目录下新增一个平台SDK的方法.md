### os目录结构

```
.
├── bl602_sdk_lts					//sdk命名规范：芯片型号_sdk_lts
│   ├── changeLog.md
│   ├── env.sh
│   ├── patch
│   ├── patch.sh
│   ├── sdk
│   ├── tools
│   └── zip
├── w800_sdk_lts					//sdk命名规范：芯片型号_sdk_lts
│   ├── changeLog.md
│   ├── env.sh
│   ├── patch
│   ├── patch.sh
│   ├── sdk
│   ├── tools
│   └── zip
├── os_version						//系统管理目录
│   ├── w800_sdk_lts_version.txt	//文件命名规范：芯片型号_sdk_lts_version.txt
│   └── bl602_sdk_lts_version.txt	//文件命名规范：芯片型号_sdk_lts_version.txt
└── os_version_ctrl.sh	//版本控制器，对具体仓库版本进行控制，无需关注
```

### 新增一个平台SDK的方法：

```
1）在os_version目录下新建一个文件，文件命名规范：芯片型号_sdk_lts_version.txt
2）在xx_sdk_lts_version.txt里面填充仓库字段
```

```
参考示例：

name:w800_sdk_lts
branch:master
commit:40375c435bc4870e7f6ecae93a68836ecf8ae7c8
repository:https://e.coding.net/axk/2023niansdkweihuguihua/w800_sdk_lts.git
```

```
字段解析：

name:w800_sdk_lts															//仓库名称，命名规范：芯片型号_sdk_lts
branch:master																//分支名称
commit:40375c435bc4870e7f6ecae93a68836ecf8ae7c8								//提交commit编号
repository:https://e.coding.net/axk/2023niansdkweihuguihua/w800_sdk_lts.git	//coding仓库HTTPS链接
```

