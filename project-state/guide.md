# 项目概览

## 项目目的

`Blast_wave` 是一个基于 C++17 和 ROOT 的小型 blast-wave 事例生成器。
当前目标是保留 Monte Carlo Glauber 的逐事例 participant 几何涨落，在一个体量小、职责清晰、便于继续扩展的代码骨架上生成固定 `b` 的 Pb-Pb 风格玩具样本，并输出可被独立 QA 读取校验的 ROOT 结果。

## 当前默认运行链

- 初态几何：逐事例 participant point cloud
- 默认介质演化：`density-evolution = affine-gaussian`
- 默认横向流方向：`flow-velocity-sampler = covariance-ellipse`
- 默认热动量：`thermal-sampler = maxwell-juttner`
- 默认末态汇总：`events.v2`、`events.centrality`、freeze-out 几何诊断、`r2_*`
- 可选 V2 扩展：
  - `density-evolution = gradient-response`
  - `flow-velocity-sampler = gradient-response`
- 可选差分分析：
  - 配置 `v2pt-bins`
  - 在主文件或独立文件中写 `v2{2}(pT)` 结果

## 代码职责分层

- `include/` + `src/`
  - ROOT-free 核心物理、共享数学和输出 schema 辅助逻辑
- `apps/generate_blastwave_events.cpp` + `apps/generate_blastwave/`
  - 生成入口、CLI/config 解析、ROOT 写出
- `apps/qa_blastwave_output.cpp`
  - 独立 ROOT 读取和输出校验
- `apps/analyze_blastwave_v2pt.cpp`
  - 独立差分 `v2{2}(pT)` 后处理
- `tests/`
  - ROOT-free 回归测试
- `config/`
  - 受版本控制的示例配置
- `docs/`
  - 人写说明、设计文档和 hand note
- `project-state/`
  - 当前协作台账，不替代代码和高权威说明文档

## 当前对外运行契约

- 生成入口支持：
  - `generate_blastwave_events [options]`
  - `generate_blastwave_events --config <path> [options]`
  - `generate_blastwave_events <config-path> [options]`
- 解析优先级：
  - CLI 显式值 > 配置文件值 > 内建默认值
- 相对路径规则：
  - `output` 和 `v2pt-output` 都相对配置文件目录解析
- 当前规范示例配置：
  - `config/test_b8.cfg`
- 当前强制 ROOT 主载荷：
  - `events`
  - `participants`
  - `particles`
  - 事件级 `centrality`、`v2`、`eps2_f/psi2_f/chi2`、`r2_0/r2_f/r2_ratio`
  - 粒子级 `x0/y0/emission_weight`
- 当前可选载荷：
  - flow ellipse debug 对象
  - `density-normal` 事件密度快照
  - V2 `gradient-response` debug 直方图
  - 差分 `v2{2}(pT)` 元数据和结果对象

完整参数说明与对象清单以 `docs/项目说明.md` 和代码 schema 为准，不在本文件重复展开。

## 文档角色约定

- `docs/项目说明.md`
  - 详细运行说明和当前物理/接口解释
- `docs/手记文档.md`
  - 简明主链和易混淆概念
- `docs/agent_guide.md`
  - 面向 agent 的改动约束和文档同步规则
- `project-state/current-status.md`
  - 当前基线和注意事项
- `project-state/tests.md`
  - 精简后的耐久验证证据
- `project-state/handoff.md`
  - 最新可执行 handoff

## 当前协作规则

- 若修改算法语义、配置契约、ROOT schema、QA 逻辑或用户运行方式，必须同步相应 `project-state/` 文件。
- `project-state/` 的规范路径是 `project-state/`，不是旧写法 `.project-state/`。
- `project-state/tests.md` 只保留验证结论和最小必要证据，不再堆完整命令转录。
- `project-state/current-status.md` 只保留当前视图，不承担完整历史说明。
- 历史演化过程放在 `project-state/changelog.md` 和 `project-state/decisions.md`，不要回灌到当前视图文件。

## 当前操作提醒

- `v2pt-output-mode = separate-file` 时，主结果文件只有 `v2_2_pt_edges` 也是合法状态。
- `qa/` 目录中存在旧 schema ROOT 文件；复用前先确认是否匹配当前契约。
- 在这台机器上，沙箱内 `alienv` ROOT smoke 若出现 PCM / module 噪声，不应当作权威结果；权威验证依旧来自沙箱外 O2Physics 路径。
