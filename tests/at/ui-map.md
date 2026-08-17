# deepin-terminal UI Map (AT-SPI)

## 组件结构 Mermaid

```mermaid
graph TD
    subgraph 主窗口
        NormalWindow["NormalWindow (frame)"]
        NormalWindow --> TitleBar["TitleBar (panel)"]
        NormalWindow --> TabBar["TabBar (panel)"]
        NormalWindow --> Terminal["Terminal (panel)"]
        NormalWindow --> RightPanel["RightPanel (panel)"]
        NormalWindow --> PageSearchBar["PageSearchBar (panel)"]
        NormalWindow --> TermWidgetPage["TermWidgetPage (panel)"]
        NormalWindow --> MainWindowCentralQWidget["MainWindowCentralQWidget"]
        
        TitleBar --> TitleBarLayout
        TabBar --> TabBarRightMenu["TabBarRightMenu (DMenu)"]
        TabBar --> TabBarCloseTabAction["TabBarCloseTabAction"]
        TabBar --> TabBarCloseOtherTabAction["TabBarCloseOtherTabAction"]
        TabBar --> TabTitleRenameAction["TabTitleRenameAction"]
        
        PageSearchBar --> SearchEdit["SearchEdit (DLineEdit)"]
        PageSearchBar --> FindNextBtn["FindNext (DIconButton)"]
        PageSearchBar --> FindPrevBtn["FindPrevious (DIconButton)"]
        
        RightPanel --> CommonPanel["CommonPanel"]
        RightPanel --> CustomCommandPanel["CustomCommandPanel"]
        RightPanel --> RemoteManagementPanel["RemoteManagementPanel"]
    end
    
    subgraph 设置对话框
        Settings["Settings (container)"]
        Settings --> SettingsFontComboBox["SettingsFontFamilyComboBox"]
        Settings --> NewDspinBox["FontSizeSpinBox (NewDSpinBox)"]
    end
    
    subgraph 右键菜单(workspace)
        RightClick1["右键菜单 (terminal)"]
        RightClick1 --> Copy["复制"]
        RightClick1 --> Paste["粘贴"]
        RightClick1 --> HSplit["横向分屏"]
        RightClick1 --> VSplit["纵向分屏"]
        RightClick1 --> CloseWS["关闭工作区"]
        RightClick1 --> NewTab["新建标签页"]
        RightClick1 --> Fullscreen["全屏/退出全屏"]
        RightClick1 --> Search["搜索"]
        RightClick1 --> Encoding["编码方式"]
        RightClick1 --> CustomCMD["自定义命令"]
        RightClick1 --> RemoteMgr["远程管理"]
        RightClick1 --> Settings2["设置"]
    end
    
    subgraph 右键菜单(tab)
        RightClick2["右键菜单 (tab)"]
        RightClick2 --> CloseTab["关闭标签页"]
        RightClick2 --> CloseOthers["关闭其他标签页"]
        RightClick2 --> RenameTab["重命名标题"]
    end
    
    subgraph 主菜单(DTitlebarMainMenu)
        MainMenu["DTitlebarMainMenu"]
        MainMenu --> NewWindow["新建窗口"]
        MainMenu --> CustomCMD2["自定义命令"]
        MainMenu --> RemoteMgr2["远程管理"]
        MainMenu --> Settings3["设置"]
        MainMenu --> Theme["主题"]
        MainMenu --> Help["帮助"]
        MainMenu --> About["关于"]
        MainMenu --> Quit["退出"]
    end
    
    subgraph 编码面板
        EncodePanel["EncodePanel"]
        EncodePanel --> EncodeListView["EncodeListView"]
    end
    
    subgraph 自定义命令相关
        CustomCommandPanel --> CustomCommandTopPanel["CustomCommandTopPanel"]
        CustomCommandPanel --> CustomCommandSearchRstPanel["CustomCommandSearchRstPanel"]
        CustomCommandOptDlg["CustomCommandOptDlg (dialog)"]
    end
    
    subgraph 远程管理相关
        RemoteManagementPanel --> RemoteManagementTopPanel["RemoteManagementTopPanel"]
        RemoteManagementPanel --> RemoteManagementSearchPanel["RemoteManagementSearchPanel"]
        ServerConfigGroupPanel["ServerConfigGroupPanel"]
        ServerConfigOptDlg["ServerConfigOptDlg (dialog)"]
    end
    
    subgraph 其他
        IconButton["IconButton"]
        TermCommandLinkButton["TermCommandLinkButton"]
        FocusFrame["FocusFrame"]
        ItemWidget["ItemWidget"]
        ListView["ListView"]
        TabRenameDlg["TabRenameDlg (dialog)"]
    end
```

## 控件表

