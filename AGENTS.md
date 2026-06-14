# C++ 代码规范（AI 规则版）

## 0. 读取规则

- 本文件用于约束 AI 生成、修改、审查 C/C++ 代码。
- `MUST` 表示必须遵守。
- `MUST_NOT` 表示禁止。
- `SHOULD` 表示默认遵守，除非有明确上下文理由。
- `MAY` 表示允许。
- 规则冲突时，优先遵守更具体、更严格的规则。
- 生成代码时优先匹配现有工程风格；现有风格缺失时使用本文规则。

## 1. 基础环境

- MUST: 文件编码使用 `UTF-8 (No BOM)`。
- SHOULD: VS 编译选项增加 `/source-charset:utf-8`。
- SHOULD: Windows 开发环境设置 Git 换行转换。

```bash
git config --global core.autocrlf true
```

## 2. 代码格式

- MUST: 缩进使用 4 个空格。
- MUST_NOT: 使用 Tab 缩进。
- MUST: `{` 与前序语句同行。
- MAY: 没有前序语句时，`{` 可独占一行。
- MUST: 双目运算符两侧加空格。
- MUST: 单目运算符不额外加空格。
- MUST: `if`、`while`、`for` 后的 `(` 前加空格。
- MUST: `{` 前与前序代码用空格分隔。
- MUST: 指针和引用写作 `Type *ptr`、`Type &ref`。
- MUST: `*` 和 `&` 靠近变量类型，不靠近变量名。
- MUST: 变量定义时初始化。
- MUST: 一行只定义一个变量。
- MUST_NOT: 除注释外，在代码中写中文字符。
- MUST_NOT: 在 log 文本中写中文字符。
- MUST: `namespace` 内部不整体额外缩进。
- SHOULD: 旧代码在 VS 中用 `Ctrl + K`、`Ctrl + F` 格式化。

## 3. 可读性

- MUST: 代码优先简洁、可朗读、可 review。
- MUST_NOT: 编写无必要的晦涩逻辑。
- MAY: 在极限性能、硬件相关、明确技术验证场景中使用复杂实现。
- MUST_NOT: 在函数调用参数中直接写复杂表达式。
- MUST_NOT: 在宏调用参数中直接写复杂表达式。
- SHOULD: 先计算复杂表达式并命名，再传参。
- MAY: 使用 lambda 作为简单参数。
- SHOULD: lambda 内部逻辑不超过 2 个行为。
- SHOULD_NOT: 使用宏。
- SHOULD: 优先使用 `const`、`constexpr`、`inline`、函数、模板替代宏。
- MUST: 必须写宏时，用 `do { ... } while (false)` 包裹语句块。
- MUST: 宏参数使用 `()` 保护。

## 4. 函数接口

- MUST: 不修改基础类型参数时，使用值传递。
- MUST: 不修改非基础类型参数时，使用 `const T &`。
- MUST: 参数需要被函数修改时，使用指针。
- MUST: 参数需要 move 时，使用右值引用。
- MUST: 函数绝对成功时，返回 `void`。
- MUST: 函数可能失败时，返回 `bool`。
- MUST: 需要表达失败原因时，返回明确错误枚举，或传入错误码指针。
- MUST_NOT: 默认使用 `int` 作为错误码。
- MAY: 确实需要整型错误码时，显式定义 `xxx_error_code_t`。
- SHOULD: 函数逻辑保持单进单出。
- SHOULD: 函数逻辑线性展开。
- MUST: 成功/失败返回值命名为 `result`。
- MUST: 临时 `bool` 返回值命名为 `ret_code`。
- MUST: 头部声明的临时变量初始化。
- MUST: 资源声明时初始化。
- MUST: 资源在统一出口释放。
- MUST: `public` 接口检查参数。
- MUST: 所有指针使用前判空。

## 5. 错误、异常、断言

- SHOULD: 错误处理倾向 `Fail Early, Fail Fast`。
- SHOULD: 尽快暴露错误第一现场。
- MUST_NOT: 使用异常实现普通业务流程。
- MAY: 为兼容第三方库或历史代码使用异常。
- MUST_NOT: 频繁抛异常。
- MUST: `assert` 只检查“代码写错才会发生”的条件。
- MUST_NOT: `assert` 参与业务逻辑。
- MUST_NOT: release 版依赖断言。
- SHOULD: 自测优先使用 debug 版。

## 6. 初始化和生命周期

- SHOULD: 类和结构体的非静态成员变量就地初始化。
- SHOULD: 优先使用初始化列表。
- MUST: 不就地初始化时说明收益。
- MUST_NOT: 使用类的静态存储周期变量。
- MAY: 使用 `constexpr` 静态变量。
- MUST: 静态生存周期对象使用 POD 类型。
- MUST: 非临时对象有明确且唯一的拥有者。
- MUST: 拥有者负责对象构造和析构。
- MUST: 动态分配对象的函数保证单一出口。
- MUST: 生命周期超出创建者作用域的对象使用引用计数或 `std::shared_ptr`。
- SHOULD: 运行期严格控制动态分配。
- SHOULD: 可启动时预分配的内存应启动时预分配并复用。
- MUST: lib 使用动态内存时，提供显式初始化接口和清理接口。

