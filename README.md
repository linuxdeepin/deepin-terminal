DTK + QTermWidget terminal application.

## Building from source

### Build depts:

 - CMake
 - Qt >= 5.7.1 (requires QtWidgets)
 - Deepin Toolkit (requires DtkCore and DtkWidgets)
 - QTermWidget5

#### Debian based distro for lazy guy

``` shell
# apt build-dep qterminal
# apt build-dep deepin-screenshot
```

#### Other distro

CMake will tell you which package you are missing.

### Build:

Generic CMake way:

``` shell
$ cd /path/to/the/source/
$ mkdir build && cd $_
$ cmake ../
$ make # then you'll got the proper binary to run.
```

## TODO

 - [ ] Proper license
 - [ ] CLI arguments processing
 - [ ] Drop-down mode (quark mode in deepin-terminal)
 - [ ] Proper way to implement shortcut support
 - [ ] Properties dialog
 - [ ] Color scheme preview from selection panel
 - [ ] Independent terminal config
 - [ ] Proper way to support ZModel / remote connection panel
 - [ ] Command mode (planned feature)

## License

// Should add a license before this repo got public.

Haven't check about the license issue, will probably be licensed under GPLv3.

## Disclaimer

"Deepin" is a trademark of Wuhan Deepin Technology.

This project is (currently) not affiliated with, funded, or in any way associated with Wuhan Deepin Technology.

THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.