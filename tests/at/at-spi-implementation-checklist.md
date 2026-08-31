# AT-SPI Implementation Checklist for deepin-terminal

## 覆盖状态

**总 UI 控件类**: 约 40+ 个
**已补 setAccessibleName**: 31 个
**覆盖率**: ~75%（主要控件已覆盖）

## 已实现 — 无需修改

以下控件已通过 commit `c2a720cb` 补全了 setAccessibleName：

| AT-SPI Name | 文件 | 实现方式 |
|-------------|------|---------|
| NormalWindow | src/main/mainwindow.cpp | setAccessibleName |
| NormalWindowTitleBar | src/main/mainwindow.cpp | setAccessibleName |
| QuakeWindow | src/main/mainwindow.cpp | setAccessibleName |
| QuakeWindowTitleBar | src/main/mainwindow.cpp | setAccessibleName |
| TitleBar | src/views/titlebar.cpp | setAccessibleName |
| TabBar | src/views/tabbar.cpp | setAccessibleName |
| Terminal | src/views/termwidget.cpp | setAccessibleName |
| TermWidgetPage | src/views/termwidgetpage.cpp | setAccessibleName |
| PageSearchBar | src/views/pagesearchbar.cpp | setAccessibleName |
| Search | src/views/pagesearchbar.cpp | setAccessibleName on child |
| FindNext | src/views/pagesearchbar.cpp | setAccessibleName on child |
| FindPrevious | src/views/pagesearchbar.cpp | setAccessibleName on child |
| IconButton | src/views/iconbutton.cpp | setAccessibleName |
| TermCommandLinkButton | src/views/termcommandlinkbutton.cpp | setAccessibleName |
| FocusFrame | src/views/focusframe.cpp | setAccessibleName |
| RightPanel | src/views/rightpanel.cpp | setAccessibleName |
| ListView | src/views/listview.cpp | setAccessibleName |
| ItemWidget | src/views/itemwidget.cpp | setAccessibleName |
| CommonPanel | src/views/commonpanel.cpp | setAccessibleName |
| CustomCommandPanel | src/customcommand/customcommandpanel.cpp | setAccessibleName |
| CustomCommandTopPanel | src/customcommand/customcommandtoppanel.cpp | setAccessibleName |
| CustomCommandSearchRstPanel | src/customcommand/customcommandsearchrstpanel.cpp | setAccessibleName |
| CustomCommandOptDlg | src/customcommand/customcommandoptdlg.cpp | setAccessibleName |
| RemoteManagementPanel | src/remotemanage/remotemanagementpanel.cpp | setAccessibleName |
| RemoteManagementTopPanel | src/remotemanage/remotemanagementtoppanel.cpp | setAccessibleName |
| RemoteManagementSearchPanel | src/remotemanage/remotemanagementsearchpanel.cpp | setAccessibleName |
| ServerConfigGroupPanel | src/remotemanage/serverconfiggrouppanel.cpp | setAccessibleName |
| ServerConfigOptDlg | src/remotemanage/serverconfigoptdlg.cpp | setAccessibleName |
| EncodePanel | src/encodeplugin/encodepanel.cpp | setAccessibleName |
| EncodeListView | src/encodeplugin/encodelistview.cpp | setAccessibleName |
| FontSizeSpinBox | src/settings/newdspinbox.cpp | setAccessibleName |

## 建议补充（下一轮修复）

以下控件仅有 setObjectName 而无 setAccessibleName，建议补充：

| # | 建议名称 | 位置 | 当前标识 | 优先级 | 建议代码 |
|---|---------|------|---------|-------|---------|
| 1 | CustomCommandPanelSearchEdit | src/customcommand/customcommandpanel.cpp:227 | objectName="CustomSearchEdit" | P1 | `m_searchEdit->setAccessibleName("CustomCommandPanelSearchEdit");` |
| 2 | CustomCommandListWidget | src/customcommand/customcommandpanel.cpp:232 | objectName="CustomCommandListWidget" | P1 | `m_cmdListWidget->setAccessibleName("CustomCommandListWidget");` |
| 3 | CustomCommandAddButton | src/customcommand/customcommandpanel.cpp:235 | objectName="CustomAddCommandButton" | P1 | `m_pushButton->setAccessibleName("CustomCommandAddButton");` |
| 4 | TabRenameDialog | src/views/tabrenamedlg.cpp | 无 | P2 | 在构造函数中加 `setAccessibleName("TabRenameDlg");`（已有 objectName） |
| 5 | SettingsDialog | src/settings/settings.cpp | 无 | P2 | 在 init 中加 `setAccessibleName("SettingsDialog");` |

**注意**: 上述仅为建议。当前 31 个 setAccessibleName 已覆盖 AT 自动化测试的核心需求。缺少的这些虽有助于精确定位，但不阻塞测试。

## 验证方式

1. 启动应用后通过 `python3 -c "import pyatspi; ..."` 或 `accerciser` 查看 AT-SPI 树
2. 确认每个预期元素在正确角色下可见
3. 菜单项在菜单打开时动态出现（不在静态树中）