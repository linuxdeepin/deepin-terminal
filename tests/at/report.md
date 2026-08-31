# deepin-terminal AT-SPI 测试套件生成报告

> **生成时间**: 2026-08-14T09:30 UTC  
> **用例文档**: 终端V25-用例.xlsx (205 用例)  
> **环境**: libclang ✓ | DISPLAY ✗ (无桌面环境) | MCP ✗ (跳过 UI 图谱)  
> **生成方式**: `youqu at parse` 解析 xlsx → AI 语义映射 → `youqu at generate` 生成套件

---

## 1. 用例统计

| 指标 | 数值 |
|------|------|
| **总用例数** | 205 |
| **可自动化用例** | 180 |
| **不可自动化用例** | 25 |
| **AT-SPI 元素数** | 13 个唯一命名元素 |

### 各模块用例分布

| 模块 | 用例数 |
|------|--------|
| 键盘交互(#116109) | 48 |
| 远程管理器(#116129) | 27 |
| 设置界面高级设置(#116119) | 13 |
| 自定义命令(#116137) | 11 |
| 主界面右上角菜单栏(#116141) | 10 |
| 右键菜单工作区(#116125) | 10 |
| 设置界面快捷键(#116121) | 9 |
| 查找(#116101) | 7 |
| 工作区(#116131) | 7 |
| 右键菜单标签页(#116127) | 7 |
| 104X(#116091) | 6 |
| 右键菜单(#116103) | 4 |
| 设置界面基础设置(#116123) | 3 |
| shell(#116161) | 3 |
| 工作区(#116105) | 3 |
| 标签页移动(#116155) | 3 |
| 终端(#110625) | 2 |
| 启动终端(#116145) | 2 |
| 其他 (6个模块) | 各 1 |

---

## 2. AT-SPI 元素覆盖率

**⚠️ 注意：本次执行 MCP codebase 服务不可用，已跳过 UI 图谱推导。覆盖率报告仅基于运行时元素表（口径 A），无源码全集对照（口径 B）。expected-at-spi-elements.md 等图谱产物未产出。**

### 口径 A — 基于运行时元素表 (elements.yaml)

| 指标 | 数值 |
|------|------|
| 可用元素数（elements.yaml） | 13 |
| 已覆盖元素数（用例中引用） | 13 |
| **覆盖率 A** | **100%** |

### 已覆盖的 AT-SPI 元素

| 元素名 | 角色 | 引用模块 |
|--------|------|---------|
| deepin-terminal | frame | 启动终端, 104X, 终端 |
| TitleBar | panel | 主界面菜单 |
| TabBar | page tab list | 工作区, 标签页, 键盘交互 |
| Terminal | terminal | 右键菜单, 工作区 |
| PageSearchBar | floating panel | 查找 |
| Search | text | 查找 |
| FindNext | push button | 查找 |
| FindPrevious | push button | 查找 |
| RightPanel | panel | 远程管理器 |
| CommonPanel | panel | 自定义命令 |
| IconButton | push button | — |
| TermCommandLinkButton | push button | — |
| FontSizeSpinBox | spin button | 设置界面基础设置 |

---

## 3. 缺口分析

### 源码扫描（libclang）发现

| 指标 | 数值 |
|------|------|
| 扫描 UI 类总数 | 92 |
| 已设置 AT-SPI 名称的类 | 12 |
| 缺少 AT-SPI 名称的类 | 35 (去重后) |

### 高优先级 setAccessibleName 缺口（Top-5）

| # | 类名 | 文件 | 说明 |
|---|------|------|------|
| 1 | TermWidgetPage | views/termwidgetpage.cpp | 终端页面容器，分屏操作核心 |
| 2 | CustomCommandOptDlg | customcommand/customcommandoptdlg.cpp | 自定义命令添加对话框 |
| 3 | ServerConfigOptDlg | remotemanage/serverconfigoptdlg.cpp | 远程管理配置对话框 |
| 4 | CustomThemeSettingDialog | views/customthemesettingdialog.cpp | 自定义主题设置对话框 |
| 5 | TabRenameDlg | views/tabrenamedlg.h | 标签页重命名对话框 |

**注意**：完整缺口清单见 `tests/at/element_gaps.yaml`（35 个缺失类）。这些类需补充 `setAccessibleName()` 调用以支持更精准的 AT-SPI 元素定位。

---

## 4. 不可自动化用例说明

| # | 模块 | 用例数 | 不可自动化原因 | 建议替代策略 |
|---|------|--------|---------------|-------------|
| 1 | 触摸屏(#116111) | 16 | 需要触摸屏硬件输入 | 手动测试或使用触摸模拟设备 |
| 2 | 自定义主题(#116159) | 7 | 颜色/视觉效果验证 | 截图对比或人工验收 |
| 3 | 雷神窗口动画效果(#116157) | 1 | 动画效果视觉验证 | 截图对比或人工验收 |
| 4 | 翻译(#116175) | 1 | 多语言翻译人工校验 | 人工校对或使用 i18n 检查工具 |

---

## 5. 交付产物

### 随代码提交（PR）

| 文件 | 说明 |
|------|------|
| `tests/at/yaml/elements.yaml` | AT-SPI 元素定位表 (13 元素) |
| `tests/at/yaml/**/*.suite.yaml` | 23 个模块测试套件 (180 可执行用例) |
| `tests/at/cases_mapped.yaml` | AI 语义映射中间产物 |
| `tests/at/cases_raw.yaml` | xlsx 解析原始用例 |
| `tests/at/at-tree.yaml` | 静态扫描生成的 AT-SPI 树 |
| `tests/at/at-tree-annotated.yaml` | 带注释的树结构 |
| `tests/at/scanned_ok.yaml` | 源码扫描结果（已命名） |
| `tests/at/scanned_gaps.yaml` | 源码扫描结果（缺口） |
| `tests/at/element_gaps.yaml` | 元素缺口报告 |
| `tests/testcases.xlsx` | 原始用例文档 |

### 报告附件（本评论附件）

- `tests/at/report.md` — 本完整覆盖率报告

---

## 6. 注意事项

1. **运行时验证待执行**：所有用例基于静态源码扫描生成，未在桌面环境中执行验证。需在有 DISPLAY 的桌面环境安装 deepin-terminal 后运行 `youqu at run --testdir tests/at/yaml`。

2. **选择性映射**：无桌面环境的限制下，使用 AI 语义推理而非运行时 AT-SPI dump 进行步骤映射，操作选择（如 dtk_main_menu、dtk_context_menu、keyboard_hot_key）基于模块语义推断，准确性需运行时验证。

3. **setAccessibleName 补全建议**：35 个 UI 类缺少 AT-SPI 名称，建议优先补全 Top-5 缺口（见 §3），参考 `tests/at/spi/expected_names.yaml` 中的命名风格。

4. **触摸屏用例**：16 个触摸屏用例标记为 UNSUPPORTED —— 无法通过 AT-SPI 自动化，需人工测试。