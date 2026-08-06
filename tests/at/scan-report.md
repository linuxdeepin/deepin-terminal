# AT-SPI Scan Report - deepin-terminal

## Scan Method
- Manual source code static analysis
- Target: linuxdeepin/deepin-terminal (branch: master)
- Date: 2026-08-06

## UI Component Overview

The application consists of:
1. **MainWindow** (NormalWindow / QuakeWindow) - Top-level container
2. **TitleBar** - Custom navigation bar with tabs
3. **TabBar** - Tab switching for terminal sessions
4. **TermWidgetPage** - Page container for terminal splits
5. **TermWidget** - The terminal emulator widget (core interactive element)
6. **PageSearchBar** - Floating search bar (find text in terminal)
7. **RightPanel** - Base container for side panels
8. **CommonPanel** - Base for plugin panels (custom command, remote management, encoding)

## AT-SPI Gap Analysis

### Critical gaps (interactive elements without setAccessibleName)

| Widget | Location | File | Usage |
|--------|----------|------|-------|
| TermWidget | Terminal emulator | src/views/termwidget.cpp | Core interaction - typing, reading output |
| TabBar | Tab switching | src/views/tabbar.cpp | Tab navigation |
| TitleBar | Custom title bar | src/views/titlebar.cpp | Contains tabs and window controls |
| PageSearchBar | Search bar | src/views/pagesearchbar.cpp | Find text in terminal |
| SearchEdit | Search input | src/views/pagesearchbar.cpp | Text input for search |
| FindNextButton | Next match | src/views/pagesearchbar.cpp | Find next result |
| FindPrevButton | Previous match | src/views/pagesearchbar.cpp | Find previous result |
| IconButton | Generic icon button | src/views/iconbutton.cpp | Used in panels for actions |
| TermCommandLinkButton | Link button | src/views/termcommandlinkbutton.cpp | "Advanced options" link |
| FocusFrame | Focus tracking frame | src/views/focusframe.cpp | Focus indicator for list items |
| ListView | Scrollable list | src/views/listview.cpp | Lists in remote/custom panels |
| ItemWidget | List item widget | src/views/itemwidget.cpp | Individual items in lists |
| CommonPanel | Panel base | src/views/commonpanel.cpp | Base class for side panels |
| RightPanel | Side panel base | src/views/rightpanel.cpp | Side panel container |
| NormalWindow | Main window | src/main/mainwindow.cpp | Primary application window |
| QuakeWindow | Drop-down window | src/main/mainwindow.cpp | Quake-mode terminal window |
| NormalWindowTitleBar | Title bar | src/main/mainwindow.cpp | Normal window title bar |
| QuakeWindowTitleBar | Title bar | src/main/mainwindow.cpp | Quake window title bar |

### Plugin panels
| CustomCommandPlugin | Plugin entry | src/customcommand/customcommandplugin.cpp | Custom commands |
| CustomCommandPanel | Panel | src/customcommand/customcommandpanel.cpp | Command list UI |
| CustomCommandTopPanel | Top wrapper | src/customcommand/customcommandtoppanel.cpp | Manages panel/search |
| CustomCommandSearchRstPanel | Search results | src/customcommand/customcommandsearchrstpanel.cpp | Search results UI |
| CustomCommandOptDlg | Add/edit dialog | src/customcommand/customcommandoptdlg.cpp | Command editing dialog |
| RemoteManagementPlugin | Plugin entry | src/remotemanage/remotemanagementplugn.cpp | Remote management |
| RemoteManagementPanel | Panel | src/remotemanage/remotemanagementpanel.cpp | Server list UI |
| RemoteManagementTopPanel | Top wrapper | src/remotemanage/remotemanagementtoppanel.cpp | Manages panel/search |
| RemoteManagementSearchPanel | Search results | src/remotemanage/remotemanagementsearchpanel.cpp | Search results UI |
| ServerConfigGroupPanel | Group panel | src/remotemanage/serverconfiggrouppanel.cpp | Server group UI |
| ServerConfigOptDlg | Config dialog | src/remotemanage/serverconfigoptdlg.cpp | Server config dialog |
| EncodePanel | Encoding panel | src/encodeplugin/encodepanel.cpp | Encoding selection |
| EncodeListView | Encoding list | src/encodeplugin/encodelistview.cpp | Encoding list view |
| NewDSpinBox | Font size input | src/settings/newdspinbox.cpp | Font size control |

## Baseline Coverage
- setAccessibleName: 0 calls (0%)
- Components requiring accessible names: 32
- Recommended coverage target: 100% of interactive widgets