## 7. 文件、工程、头文件

- MUST: 目录名全小写。
- MUST: 文件名全小写。
- MUST: 文件名单词使用 `_` 分隔。
- MUST: C++ 源文件使用 `.cpp`。
- MUST: C 源文件使用 `.c`。
- MUST: 普通头文件使用 `.h`。
- MUST: 纯模板或仅头文件实现使用 `.hpp`。
- SHOULD: 单个 lib/dll 对外提供同名 `.hpp` 聚合对外头文件。
- MUST: `gameapp` 代码加 namespace。
- MUST: 其他 app 使用各自 namespace。
- MUST: `#include` 放在 header/source 文件开头。
- MUST: `#include` 例外位置需要说明原因。
- MUST: 引擎头文件使用 `#include "..."`。
- SHOULD_NOT: 头文件中 `#include` 其他头文件。
- SHOULD: 优先使用 `#pragma once`。
- MUST: 纯 C 代码使用 include guard。
- MUST: 不使用 `#pragma once` 时使用 include guard。
- MUST: include guard 宏全大写。
- MUST: include guard 宏单词使用 `_` 分隔。

```cpp
#ifndef _HEADER_FILE_NAME_H_
#define _HEADER_FILE_NAME_H_

// ...

#endif
```

目录结构：

- `src/yuengine/<module>/include/yuengine/<module>/`: 模块对外头文件。
- `src/yuengine/<module>/src/`: 模块 `.cpp` 实现文件。
- `tests/<module>/`: 模块测试文件。
- `docs/`: 设计参考文档，不自动成为运行规则。
- `.codex/`: 本地运行目录，禁止提交。

## 8. 继承

- MUST_NOT: 实现类多继承。
- MUST_NOT: 引入不必要的菱形继承。
- MAY: 确实需要共享一份基类数据时使用 `virtual` 继承。
- MAY: 同时继承接口类和实现类。
- MUST: 接口类不包含数据成员。
- MUST: 接口类命名以 `I` 开头。

## 9. 命名

- MUST: namespace 使用全小写和 `_` 分隔。
- MUST: class 使用 PascalCase。
- MUST: function 使用 lowerCamelCase。
- MUST: variable 使用全小写和 `_` 分隔。
- MUST: member variable 使用普通变量规则，并以 `_` 结尾。
- MUST: global variable 使用 `g_` 前缀。
- MUST: static variable 使用 `s_` 前缀。
- MUST: `const` 和 `constexpr` 变量使用全大写和 `_` 分隔。
- MUST: `typedef` 产生的自定义类型使用全小写、`_` 分隔、`_t` 后缀。
- MUST: `struct` 产生的自定义类型使用全小写、`_` 分隔、`_t` 后缀。
- MUST: enum 和常量使用全大写和 `_` 分隔。
- MAY: `extern "C"` 导出接口重名时加 `bd_` 前缀。
- MUST: `std::shared_ptr` 类型别名保留 `shared` 语义。
- MUST: `std::unique_ptr` 类型别名保留 `unique` 语义。
- MUST_NOT: 命名使用拼音。
- MUST_NOT: 命名使用超出 CET-6 范围的生僻英文词。
- SHOULD: 变量使用准确名词。
- SHOULD: 函数使用动宾结构。
- MUST: 函数行为符合函数名含义。
- MAY: Review 人员提出命名异议并要求重命名。

命名示例：

- namespace: `code_format`
- class: `YuIniFile`
- function: `modifyLife`
- variable: `current_life`
- member variable: `current_life_`
- global variable: `g_config`
- static variable: `s_count`
- const variable: `MAX_COUNT`
- custom type: `player_data_t`
- enum value: `ERROR_TIMEOUT`

## 10. 日志

- MUST: log 按文件级模块区分。
- MUST: log 支持按模块过滤和动态开关。
- MAY: 保留 `BD_TRACE_LOG`。
- MUST: 发布版本过滤 `BD_TRACE_LOG`。
- MUST: 头文件中重定向 log 模块时，文件末尾取消重定向。
- MUST: 日志参数类型和格式严格匹配。

日志格式：

- `size_t`: `%zu`
- `int64_t`: `"%" PRId64`
- `uint64_t`: `"%" PRIu64`
- `uint32_t`: `%u`
- `int32_t`: `%d`

日志脚本接口：

```lua
bd.debug.openLogObject(name)
bd.debug.closeLogObject(name)
bd.debug.openTraceLogObject(name)
bd.debug.closeTraceLogObject(name)
bd.debug.setLogObjectLevel(name, level)
```

