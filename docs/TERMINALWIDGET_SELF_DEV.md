# 终端组件自研化重构 — 变更说明

> 分支：`feat/terminalwidget-self-rework`（基于 master `ab675d8b`）
> 目标：将 `3rdparty/terminalwidget`（QTermWidget 0.14.1 内嵌拷贝，45 个 C++ 文件约 30452 行）重构为自研实现，以**源码形式直接编入主程序**（非库链接），功能与对外行为完全不变。

## 一、总体方案

- **编译期二选一**：CMake 开关 `BUILD_SELF_TERMWIDGET`（默认 OFF）。
  - OFF（现状默认）：编译 `3rdparty/terminalwidget` 三方库并链接，行为与历史版本一致。
  - ON：自研源码直接编入 `deepin-terminal` 主目标，不构建、不链接三方库。
- **上层兼容**：`TermWidget` 公开接口不变；三方模式继承 `QTermWidget`（零回归），自研模式持有 `DTerminalDriver` 驱动并原样转发（旧名委托方法为 D2 兼容例外）。
- **命名体系**：自研类名 D 前缀（DTK 风格）；文件名按功能整合、不与三方及既有文件同名。
- **数据资源**：配色与键盘布局数据保留内容、重命名归位；三方内部 UI（SearchBar）与翻译随三方库退休。
- **第三方源码仅作行为参考**：上游部分结构级重写（自研命名与实现）；deepin 自有贡献（"Modify by/Add by" 标记）直接移植并统一改名。

## 二、构建与验证

```bash
# 三方模式（默认，行为不变）
cmake -B build -DCMAKE_BUILD_TYPE=Debug && make -j16

# 自研模式（源码直编主程序）
cmake -B build-self -DCMAKE_BUILD_TYPE=Debug -DBUILD_SELF_TERMWIDGET=ON && make -j16
```

| 验证项 | 结果 |
|---|---|
| OFF 全量构建（主程序 + 测试二进制） | ✅ 通过 |
| ON 全量构建（主程序 + 测试二进制） | ✅ 通过 |
| ON 离屏冒烟（offscreen 启动 12s） | ✅ 启动无崩溃 |
| OFF 单元测试（426/427） | 唯一失败为 master 基线同样失败的环境性计时用例（墙钟回拨），非本次引入 |

## 三、文件变更清单

### 新增（`src/terminalwidget/`，24 个源码文件 + 数据目录）

| 文件 | 说明 |
|---|---|
| `dterminaldriver.h/.cpp` | 驱动接口（双模式共用）：统一抽象终端组件能力 |
| `dterminalcompat.h/.cpp` | Konsole:: 兼容 shim（字体大小全局变量、光标形状枚举，仅自研模式） |
| `dterminalwidget.h/.cpp` | 自研门面：组装会话/解析/屏幕/显示/配色/搜索 |
| `dterminalutil.h/.cpp` | 宽字符宽度（DCharWidth）+ 显示宽度工具 + 字符单元基础类型 |
| `dterminalhistory.h/.cpp` | 历史缓冲（None/Buffer/File 三种存储归一） |
| `dtextfilter.h/.cpp` | 文本过滤链 + 终端图像扫描器 + OSC8 提取器 |
| `dterminalscreen.h/.cpp` | 屏幕缓冲（光标/滚动/选区/回绕） |
| `dkeyboardtranslator.h/.cpp` | 键盘映射（keytab 解析内置整合） |
| `dterminalparser.h/.cpp` | VT 解析状态机 + 组合字符序列表 |
| `dterminalpty.h/.cpp` | PTY 直实现（forkpty 语义 + 卸载检测/密码屏蔽） |
| `dshellsession.h/.cpp` | 会话（PTY+解析器串接）+ DProcessInfo + DSessionManager |
| `dencodingdetector.h/.cpp` | 编码识别与转换（deepin 自有模块移植） |
| `dcolorscheme.h/.cpp` | 配色解析 + 注册表 |
| `dhistorysearch.h/.cpp` | 历史搜索 |
| `dterminalview.h/.cpp` | 显示部件（渲染/选区/鼠标/IME/手势） |
| `data/schemes/`（21 个 .colorscheme） | 配色数据（内容保留） |
| `data/keytabs/`（5 个 .keytab） | 键盘布局数据（内容保留） |

