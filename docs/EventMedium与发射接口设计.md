# EventMedium 与发射接口设计

## 目标

当前实现已经在这套接口上落地 V1a 固定参数 density response：

- 默认 `density-evolution = affine-gaussian` 会执行 `s_0 -> s_f` 的仿射膨胀和平滑。
- opt-in `flow-velocity-sampler = affine-effective` 会把 `s_0 -> s_f` 恢复出的初末态半轴闭合量转成有效流场。
- `density-evolution = none` 保留原先 participant hotspot 抽 multiplicity、横向 Gaussian smear、再查询流体速度并 boost 的对照路径。
- opt-in `density-evolution = gradient-response` 与 `flow-velocity-sampler = gradient-response` 会共同启用 V2 梯度响应：从发射 marker 密度抽 `r0`，再用动力学密度梯度给出 `rf` 和横向 `beta_T`。
- ROOT 默认契约新增 freeze-out 几何诊断 `eps2_f`、`psi2_f`、`chi2`。
- ROOT 默认契约还包含 `r2_0`、`r2_f`、`r2_ratio` 以及粒子级 `x0/y0/emission_weight`，使 V2 的 marker 初态、末态发射点和可选权重可被 QA 与下游分析复算。
- 后续 density-field emission 或 density evolution backend 应接入 `buildEventMedium()` 或 `sampleEmissionSites()`，而不是改写 `BlastWaveGenerator::generateEvent()` 的主循环。

## 事件介质

`EventMedium` 是每个事件共享的 ROOT-free 介质状态：

- `participantPoints`
  初态 participant 横向点云，当前权重为 `1`。
- `participantGeometry`
  由 participant 点云直接恢复的协方差椭圆；`events.eps2` 和 `events.psi2` 始终来自这里。
- `initialDensity`
  由 participant 点云构造的初始 Gaussian point-cloud density。
- `markerDensity`
  V2 发射 marker 使用的 `s_em`；有效宽度为 `sqrt(flowDensitySigma^2 + gradientSigmaEm^2)`。
- `dynamicsDensity`
  V2 梯度响应使用的 `s_dyn`；有效宽度为 `sqrt(flowDensitySigma^2 + gradientSigmaDyn^2)`，并要求 `gradientSigmaDyn > gradientSigmaEm`。
- `emissionDensity`
  粒子发射与 density-normal flow 查询使用的发射阶段 density。
- `emissionGeometry`
  发射阶段 density 对应的几何代理；默认 V1a 下由 `s_f` 的二阶矩给出。
- `affineEffectiveClosure`
  仅在 `AffineGaussianResponse` 下恢复的几何闭合诊断：`sigma_in/out_0`、`sigma_in/out_f`、`G_in/out`、`Lambda_in/out`、`Lambda_bar` 和 `deltaLambda`。

当前支持两种 `DensityEvolutionMode`：

```text
AffineGaussianResponse  # 默认 V1a 固定参数响应
None                    # 原先 identity 对照模式
GradientResponse        # opt-in V2 梯度响应
```

`AffineGaussianResponse` 默认在 participant-plane 中对短轴方向用 `lambda_in = 1.20`、长轴方向用 `lambda_out = 1.05`，并加入 `sigma_evo = 0.5 fm` 的 Gaussian evolution smoothing。三个默认值现在都可通过 config/CLI 调整。它只改变 `emissionDensity` 和 `emissionGeometry`，不会 silently 改变 `participantGeometry` 的事件摘要语义。

若同时选择 `flow-velocity-sampler = affine-effective`，`buildEventMedium()` 会在同一处直接恢复 affine 闭合量：

- `sigma_in_0 = participantGeometry.radiusMinor`
- `sigma_out_0 = participantGeometry.radiusMajor`
- `sigma_in_f = emissionGeometry.radiusMinor`
- `sigma_out_f = emissionGeometry.radiusMajor`
- `Psi2 = participantGeometry.psi2`
- `G_in/out = sigma_f / sigma_0`
- `Lambda_in/out = log(G_in/out)`
- `Lambda_bar = 0.5 * (Lambda_in + Lambda_out)`
- `deltaLambda = 0.5 * (Lambda_in - Lambda_out)`

