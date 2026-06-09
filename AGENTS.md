**硬性规范（违反即错）**：
- 项目是千万级商业项目，不要在项目中拉屎，代码价值要求高，始终保持高性能、高维护性、高拓展性、高稳定性的原则
- 只生成代码，不生成任何说明文档（README、GUIDE、TODO、MD等），除非用户明确要求
- 禁止`else`，只用 Early Return
- 只用 foreach，禁止 LINQ（Where/Select/Any/All/FirstOrDefault 等）
- 一文件一类型，禁止一个文件定义多个类/枚举/结构体
- 多类型分支只用注册表+字典 O(1) 查询，禁止多层 if/switch
- 调用类时只用顶部 using，禁止在代码中加命名空间前缀
- UI 组件声明后直接用，禁止判空
- 字符串: 日志log的直接写，其他必须 const/static 声明的常量，禁止硬编码
- 只实现明确要求的功能，禁止私自扩展

**代码风格**：
- 函数写简要注释，空方法体留空不写注释
- `if` / `foreach` / `for` / `while` 语句块必须用 `{}`，且 `}` 后空一行
- 新文件创建后提醒用户在 Unity 中手动编译，Unity 会自动生成 `.meta`

---

**命名规范**：
```
class ClassName : IInterfaceName
enum EnumName { EnumValue }
public Type PublicField;
public Type PropertyName { get; set; }
private Type _privateField;
[SerializeField] private Type _serializedField;
void MethodName() { var localName; }
const Type CONST_NAME;
static readonly Type STATIC_NAME;
```