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
- 可选 affine 闭合流场：
  - `density-evolution = affine-gaussian`
  - `flow-velocity-sampler = affine-effective`
  - 默认 `affine-effective-mode = additive-rho`
  - 可选 `affine-effective-mode = full-tensor`
- 可选 V2 扩展：
  - `density-evolution = gradient-response`
  - `flow-velocity-sampler = gradient-response`
- 可选三阶响应测试初态：
  - `initial-geometry = response-test-023`
  - 人工 `0+2+3` 点云，`participants.nucleus_id = -1`
  - `initial-geometry-fluctuate = true` 为宽分布 response test；默认关闭，固定 023 仍是 closure baseline
- 可选差分分析：
  - 配置 `v2pt-bins`
  - 配置 `v3pt-bins`
  - 在主文件或独立文件中写 `v2{2}(pT)` / `v3{2}(pT)` 结果

## 代码职责分层

- `include/` + `src/`
  - ROOT-free 核心物理、共享数学和输出 schema 辅助逻辑
- `apps/generate_blastwave_events.cpp` + `apps/generate_blastwave/`
  - 生成入口、CLI/config 解析、ROOT 写出
- `apps/qa_blastwave_output.cpp`
  - 独立 ROOT 读取和输出校验
- `apps/analyze_blastwave_vnpt.cpp`
  - 独立差分 `v2/v3{2}(pT)` 后处理
- `tests/`
  - ROOT-free 回归测试
- `config/`
  - 受版本控制的示例配置
- `notebooks/`
  - 面向 ROOT 输出文件的交互式分析入口；当前包含事件级 `v_n`-`epsilon_n` 线性回归 notebook
- `docs/`
  - 当前说明、公式说明、简明手记和历史计划归档
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
  - `output` 和 `flowpt-output` 都相对配置文件目录解析
- 当前规范示例配置：
  - `config/test_b8.cfg`
  - `config/test_b8_affine_effective.cfg`
  - `config/test_023_dense.cfg`
  - `config/test_023_fluctuating.cfg`
  - `config/test_023_dense_eps2_only_fluct.cfg`
  - `config/test_023_dense_eps3_only_fluct.cfg`
  - `config/test_b8_flowpt.cfg`
  - `config/test_b8_density_normal_flow_trans.cfg`
  - `config/test_b8_density_normal_flow_trans_gradient.cfg`
- 当前强制 ROOT 主载荷：
  - `events`
  - `participants`
  - `particles`
  - 事件级 `centrality`、`v2`、`eps2_f/psi2_f/chi2`、`r2_0/r2_f/r2_ratio`
  - 三阶响应摘要 `eps3/psi3`、`v3`、`v2_wrt_psi2`、`v3_wrt_psi3`
  - response-test 模板快照 `geo_a2/geo_a3/geo_r2x/geo_r2y/geo_r3/geo_sigma3`
  - response/cross-talk TH2 默认以紧凑显示窗口打开，但保留完整物理存储范围
  - 粒子级 `x0/y0/emission_weight`
- 当前可选载荷：
  - `debug-initial-geometry` 首个有效事件 density map：`initial_geometry_density_x-y`
  - flow ellipse debug 对象，以及 affine-effective 模式下附带的 mode 编码、闭合诊断和 additive-rho surface rho 分解
  - affine-effective 首个有效事件的演化前/后 density map：`affine_effective_density_initial_x-y`、`affine_effective_density_final_x-y`
  - `density-normal` 事件密度快照
  - V2 `gradient-response` debug 直方图
  - 差分 `v2{2}(pT)` / `v3{2}(pT)` 元数据和结果对象

完整参数说明与对象清单以 `docs/项目说明.md` 和代码 schema 为准，不在本文件重复展开。

## 文档角色约定

- `docs/README.md`
  - 当前文档索引，说明当前说明、协作台账和历史计划归档的分工
- `docs/项目说明.md`
  - 详细运行说明和当前物理/接口解释
- `docs/数学物理公式流程说明.md`
  - 公式导向的主链说明，串起 participant 几何、密度演化、发射抽样、热动量、流场 boost 和 flow 观测量
- `docs/手记文档.md`
  - 简明主链和易混淆概念
- `docs/PLAN/`
  - 历史设计、旧 handoff 和已执行计划归档，默认不作为当前契约来源
- `AGENTS.md`
  - 当前仓库级 agent 工作规则
- `project-state/doc-sync-map.yml`
  - 当前 `sync-project-knowledge` 文档同步路由
