# Deepin Terminal

Deepin Terminal is an advanced terminal emulator with workspace , multiple windows, remote management, quake mode and other features.

## Dependencies

### Build dependencies
* cmake
* pkg-config
* Qt5(>= 5.7.1) with modules:
  - qt5-default
  - qtbase5-dev
  - qtbase5-private-dev
  - qttools5-dev-tools
* libdtkwidget-dev(>=5.1.0) with modules:
  - dtkcore-dev
  - dtkwidget-dev
  - dtkgui-dev
* lxqt-build-tools(>= 0.6.0)
* libutf8proc-dev

### Debian based distro for lazy guy

``` shell
$ cd /path/to/the/source/
$ sudo apt build-dep .
```

### Other distro

CMake will tell you which package you are missing.

### Build:

Generic CMake way:

``` shell
$ cd /path/to/the/source/
$ mkdir build && cd build
$ cmake ..
$ make
$ ./deepin-terminal
```


## Config file

* Terminal Settings's configure save at:<br/>
~/.config/deepin/deepin-terminal/config.conf

* Remote Servers' configure save at:<br/>
~/.config/deepin/deepin-terminal/server-config.conf

* Custom Command's configure save at:<br/>
~/.config/deepin/deepin-terminal/command-config.conf

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

Deepin Terminal is licensed under [GPLv3](LICENSE).
