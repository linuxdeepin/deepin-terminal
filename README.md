# Deepin Terminal

Deepin Terminal is an advanced terminal emulator with workspace , multiple windows, remote management, quake mode and other features.

## Dependencies

### Build dependencies
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

## Installation

### Build from source code

1. Make sure you have installed all dependencies, you can use the following command.
```
$ cd deepin-terminal
$ sudo apt build-dep .
```

2. Build:
```
$ cd deepin-terminal
$ mkdir build
$ cd build
$ cmake ..
$ make
```

3. Install:
```
$ sudo make install
```

The executable binary file could be found at `/usr/bin/deepin-terminal` after the installation is finished.

### Other distro

CMake will tell you which package you are missing.

## Config file

* Terminal Settings's configure save at:<br/>
~/.config/deepin/deepin-terminal/config.conf

* Remote Servers' configure save at:<br/>
~/.config/deepin/deepin-terminal/server-config.conf

* Custom Command's configure save at:<br/>
~/.config/deepin/deepin-terminal/command-config.conf

## Usage

Execute `deepin-terminal -h` to get more details.

## Getting help

Any usage issues can ask for help via

* [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
* [IRC channel](https://webchat.freenode.net/?channels=deepin)
* [Forum](https://bbs.deepin.org)
* [WiKi](https://wiki.deepin.org/)

## Getting involved

We encourage you to report issues and contribute changes

* [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en). (English)
* [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## License

Deepin Terminal is licensed under [GPL-3.0-or-later](LICENSE).
