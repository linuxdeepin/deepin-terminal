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

 - [x] Proper license
 - [x] CLI arguments processing
 - [ ] Custom commands
 - [ ] Drop-down mode (quark mode in deepin-terminal)
 - [ ] Proper way to implement shortcut support
 - [ ] Properties dialog
 - [ ] Color scheme preview from selection panel
 - [ ] Independent terminal config
 - [ ] Proper way to support ZModem / remote connection panel
 - [ ] Command mode (planned feature)

## License

This program is released under the terms of the GNU General Public License, version 3.0 or, at your option, any later version, as published by the Free Software Foundation. See the LICENSE file for the full text of the license.

## Disclaimer

"Deepin" is a trademark of Wuhan Deepin Technology.

This project is (currently) not affiliated with, funded, or in any way associated with Wuhan Deepin Technology.

THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW. EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OFALL NECESSARY SERVICING, REPAIR OR CORRECTION.