### 修改

| 文件 | 变更 |
|---|---|
| `CMakeLists.txt` | `BUILD_SELF_TERMWIDGET` 开关；ON 源清单/链接（ICU/chardet/uchardet）/数据目录宏 |
| `tests/CMakeLists.txt` | 双轨源清单；ON 暂缓依赖三方内部类的 3 个用例（M5 后恢复）；自研头文件 glob 修正 |
| `src/views/termwidget.h/.cpp` | 组合化双模式改造（全部旧 API 经驱动转发） |
| `src/common/define.h` | `TERM_WIDGET_NAME` 双模式（焦点判定类名） |
| `src/main/mainwindow.cpp` | 配色列表入口改走 `TermWidget::availableColorSchemes()` |

### 删除

无（`3rdparty/terminalwidget` 保持冻结，按既定门槛：UT 全绿 + AT 清单 + 2 周灰度无 P0/P1 → 切默认 ON → 再留一个版本周期 → 删除）。

## 四、类与函数重命名映射

### 类名映射（三方 → 自研）

| 三方类 | 自研类 | 备注 |
|---|---|---|
| QTermWidget | DTerminalWidget | 门面（DTerminalDriver 实现） |
| （新） | DTerminalDriver | 双模式驱动接口 |
| （新） | DCompatTerminalDriver | 三方适配器（仅 OFF 编译） |
| TerminalDisplay + ScreenWindow | DTerminalView | 显示 + 滚动窗口整合 |
| Screen + BlockArray | DTerminalScreen | 屏幕缓冲 |
| Character / CharacterColor / ColorEntry | DTerminalChar / DTerminalCellColor / DTerminalPaletteEntry | 字符单元基础类型 |
| HistoryScroll + HistoryType（两级抽象） | DTerminalHistory（单一体系） | None/Buffer/File |
| Emulation + Vt102Emulation | DTerminalParser | 解析状态机 |
| ExtendedCharTable | DCharSequenceTable | 组合字符序列表 |
| KeyboardTranslator（+Reader/Writer/Manager） | DKeyboardTranslator（解析内置） | 键盘映射 |
| Pty + KPty* + KProcess | DTerminalPty | PTY 直实现 |
| Session | DShellSession | 会话 |
| ProcessInfo | DProcessInfo | 进程信息 |
| SessionManager | DSessionManager | 会话注册表 |
| DetectCode | DEncodingDetector | 编码检测（deepin 自有移植） |
| ColorScheme（+Manager） | DColorScheme（+DColorSchemeRegistry） | 配色 |
| HistorySearch | DHistorySearch | 搜索 |
| Filter / RegExpFilter / UrlFilter / FilterChain | DTextFilter / DRegexFilter / DUrlFilter / DTextFilterChain | 过滤链 |
| Filter::HotSpot | DTextFilter::Spot（DUrlSpot） | 热点 |
| TerminalImageFilterChain | DTerminalTextScanner | 图像扫描器 |
| EscapeSequenceUrlExtractor | DUrlExtractor | OSC8 提取器 |

### 关键函数映射（代表性）

| 三方 | 自研 | 说明 |
|---|---|---|
| QTermWidget::setHistorySize | DTerminalDriver::setHistoryLimit | 0=不保留；<0=文件；>0=内存 |
| sendText / startShellProgram | sendToShell / startShell | |
| setColorScheme | applyColorScheme | 加载/重载/回退默认 |
| setTrackOutput / isAtEndOfOutput | setFollowOutput / atOutputEnd | |
| getIsAllowScroll / setIsAllowScroll | allowScrollOnOutput / setAllowScrollOnOutput | |
| setNoHasSelect | resetSelectionGuard | 搜索框选区守卫 |
| search / clearSelection / setSelectionAll | searchText / resetSelection / selectAllText | |
| setBackspaceMode/setDeleteMode(char*,int) | setBackspaceSequence/setDeleteSequence | |
| Screen::displayCharacter | DTerminalScreen::displayCharacter | |
| Screen::resizeImage（zsh 修正） | DTerminalScreen::resizeImage（回调注入） | |
| Emulation::receiveData | DTerminalParser::feedData | 含 isCommandExec |
| Pty::dataReceived（卸载/密码屏蔽/乱码过滤） | DTerminalPty::handleIncomingData | deepin 自有移植 |
| TerminalDisplay::changedCursonPosition | DTerminalView::cursorPositionChanged | Ctrl+点击 |
| konsole_wcwidth characterWidth | DCharWidth::cellWidth | 常规模式改用系统 ICU 判定 |

