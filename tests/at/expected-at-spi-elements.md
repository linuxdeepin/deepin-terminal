# Expected AT-SPI Elements for deepin-terminal

## 生成方式
通过源码静态分析推导。采用 remote-codebase MCP + 本地源码 grep 交叉验证。

## 预期元素列表

| 行号 | AT-SPI Name | Role | Widget Class | 文件 | 触发条件 | 推导链 |
|-----|-------------|------|-------------|------|---------|--------|
| 1 | NormalWindow | frame | NormalWindow | src/main/mainwindow.cpp:2996 | 普通窗口模式 | setAccessibleName("NormalWindow") |
| 2 | NormalWindowTitleBar | panel | TitleBar | src/main/mainwindow.cpp:3015 | NormalWindow 初始化 | m_titleBar->setAccessibleName("NormalWindowTitleBar") |
| 3 | QuakeWindow | frame | QuakeWindow | src/main/mainwindow.cpp:3225 | 雷神窗口模式 | setAccessibleName("QuakeWindow") |
| 4 | QuakeWindowTitleBar | panel | TitleBar | src/main/mainwindow.cpp:3255 | QuakeWindow 初始化 | m_titleBar->setAccessibleName("QuakeWindowTitleBar") |
| 5 | TitleBar | panel | TitleBar | src/views/titlebar.cpp:33 | 标题栏创建 | setAccessibleName("TitleBar") |
| 6 | TabBar | panel | TabBar | src/views/tabbar.cpp:184 | 标签栏创建 | setAccessibleName("TabBar") |
| 7 | Terminal | panel | TermWidget | src/views/termwidget.cpp:52 | 终端小部件创建 | setAccessibleName("Terminal") |
| 8 | TermWidgetPage | panel | TermWidgetPage | src/views/termwidgetpage.cpp:48 | 终端页面创建 | setAccessibleName("TermWidgetPage") |
| 9 | PageSearchBar | panel | PageSearchBar | src/views/pagesearchbar.cpp:29 | 打开搜索栏 | setAccessibleName("PageSearchBar") |
| 10 | Search | panel | DLineEdit | src/views/pagesearchbar.cpp:220 | PageSearchBar 创建 | m_searchEdit->setAccessibleName("Search") |
| 11 | FindNext | panel | DIconButton | src/views/pagesearchbar.cpp:195 | PageSearchBar 创建 | m_findNextButton->setAccessibleName("FindNext") |
| 12 | FindPrevious | panel | DIconButton | src/views/pagesearchbar.cpp:174 | PageSearchBar 创建 | m_findPrevButton->setAccessibleName("FindPrevious") |
| 13 | IconButton | panel | IconButton | src/views/iconbutton.cpp:19 | 图标按钮创建 | setAccessibleName("IconButton") |
| 14 | TermCommandLinkButton | panel | TermCommandLinkButton | src/views/termcommandlinkbutton.cpp:21 | 命令链接按钮创建 | setAccessibleName("TermCommandLinkButton") |
| 15 | FocusFrame | panel | FocusFrame | src/views/focusframe.cpp:28 | 焦点框创建 | setAccessibleName("FocusFrame") |
| 16 | RightPanel | panel | RightPanel | src/views/rightpanel.cpp:33 | 右侧面板创建 | setAccessibleName("RightPanel") |
| 17 | ListView | panel | ListView | src/views/listview.cpp:30 | 列表视图创建 | setAccessibleName("ListView") |
| 18 | ItemWidget | panel | ItemWidget | src/views/itemwidget.cpp:46 | 列表项创建 | setAccessibleName("ItemWidget") |
| 19 | CommonPanel | panel | CommonPanel | src/views/commonpanel.cpp:24 | 通用面板创建 | setAccessibleName("CommonPanel") |
| 20 | CustomCommandPanel | panel | CustomCommandPanel | src/customcommand/customcommandpanel.cpp:28 | 自定义命令面板 | setAccessibleName("CustomCommandPanel") |
| 21 | CustomCommandTopPanel | panel | CustomCommandTopPanel | src/customcommand/customcommandtoppanel.cpp:29 | 自定义命令顶栏 | setAccessibleName("CustomCommandTopPanel") |
| 22 | CustomCommandSearchRstPanel | panel | CustomCommandSearchRstPanel | src/customcommand/customcommandsearchrstpanel.cpp:31 | 搜索结果显示 | setAccessibleName("CustomCommandSearchRstPanel") |
| 23 | CustomCommandOptDlg | panel | DDialog | src/customcommand/customcommandoptdlg.cpp:47 | 添加/编辑命令弹窗 | setAccessibleName("CustomCommandOptDlg") |
| 24 | RemoteManagementPanel | panel | RemoteManagementPanel | src/remotemanage/remotemanagementpanel.cpp:23 | 远程管理面板 | setAccessibleName("RemoteManagementPanel") |
| 25 | RemoteManagementTopPanel | panel | RemoteManagementTopPanel | src/remotemanage/remotemanagementtoppanel.cpp:22 | 远程管理顶栏 | setAccessibleName("RemoteManagementTopPanel") |
| 26 | RemoteManagementSearchPanel | panel | RemoteManagementSearchPanel | src/remotemanage/remotemanagementsearchpanel.cpp:26 | 远程管理搜索 | setAccessibleName("RemoteManagementSearchPanel") |
| 27 | ServerConfigGroupPanel | panel | ServerConfigGroupPanel | src/remotemanage/serverconfiggrouppanel.cpp:26 | 服务器分组面板 | setAccessibleName("ServerConfigGroupPanel") |
| 28 | ServerConfigOptDlg | panel | DDialog | src/remotemanage/serverconfigoptdlg.cpp:63 | 服务器配置弹窗 | setAccessibleName("ServerConfigOptDlg") |
| 29 | EncodePanel | panel | EncodePanel | src/encodeplugin/encodepanel.cpp:24 | 编码面板 | setAccessibleName("EncodePanel") |
| 30 | EncodeListView | panel | EncodeListView | src/encodeplugin/encodelistview.cpp:31 | 编码列表 | setAccessibleName("EncodeListView") |
| 31 | FontSizeSpinBox | panel | NewDSpinBox | src/settings/newdspinbox.cpp:23 | 字体大小调节 | setAccessibleName("FontSizeSpinBox") |

## 运行时命名元素（AT-SPI 树中出现的名称）

| AT-SPI Name | Role | 说明 |
|-------------|------|------|
| DTitlebarMainMenu | popup menu | 标题栏主菜单容器 |
| 新建窗口 | menu item | 主菜单项 |
| 自定义命令 | menu item | 主菜单项/右键菜单项 |
| 远程管理 | menu item | 主菜单项/右键菜单项 |
| 设置 | menu item | 主菜单项/右键菜单项 |
| 主题 | menu item | 主菜单项（有子菜单） |
| 帮助 | menu item | 主菜单项 |
| 关于 | menu item | 主菜单项 |
| 退出 | menu item | 主菜单项 |
| 浅色 | menu item | 主题子菜单 |
| 深色 | menu item | 主题子菜单 |
| 跟随系统 | menu item | 主题子菜单 |

## 统计

- **总预期 AT-SPI 命名元素**: 31 (setAccessibleName)
- **运行时命名元素**: 12+ (菜单项)
- **setObjectName 备用名称**: 约 40+ 个（作为 selector.name 备选）
- **无命名元素缺口**: 约 10 个内部布局/容器（不直接影响测试）