有效性判据固定为：初末态四个半轴都必须有限且严格大于 `0`。否则 affine-effective 流 sampler 直接返回零横向流，不回退到其他模式。

`GradientResponse` 构造三套密度：`s0`、`s_em` 和 `s_dyn`。发射 sampler 从每个 participant 对应的 `s_em` Gaussian component 抽 marker 初态 `r0`，在 `r0` 查询 `-grad(s_dyn)/(s_dyn + floor)`，再生成位移 `dr` 与横向速度 `beta_T`。本模式必须和 `flow-velocity-sampler = gradient-response` 成对使用，避免 medium response 与 velocity source 混搭。

## 密度场接口

`DensityFieldModel` 负责 Gaussian point-cloud density：

- `WeightedTransversePoint`
- `DensityField`
- `DensityFieldSample`
- `buildGaussianPointCloudDensityField()`
- `evaluateDensityField()`

`DensityField` 支持等方 Gaussian kernel 和共享全二维 covariance kernel。`evaluateDensityField()` 返回解析 density 和 gradient。`density-normal` flow、可选 ROOT density snapshot、V1a density-field emission 都共享这个接口，避免各处重复重建密度。

## 发射接口

`EmissionSampler` 把横向发射点抽样从 generator 主循环中拆出：

- `EmissionSite::position`
  实际横向发射坐标，写入 `particles.x/y`。
- `EmissionSite::initialPosition`
  发射 marker 初态坐标，写入 `particles.x0/y0`；非 V2 模式下设为 participant anchor，保证 schema 语义不空。
- `EmissionSite::sourceAnchor`
  源锚点，当前是未 smear 的 participant 坐标，继续写入 `particles.source_x/source_y`。
- `EmissionSite::betaTX/betaTY`
  V2 梯度响应保存的横向速度；V2 的 flow sampler 直接使用这组速度。
- `EmissionSite::emissionWeight`
  可选 Cooper-Frye 风格分析权重，目前支持 `none` 与 `mt-cosh`。
- `EmissionParameters`
  当前包含 `ParticipantHotspot`、`DensityField` 和 `GradientResponse` backend，以及 hotspot multiplicity、V2 梯度响应和可选权重参数。

默认 V1a 使用 `DensityField` backend，从 `emissionDensity` 抽样发射点，同时保留原始 participant 作为 `sourceAnchor`。`density-evolution = none` 使用 `ParticipantHotspot` backend，以保持旧路径可对照。

V2 使用 `GradientResponse` backend。每个候选粒子先得到 `initialPosition = r0`，再由局域动力学密度梯度得到 `position = rf`、`displacementX/Y`、`gradientMagnitude` 和 `betaTX/Y`。`sourceAnchor` 始终保持原 participant 坐标。

本轮 affine-effective 闭合只修改 `FlowFieldModel` 的速度生成，不调整 `EmissionSite::emissionWeight` 结构，也不引入新的 `shell_weight` 概念。

## 当前事件流程

当前 `generateEvent()` 的职责边界是：

```text
sampleParticipants()
  -> buildEventMedium()
  -> sampleEmissionSites()
  -> sample eta_s / thermal momentum / flow velocity
  -> Lorentz boost
  -> ParticleRecord / EventInfo
```

这使得两个方向可以继续独立演进：

- density evolution：替换或扩展 `buildEventMedium()` 里的 `emissionDensity/emissionGeometry`
- emission sampling：继续通过 `sampleEmissionSites()` backend 切换
- velocity sampling：继续通过 `FlowFieldModel` dispatch；V2 的 site overload 消费 `EmissionSite` 中保存的 `betaTX/Y`
- affine 闭合：继续通过 `EventMedium.affineEffectiveClosure -> FlowFieldModel` 连接，不把闭合诊断量塞回 `FlowEllipseInfo`

只要 ROOT schema 没有明确版本化，`ParticleRecord` 的默认分支含义不应改变；事件摘要新增量应继续集中在 `EventInfo` 和 ROOT schema 层同步。
