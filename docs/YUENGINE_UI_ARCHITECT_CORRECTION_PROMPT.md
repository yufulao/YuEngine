# UI Framework / Web Editor Correction Prompt

把下面这段原样发给团队架构师即可：

```text
请立即修正 UI 方向，按以下边界回退和继续：

1. YuEngine 现在只做通用 UI runtime framework 和 UIManager/BaseUI framework。
   不做背包、剧情、任务、商店、经营、时间、存档、角色等任何游戏工程强耦合 UI。
   这些名字不要再出现在引擎 UI 计划、验收、测试样例、迁移矩阵或任务里。

2. 当前已经落地的错误方向要清干净，不要保留兼容、不留 fallback：
   - 删除旧的业务/代表窗口样例代码和测试。
   - 删除旧的窗口迁移矩阵。
   - 删除 C++ app / ImGui / native shell 这条 UI Editor 作为引擎能力的路径。
   - CMake 里不能再有相关 target。
   - 文档里不能再把业务窗口迁移当作 UI Framework 完成度。

3. UI Core 只做底层 UI 节点和布局：
   node tree、Rect/Anchor/Pivot/Margin/Padding/SizePolicy、layout container、
   hit-test/input route foundation、draw IR、dirty category、Render bridge。
   UI Core 不负责窗口生命周期、打开关闭、缓存释放、popup/fullscreen stack。

4. UIManager/BaseUI framework 做通用窗口管理能力：
   BaseUI lifecycle、registry、layer model、loaded/active map、popup stack、
   fullscreen stack、open args、close/reopen/release/cache policy。
   这里的 panel/view 都只能是通用运行时记录，不是业务 panel 文件夹。

5. Component Library 做通用组件和机制：
   Text、Image、Button、Slider、Toggle、Progress、Scroll、List/GridView、
   atlas、batching、invalidation/rebuild、diagnostics。
   GridView/List 参考现有 FancyScrollView 派生 GridView 的虚拟化和复用思路，
   但不能带任何背包/商店/任务数据。

6. UI Editor 的方向就是 Web，不是 Web 优先。
   Web frontend + local editor service + engine preview/validator。
   Web 负责编辑体验、属性面板、拖拽、层级树、资源选择、校验展示。
   local service 负责读写文件、schema/version、validator、cook、preview IPC。
   YuEngine runtime 只负责真实预览和运行时解释。
   不再做 C++ app 编辑器，不再做 ImGui 编辑器 fallback。

7. 接下来只允许创建通用任务：
   - UI Core layout/node/input/draw
   - UI component library
   - UIManager/BaseUI runtime records
   - Web Editor schema/service/frontend/preview protocol
   禁止创建任何业务窗口迁移任务。

8. 提交前必须跑：
   git diff --check
   cmake --preset windows-fast-gate
   cmake --build --preset windows-fast-gate
   ctest --preset windows-fast-gate --output-on-failure

请先提交一版 cleanup，把错误落地清掉，再按新的 Web Editor 和通用 UI Framework 计划继续。
```
