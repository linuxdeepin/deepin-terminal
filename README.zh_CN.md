# Deepin Terminal

深度终端是一款高级终端仿真器，具有工作区、多窗口、远程管理、震动模式等功能.

## 依赖关系

### 建立依赖关系
* cmake
* pkg-config
* Qt5(>= 5.7.1) with modules:
  - qtbase5-dev
  - qtbase5-private-dev
  - qttools5-dev-tools
* libdtkwidget-dev(>=5.2.2) with modules:
  - dtkcore-dev
  - dtkwidget-dev
  - dtkgui-dev
* lxqt-build-tools(>= 0.6.0)
* libglib2.0-dev
* libsecret-1-dev

## 安装

### 从源代码构建

1. Make sure you have installed all dependencies, you can use the following command.
```
$ cd deepin-terminal
$ sudo apt build-dep .
```

2. 构建:
```
$ cd deepin-terminal
$ mkdir build
$ cd build
$ cmake ..
$ make
```

3. 依赖:
```
$ sudo make install
```

The executable binary file could be found at `/usr/bin/deepin-terminal` after the installation is finished.

### 其他发行版

CMake will tell you which package you are missing.

## 配置文件

* Terminal Settings's configure save at:<br/>
~/.config/deepin/deepin-terminal/config.conf

* Remote Servers' configure save at:<br/>
~/.config/deepin/deepin-terminal/server-config.conf

* Custom Command's configure save at:<br/>
~/.config/deepin/deepin-terminal/command-config.conf

## 用法

Execute `deepin-terminal -h` to get more details.

## 寻求帮助

Any usage issues can ask for help via

* [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
* [IRC channel](https://webchat.freenode.net/?channels=deepin)
* [Forum](https://bbs.deepin.org)
* [WiKi](https://wiki.deepin.org/)

## 参与贡献

We encourage you to report issues and contribute changes

* [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en). (English)
* [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## 协议

Deepin Terminal 根据 [GPL-3.0-or-later]（许可证）获得许可.
