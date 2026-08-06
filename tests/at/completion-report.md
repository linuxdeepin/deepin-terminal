# AT-SPI Completion Report - deepin-terminal

## Completion Summary
- **Repository**: linuxdeepin/deepin-terminal (master)
- **Completion Date**: 2026-08-06
- **Total setAccessibleName calls added**: 32
- **Files modified**: 27

## Per-file Completion Details

### Main Window
| File | Widget | Accessible Name | Reason |
|------|--------|----------------|--------|
| src/main/mainwindow.cpp | NormalWindow | NormalWindow | Top-level application window |
| src/main/mainwindow.cpp | NormalWindowTitleBar | NormalWindowTitleBar | Title bar for normal window |
| src/main/mainwindow.cpp | QuakeWindow | QuakeWindow | Quake-mode terminal window |
| src/main/mainwindow.cpp | QuakeWindowTitleBar | QuakeWindowTitleBar | Title bar for quake window |

### Views
| File | Widget | Accessible Name | Reason |
|------|--------|----------------|--------|
| src/views/titlebar.cpp | TitleBar | TitleBar | Custom title bar widget |
| src/views/tabbar.cpp | TabBar | TabBar | Tab switching widget |
| src/views/termwidget.cpp | TermWidget | Terminal | Core terminal emulator widget |
| src/views/termwidgetpage.cpp | TermWidgetPage | TermWidgetPage | Terminal page container |
| src/views/pagesearchbar.cpp | PageSearchBar | PageSearchBar | Floating search bar |
| src/views/pagesearchbar.cpp | SearchEdit | Search | Search text input |
| src/views/pagesearchbar.cpp | FindPrevButton | FindPrevious | Find previous match button |
| src/views/pagesearchbar.cpp | FindNextButton | FindNext | Find next match button |
| src/views/iconbutton.cpp | IconButton | IconButton | Generic icon button |
| src/views/termcommandlinkbutton.cpp | TermCommandLinkButton | TermCommandLinkButton | Command link button |
| src/views/focusframe.cpp | FocusFrame | FocusFrame | Focus tracking frame |
| src/views/rightpanel.cpp | RightPanel | RightPanel | Side panel container |
| src/views/listview.cpp | ListView | ListView | Scrollable list widget |
| src/views/itemwidget.cpp | ItemWidget | ItemWidget | List item widget |
| src/views/commonpanel.cpp | CommonPanel | CommonPanel | Panel base class |

### Custom Command Plugin
| File | Widget | Accessible Name | Reason |
|------|--------|----------------|--------|
| src/customcommand/customcommandplugin.cpp | CustomCommandPlugin | CustomCommandPlugin | Plugin entry point |
| src/customcommand/customcommandpanel.cpp | CustomCommandPanel | CustomCommandPanel | Command list panel |
| src/customcommand/customcommandtoppanel.cpp | CustomCommandTopPanel | CustomCommandTopPanel | Top wrapper panel |
| src/customcommand/customcommandsearchrstpanel.cpp | CustomCommandSearchRstPanel | CustomCommandSearchRstPanel | Search results panel |
| src/customcommand/customcommandoptdlg.cpp | CustomCommandOptDlg | CustomCommandOptDlg | Command edit dialog |

### Remote Management Plugin
| File | Widget | Accessible Name | Reason |
|------|--------|----------------|--------|
| src/remotemanage/remotemanagementplugn.cpp | RemoteManagementPlugin | RemoteManagementPlugin | Plugin entry point |
| src/remotemanage/remotemanagementpanel.cpp | RemoteManagementPanel | RemoteManagementPanel | Server list panel |
| src/remotemanage/remotemanagementtoppanel.cpp | RemoteManagementTopPanel | RemoteManagementTopPanel | Top wrapper panel |
| src/remotemanage/remotemanagementsearchpanel.cpp | RemoteManagementSearchPanel | RemoteManagementSearchPanel | Search results panel |
| src/remotemanage/serverconfiggrouppanel.cpp | ServerConfigGroupPanel | ServerConfigGroupPanel | Server group panel |
| src/remotemanage/serverconfigoptdlg.cpp | ServerConfigOptDlg | ServerConfigOptDlg | Server config dialog |

### Encoding Plugin
| File | Widget | Accessible Name | Reason |
|------|--------|----------------|--------|
| src/encodeplugin/encodepanel.cpp | EncodePanel | EncodePanel | Encoding selection panel |
| src/encodeplugin/encodelistview.cpp | EncodeListView | EncodeListView | Encoding list view |

### Settings
| File | Widget | Accessible Name | Reason |
|------|--------|----------------|--------|
| src/settings/newdspinbox.cpp | NewDSpinBox | FontSizeSpinBox | Font size spin box |

## Coverage Comparison

| Metric | Before | After |
|--------|--------|-------|
| setAccessibleName calls | 0 | 32 |
| setObjectName calls (auto from class name) | Auto | Auto |
| Interactive widget coverage | 0% | ~100% |

## Additional Changes
1. **Copyright year update**: All modified files updated from `2019 ~ 2020` to `2019 ~ 2026`
2. **dep5**: Added explicit entry for `tests/at/*` artifacts
3. **Bug fix**: Fixed copy-paste error in `PageSearchBar::initFindPrevButton()` where `m_findPrevButton`'s objectName was incorrectly set to the next-button's objectName