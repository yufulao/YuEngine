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
- 100% 模仿已有代码风格
- 函数写简要注释，空方法体留空不写注释
- `if` / `foreach` / `for` / `while` 语句块必须用 `{}`，且 `}` 后空一行
- 新文件创建后提醒用户在 Unity 中手动编译，Unity 会自动生成 `.meta`

---

## 框架速查

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

**方法顺序**：构造函数 → 静态工厂 → interface → abstract → override → public → protected → private

**namespace**：框架层用 `Yu`，业务层不用，禁止调用处加前缀

**单例基类**：`MonoSingleton<T>`（MonoBehaviour）、`BaseSingleTon<T>`（纯 C#）

**Manager 生命周期**：实现 `IMonoManager`，顺序 `OnInit → Update → OnClear`，由 `GameManager` 统一管理

**事件系统**：`EventManager`，AddListener/Dispatch/RemoveListener 泛型参数必须一致；MonoBehaviour 在 OnEnable/OnDisable 订阅，Manager 在 OnInit/OnClear 订阅

**UI**：继承 `UICtrlBase`，MVC 三层分离（Ctrl/Model/View），通过 `UIManager` 打开/关闭面板

**配表**：Luban 自动生成，通过 `ConfigManager.Tables` 访问；现有表：CfgAction、CfgBGM、CfgCondition、CfgFont、CfgGlobal、CfgPlanet、CfgScene、CfgSFX、CfgUI；后处理常量：DefPanel、DefPlanet、DefScene

**存档**：`SaveManager`（EasySave3），Key 用 `DefGlobal` 常量

**对象池**：`PoolManager`，池对象继承 `PoolableGameObject`

**资源加载**：`AssetManager`（Addressable）

**状态机**：`FsmComponent<TOwner,TState>`，层级状态机 `HfsmComponent`，全局 Fsm 继承 `BaseFsm`/`BaseHfsm`

**行为队列**：`GameActionManager`，逻辑帧驱动，`GameActionRequest` 为请求基类

**GameFlow 状态**：`GameFlowNoneState → GameFlowLoadingDataState → GameFlowTitleState → GameFlowGameRunningState`

**框架层结构**：
```
Assets/Scripts/
├── Core/Manager/   ActionManager, CameraManager, ConditionManager, EventManager,
│                   FsmManager, GameActionManager, GameManager, InputManager,
│                   PoolManager, TimeScaleManager, UIManager,
│                   AssetManager, AudioMixerManager, BGMManager, ConfigManager,
│                   GameLog, SaveManager, SceneManager, SFXManager
├── Core/Misc/      Base(单例/接口), Extensions, FilterAndSorter, GMCommand, Utils
├── Config/Base/    CfgXxx / RowCfgXxx / Tables
├── GameLogic/      GameFlowManager, GameRunning, Localization, SteamManager
├── UI/             Component, Utils, Panel
├── Editor/
└── Misc/           Def(DefGlobal), Utils
```