## 11. 提交和注释

- MUST: 提交信息格式为 `[#单号][修改类型]修改描述`。
- MUST: 修改类型使用规定枚举。
- MAY: `[CICD]` 提交不加单号。

提交类型：

- `[Added]`: 新增功能。
- `[Fixed]`: 修复 bug。
- `[Changed]`: 已有功能逻辑变化。
- `[Refactored]`: 结构或性能优化，无功能变化。
- `[Removed]`: 删除功能。
- `[other]`: 编译修复、版本号等非业务改动。
- `[CICD]`: 流水线改动。

注释规则：

- SHOULD: 代码不清晰时优先重构。
- MUST: 确有歧义时写准确注释。
- MUST: 无用代码及时删除。
- MUST_NOT: 无用代码以注释形式入库。
- MUST: 独立模块文件头注释包含模块名。
- MUST: 独立模块文件头注释包含文件路径。
- MUST: 引用 paper 时注明来源。
- MUST: 引用第三方代码时注明来源。
- MUST: 对外导出接口维护 doxygen 注释。
- MUST: 脚本导出接口说明类名、接口名、用途、返回值、参数。

## 12. 技术约束

- MUST: 遵循 KISS 原则：`Keep it simple & stupid`。
- MUST_NOT: 炫技。
- MUST_NOT: 使用不必要的复杂设计。
- SHOULD_NOT: 使用多重继承。
- SHOULD_NOT: 使用模板深度嵌套。
- MUST_NOT: 固定项目中擅自引入新技术。
- MUST: 新技术先进入技术池，再进入主干。
- MUST_NOT: 使用 `using namespace std;`。
- MUST_NOT: 使用 `iostream`。
- MUST_NOT: STL 容器拷贝传参。
- SHOULD: 已知容器大小时先 `reserve`。
- MUST: iterator 自增使用 `++iterator`。
- MUST_NOT: lambda 使用 `[&]` 全捕获。
- MUST_NOT: lambda 使用 `[=]` 全捕获。
- MUST: 使用 `nullptr` 代替 `NULL`。
- MUST: 覆盖父类方法时加 `override`。
- SHOULD: 使用 lambda 代替 `std::bind`。
- SHOULD: 优先使用 `auto`。
- SHOULD: 优先使用 `using`。
- SHOULD: 优先使用 `emplace_back`。
- SHOULD: 优先使用移动语义。
- SHOULD: 出现 `new` 时考虑 `unique_ptr`。
- SHOULD: 优先使用 `make_shared`。
- SHOULD: 优先使用 `make_unique`。
- SHOULD: 不需要转成 `int` 的枚举使用 `enum class`。
- SHOULD: 能用 `constexpr` 时使用 `constexpr`。

新技术引入条件：

- MAY: 现有技术池无法解决问题时引入新技术。
- MAY: 新技术能提升 10% 以上开发效率时引入新技术。
- MUST: 引入新技术前准备书面说明。
- MUST: 引入新技术前完成风险分析。
- MUST: 引入新技术前准备培训方案。
- MUST: 新技术提交引擎负责人组织评审。

## 13. warning 和历史问题

- MUST: warning 按 error 处理。
- MUST: 使用 strict warning check。
- MUST: 正常但编译器误报的 warning 局部手动屏蔽。
- MUST_NOT: 为规避单个 warning 修改全局编译设置。
- MUST: 第三方库头文件集中引用。
- MUST: 第三方库 warning 在引用处屏蔽。
- MUST: VS warning 以最新 VS 版本为修复基准。

常见问题：

- MUST_NOT: 成员声明顺序与构造初始化顺序不一致。
- MUST_NOT: 关系运算两侧类型不一致。
- MUST_NOT: `&&` 和 `||` 混用时省略括号。
- MUST_NOT: 日志格式与参数类型不匹配。
- MUST_NOT: 日志格式与参数数量不匹配。
- MUST_NOT: 定义未使用变量。
- MUST_NOT: 对无符号数判断 `>= 0`。

逻辑表达式示例：

```cpp
if ((step_size_x > 0 && now_point.x >= end_point.x) ||
    (step_size_x < 0 && now_point.x <= end_point.x)) {
    // ...
}
```

## 14. lambda 优先于 std::bind

- SHOULD: 使用 lambda 代替 `std::bind`。
- REASON: lambda 每次调用时重新求值。
- REASON: `std::bind` 在创建绑定对象时求值。
- REASON: lambda 处理重载函数更自然。
- REASON: `std::bind` 处理重载函数通常需要 `static_cast` 指定签名。
- REASON: lambda 更容易 inline 优化。
- REASON: lambda 捕获引用更直接。
- REASON: `std::bind` 引用传递需要 `std::ref`。