- `project-state/current-status.md`
  - 当前基线和注意事项
- `project-state/tests.md`
  - 精简后的耐久验证证据
- `project-state/handoff.md`
  - 最新可执行 handoff

## 当前协作规则

- 若修改算法语义、配置契约、ROOT schema、QA 逻辑或用户运行方式，必须同步相应 `project-state/` 文件。
- 文档同步优先按 `project-state/doc-sync-map.yml` 扩展 required doc checks；`docs/PLAN/` 下的历史计划默认列为 `skip_historical`，除非用户明确要求修改历史文档。
- 当前 affine-effective 语义以 DEC-013 与 DEC-016 为准：`additive-rho` 保留 `flow-trans-rho0` baseline，`full-tensor` 为 opt-in，`affine-kappa-aniso` 是 legacy/no-op。
- 当前横向流强公开名以 DEC-016 为准：`flow-trans-rho0` / `flow-trans-profile-power` 替代旧 `rho0` / `flow-power`，旧名直接报错。
- 当前 density-defined flow-trans 半径 profile 分辨率以 DEC-017 为准：`balanced` 是默认，`precise` 是旧 `360 x 512` 网格，`fast` 是低成本预扫。
- 当前 density-normal 壳层梯度流强修正以 DEC-018 为准：`flow-trans-magnitude-mode = radius-profile` 是默认；`shell-gradient-corrected` 只允许配 `density-normal + density-percentile/level`，显式 `flow-trans-gradient-*` 参数不能在默认模式下静默生效。
- 当前 density-defined flow-trans 半径语义以 DEC-020 为准：`density-percentile` / `density-level` 只定义角向几何边界 `R_density(phi)`；主流强使用 `xi_flow = q * xi_shell`，其中 `xi_shell = r / R_density(phi)`，`flow-trans-rho0` 是 covariance-equivalent `xi_flow = 1` 处的参考快度尺度。
- `project-state/` 的规范路径是 `project-state/`，不是旧写法 `.project-state/`。
- `project-state/tests.md` 只保留验证结论和最小必要证据，不再堆完整命令转录。
- `project-state/current-status.md` 只保留当前视图，不承担完整历史说明。
- 历史演化过程放在 `project-state/changelog.md` 和 `project-state/decisions.md`，不要回灌到当前视图文件。
- `shell_weight` 与 `EmissionSite::emissionWeight` 结构调整仍未进入当前 affine-effective rollout。

## 当前操作提醒

- `flowpt-output-mode = separate-file` 时，主结果文件只有启用 harmonic 的 `*_2_pt_edges` 也是合法状态。
- `flow-trans-direction-gradient-fraction`、`flow-trans-radius` 与显式 `flow-trans-radius-resolution` 只允许 `flow-velocity-sampler = density-normal`，不能在其他 sampler 下静默配置。
- `flow-trans-direction-gradient-fraction = 0` 是纯几何膨胀方向，`1` 是纯 density-gradient 方向，`0<f<1` 用 `1-f` 作为全局外向投影下限；它只约束反向或过横向的热点梯度，不手动指定各向异性系数或形状。
- `flow-trans-magnitude-mode = shell-gradient-corrected` 必须使用 density-defined radius；`flow-trans-gradient-max-factor-delta` 限制乘法因子偏离量，不是快度加法上限；`xi_shell > 1` 时只复用 correction table 的最外层壳，基础 `rhoRaw` 使用 sigma-equivalent `xi_flow`。
- `response-test-023` 是 opt-in response/closure test；默认 `initial-geometry = glauber` 不变。
- `initial-geometry-a2/a3` 是模板权重，不是实际 `eps2/eps3`。
- `initial-geometry-fluctuate` 只允许用于 `response-test-023`；固定 023 用于 closure baseline，fluctuating 023 用于宽 `events.eps2/eps3` 分布测试。
- 当前 `response-test-023` 模板按 `1:A2:A3` 在同一总源数池内竞争分配源点；`A2/A3` 抽样独立不保证测得的 `events.eps2/eps3` 独立，交叉性控制样本应优先用 `config/test_023_dense_eps2_only_fluct.cfg` 和 `config/test_023_dense_eps3_only_fluct.cfg`。
- `qa/` 目录中存在旧 schema ROOT 文件；复用前先确认是否匹配当前契约。
- 在这台机器上，沙箱内 `alienv` ROOT smoke 若出现 PCM / module 噪声，不应当作权威结果；权威验证依旧来自沙箱外 O2Physics 路径。