## 五、主要逻辑调整说明

1. **驱动化双后端**：`TermWidget` 不再直接继承终端组件，而是通过 `DTerminalDriver` 访问；OFF 模式经 `DCompatTerminalDriver` 逐条透传三方实现，行为链路与原实现一致（含信号转发顺序）。
2. **历史存储归一**：三方"类型工厂 + 滚动缓冲"两级抽象合并为 `DTerminalHistory` 单一体系；文件存储以顺序读写替代 mmap 优化层（行为等价、实现更简）；临时文件前缀 `dterminal-`。
3. **宽字符宽度改用 ICU 判定**：常规模式经 `UCHAR_EAST_ASIAN_WIDTH` 与组合字符类判定，替代上游静态表（数据源为权威 Unicode 数据，行为等价）；GB18030 字体宽度修正表为 deepin 自有贡献，移植保留。
4. **VT 解析器结构重写**：词法分类与分发表按自研结构组织（TokenKind 枚举 + makeToken 编码）；行为契约与三方一致，包括 DECRQM 的 `$` 中间字节吸收、24 位真彩/256 色 SGR 子参数、OSC52 分块（64MB 上限）等 deepin 自有扩展。
5. **PTY 直实现**：整合"进程 + pty 设备 + 事件层"三级结构，posix_openpt + 子进程修饰器接管控制终端；卸载命令检测（remove/purge 正则）、expect 密码屏蔽、乱码输出过滤、非 UTF-8 分块缓冲均按原语义移植。
6. **显示部件整合**：`DTerminalView` 吸收屏幕窗口模型（滚动位置/输出跟踪内置于视图）；线绘字符当前经字体渲染（过程化绘制列为后续增强）；过滤链热点以虚线下划线呈现。
7. **搜索状态机**：`DHistorySearch` 保持正/反向、"正转反"位置换算与中文损失统计（BUG22626 修复移植），分块扫描 10K 行。
8. **会话上下文**：shell 提示符/当前命令经 `DSessionManager` 存取，供卸载检测与远程管理使用；卸载确认由门面回调注入上层对话框（同步语义）。
9. **编码转换**：chardet1 主识别、uchardet/ICU 交叉验证、中文环境 GB18030 优先；GB18030 2005/2022 差异码点替换保护。

## 六、已知简化与后续事项

- [ ] 线绘字符过程化绘制（当前字体渲染回退，常用盒线字符视觉一致）
- [ ] HTML 文本导出为简化版（上游 ConvertToHtml 未被上层使用）
- [ ] 翻译轮次：门面 URL 菜单动作的 tr() 字符串并入翻译文件
- [ ] ON 模式恢复 3 个暂缓的单元测试（依赖三方内部类的用例适配）
- [ ] 全架构构建验证（amd64/arm64/loongarch64/loong64/mips64el）
- [ ] 关键场景人工冒烟（见下）与性能基准

## 七、人工冒烟清单（GUI 环境）

- [ ] 启动 shell、命令执行与输出滚动（含智能滚动）
- [ ] 中文输入与 IME 预编辑、宽字符光标移动
- [ ] 配色切换（Light/Dark/Theme1-10/customTheme 重载）
- [ ] 选区（词/行/列/三击）、复制/粘贴（含括号粘贴模式）
- [ ] 搜索（正向/反向/中文、跨行）
- [ ] Ctrl+点击光标定位、Ctrl+滚轮缩放、触控板捏合缩放
- [ ] URL 识别（Ctrl+点击打开、右键打开/复制链接）
- [ ] OSC52 剪贴板（含大块分片）、标题参数更新
- [ ] 卸载检测（remove/purge 命令触发确认框）
- [ ] 远程管理 expect 密码屏蔽、编码切换（GB18030 等）