| AT-SPI Name | Role | 所属 | 类型 | 源文件 |
|------------|------|------|------|--------|
| NormalWindow | frame | MainWindow | 主窗口 | src/main/mainwindow.cpp |
| NormalWindowTitleBar | panel | TitleBar | 标题栏实例 | src/main/mainwindow.cpp |
| QuakeWindow | frame | MainWindow | 雷神窗口 | src/main/mainwindow.cpp |
| QuakeWindowTitleBar | panel | TitleBar | 雷神标题栏 | src/main/mainwindow.cpp |
| TitleBar | panel | TitleBar | 标题栏 | src/views/titlebar.cpp |
| TabBar | panel | TabBar | 标签栏 | src/views/tabbar.cpp |
| Terminal | panel | TermWidget | 终端区域 | src/views/termwidget.cpp |
| TermWidgetPage | panel | TermWidgetPage | 终端页面 | src/views/termwidgetpage.cpp |
| PageSearchBar | panel | PageSearchBar | 搜索栏 | src/views/pagesearchbar.cpp |
| Search | panel | DLineEdit | 搜索输入框 | src/views/pagesearchbar.cpp |
| FindNext | panel | DIconButton | 向下搜索 | src/views/pagesearchbar.cpp |
| FindPrevious | panel | DIconButton | 向上搜索 | src/views/pagesearchbar.cpp |
| IconButton | panel | IconButton | 图标按钮 | src/views/iconbutton.cpp |
| TermCommandLinkButton | panel | TermCommandLinkButton | 命令链接按钮 | src/views/termcommandlinkbutton.cpp |
| FocusFrame | panel | FocusFrame | 焦点框 | src/views/focusframe.cpp |
| RightPanel | panel | RightPanel | 右侧面板 | src/views/rightpanel.cpp |
| ListView | panel | ListView | 列表视图 | src/views/listview.cpp |
| ItemWidget | panel | ItemWidget | 列表项 | src/views/itemwidget.cpp |
| CommonPanel | panel | CommonPanel | 通用面板 | src/views/commonpanel.cpp |
| CustomCommandPanel | panel | CustomCommandPanel | 自定义命令面板 | src/customcommand/customcommandpanel.cpp |
| CustomCommandTopPanel | panel | CustomCommandTopPanel | 自定义命令顶栏 | src/customcommand/customcommandtoppanel.cpp |
| CustomCommandSearchRstPanel | panel | CustomCommandSearchRstPanel | 自定义命令搜索 | src/customcommand/customcommandsearchrstpanel.cpp |
| CustomCommandOptDlg | panel | DDialog | 自定义命令对话框 | src/customcommand/customcommandoptdlg.cpp |
| RemoteManagementPanel | panel | RemoteManagementPanel | 远程管理面板 | src/remotemanage/remotemanagementpanel.cpp |
| RemoteManagementTopPanel | panel | RemoteManagementTopPanel | 远程管理顶栏 | src/remotemanage/remotemanagementtoppanel.cpp |
| RemoteManagementSearchPanel | panel | RemoteManagementSearchPanel | 远程管理搜索 | src/remotemanage/remotemanagementsearchpanel.cpp |
| ServerConfigGroupPanel | panel | ServerConfigGroupPanel | 服务器分组面板 | src/remotemanage/serverconfiggrouppanel.cpp |
| ServerConfigOptDlg | panel | DDialog | 服务器配置对话框 | src/remotemanage/serverconfigoptdlg.cpp |
| EncodePanel | panel | EncodePanel | 编码面板 | src/encodeplugin/encodepanel.cpp |
| EncodeListView | panel | EncodeListView | 编码列表 | src/encodeplugin/encodelistview.cpp |
| FontSizeSpinBox | panel | NewDSpinBox | 字体大小调节 | src/settings/newdspinbox.cpp |

## 主菜单 (DTitlebarMainMenu)

| 菜单项 | 快捷键 | 说明 |
|-------|--------|------|
| 新建窗口 | Ctrl+Shift+N | 打开新终端窗口 |
| 自定义命令 | Alt+Ins | 打开自定义命令面板 |
| 远程管理 | Alt+Del | 打开远程管理面板 |
| 设置 | - | 打开设置对话框 |
| 主题 > 浅色 | - | 应用浅色主题 |
| 主题 > 深色 | - | 应用深色主题 |
| 主题 > 跟随系统 | - | 跟随系统主题 |
| 帮助 | - | 打开帮助文档 |
| 关于 | - | 关于应用 |
| 退出 | Ctrl+Q | 退出应用 |

## 右键菜单（终端区域）

| 菜单项 | 说明 |
|-------|------|
| 复制 | 复制选中文字 |
| 粘贴 | 粘贴剪贴板内容 |
| 打开 | 打开文件/链接 |
| 在文件管理器中打开 | 打开所在目录 |
| 横向分屏 | 上下分屏 |
| 纵向分屏 | 左右分屏 |
| 关闭工作区 | 关闭当前工作区 |
| 关闭其他工作区 | 关闭其他工作区 |
| 新建标签页 | 新建标签 |
| 全屏/退出全屏 | 切换全屏模式 |
| 搜索 | 打开搜索栏 |
| 编码方式 | 设置编码 |
| 自定义命令 | 打开自定义命令 |
| 远程管理 | 打开远程管理 |
| 设置 | 打开设置 |
| 上传文件 | 上传到远程服务器 |
| 下载文件 | 从远程服务器下载 |

## 标签页右键菜单

| 菜单项 | 说明 |
|-------|------|
| 关闭标签页 | 关闭当前标签 |
| 关闭其他标签页 | 关闭其他标签 |
| 重命名标题 | 重命名标签标题 |

## 快捷键索引

| 快捷键 | 功能 | 源文件 |
|-------|------|--------|
| Ctrl+Alt+T | 启动终端 | 系统 |
| Ctrl+Shift+N | 新建窗口 | src/main/mainwindow.cpp |
| Alt+Ins | 自定义命令 | src/main/mainwindow.cpp |
| Alt+Del | 远程管理 | src/main/mainwindow.cpp |
| Ctrl+Q | 退出 | src/main/mainwindow.cpp |
| Ctrl+Shift+H | 横向分屏 | src/views/termwidget.cpp |
| Ctrl+Shift+J | 纵向分屏 | src/views/termwidget.cpp |
| Ctrl+Alt+F | 打开搜索 | src/views/termwidget.cpp |
| Shift+Enter | 向上搜索 | src/views/pagesearchbar.cpp |
| Enter | 向下搜索 | src/views/pagesearchbar.cpp |
| Ctrl+S | 流控制暂停 | src/settings |
| Ctrl+Q | 流控制恢复 | src/settings |