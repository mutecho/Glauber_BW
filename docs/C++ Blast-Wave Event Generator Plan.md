# C++ Blast-Wave Event Generator Plan

## Summary
- 新实现使用独立的 C++ 代码，不复制 `qiye` 目录中的参考实现，只借鉴其“Glauber 几何 + 逐粒子输出”的目标形态。
- 第一版目标是固定 impact parameter 的事件生成器，默认 `b = 8 fm`，保留事件级 Glauber participant 波动。
- 运行形态采用“普通 C++ 生成器核心 + ROOT 链接可执行程序”，ROOT 只负责 I/O 和基础 QA。
- 当前阶段只关注事件生成，不实现 femtoscopy 相关函数或 pair analysis。

## Implementation Changes
- 建立一个独立生成器核心，至少包含这些接口与数据类型：
  - `struct BlastWaveConfig`：保存几何、流场、温度、粒子种类、随机数种子、事件数、输出文件名等参数。
  - `struct EventInfo`：保存 `event_id`、`b`、`Npart`、`eps2`、`psi2`、`Nch`。
  - `struct ParticleRecord`：保存 `event_id`、`pid`、`charge`、`mass`、`x`、`y`、`z`、`t`、`px`、`py`、`pz`、`E`、`eta_s`、`source_x`、`source_y`。
  - `generateEvent(const BlastWaveConfig&)`：返回单事例的 `EventInfo` 和粒子列表。
- Glauber 几何模块采用 2D Monte Carlo Glauber：
  - 采样两个核的横向核子位置。
  - 按 `sigma_NN` 判定碰撞并提取 participants。
  - 计算 participant 重心、`eps2`、`psi2`。
  - 第一版固定 `b`，不做最小偏置抽样和中心度回切。
- 发射源模块采用 “participant hotspot + 简单 blast-wave”：
  - 每个 participant 作为一个发射热点，粒子发射点由 participant 位置加高斯 smearing 得到。
  - 纵向部分先保留固定 `tau0` 和高斯 `eta_s`。
  - 横向流场通过局域四速度 `u^\mu(x,y)` 描述，局域静止系抽样热分布，再做完整 Lorentz boost。
  - 严禁沿用参考代码中“直接给 `px, py` 加流速偏移”的写法。
- 输出层由 ROOT driver 完成：
  - `events` 树：`event_id, b, Npart, eps2, psi2, Nch`
  - `particles` 树：`event_id, pid, charge, mass, x, y, z, t, px, py, pz, E, eta_s, source_x, source_y`
  - 附带少量 QA 直方图：`Npart`、`eps2`、`x-y`、`pT`、`eta`、`phi`
- 参数管理使用 `BlastWaveConfig` 默认值加 CLI 覆盖：
  - 第一版至少支持覆盖 `nEvents`、`b`、`T`、`tau0`、`smearSigma`、`sigmaNN`、`seed`、`output`
  - 不引入外部 YAML/JSON 配置文件

## Public Interfaces
- 可执行程序提供一个默认入口，例如 `generate_blastwave_events`。
- CLI 最低要求：
  - `--nevents`
  - `--b`
  - `--temperature`
  - `--tau0`
  - `--smear`
  - `--seed`
  - `--output`
- ROOT 输出文件是第一版对下游分析的正式接口；后续 femto 分析只依赖该文件，不直接依赖生成器内部类。

## Test Plan
- 数值一致性：
  - 检查所有粒子满足 `E^2 - p^2 ≈ m^2`
  - 检查无 `NaN/Inf`
  - 检查流速不超光速
- 结构一致性：
  - `events.Nch` 与 `particles` 中对应事件的粒子数一致
  - 事件号连续，所有粒子都能映射回唯一事件
- 几何 smoke test：
  - 固定 `b = 8 fm` 时，participant 分布明显偏心，`eps2` 分布非零
  - 改小 `b` 时 `Npart` 上升、平均 `eps2` 下降
- 动力学 smoke test：
  - 关闭各向异性后，`phi` 分布应回到轴对称
  - 增大径向流参数后，`pT` 谱应整体变硬
- 输出可用性：
  - 用独立 QA 宏读取 ROOT 文件并画出 `x-y`、`pT`、`eta`、`phi`，确认分支命名和类型稳定

## Assumptions
- 第一版只生成单一直接 pion 物种，默认 `pi+` 或配置成单一 pion species；不做 resonance decay。
- 纵向动力学保持简单 Bjorken/Gaussian 近似，不引入 hadronic rescattering。
- 不复用 `qiye` 中的文件和函数，只吸收其物理意图与问题教训。
- 计划文档的目标落点为 `docs/blastwave_generator_plan.md`，后续若退出 Plan Mode，再按此内容写入仓库。
