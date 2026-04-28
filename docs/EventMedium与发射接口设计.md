# EventMedium 与发射接口设计

## 目标

当前实现已经在这套接口上落地 V1a 固定参数 density response：

- 默认 `density-evolution = affine-gaussian` 会执行 `s_0 -> s_f` 的仿射膨胀和平滑。
- `density-evolution = none` 保留原先 participant hotspot 抽 multiplicity、横向 Gaussian smear、再查询流体速度并 boost 的对照路径。
- ROOT 默认契约新增 freeze-out 几何诊断 `eps2_f`、`psi2_f`、`chi2`。
- 后续 density-field emission 或 density evolution backend 应接入 `buildEventMedium()` 或 `sampleEmissionSites()`，而不是改写 `BlastWaveGenerator::generateEvent()` 的主循环。

## 事件介质

`EventMedium` 是每个事件共享的 ROOT-free 介质状态：

- `participantPoints`
  初态 participant 横向点云，当前权重为 `1`。
- `participantGeometry`
  由 participant 点云直接恢复的协方差椭圆；`events.eps2` 和 `events.psi2` 始终来自这里。
- `initialDensity`
  由 participant 点云构造的初始 Gaussian point-cloud density。
- `emissionDensity`
  粒子发射与 density-normal flow 查询使用的发射阶段 density。
- `emissionGeometry`
  发射阶段 density 对应的几何代理；默认 V1a 下由 `s_f` 的二阶矩给出。

当前支持两种 `DensityEvolutionMode`：

```text
AffineGaussianResponse  # 默认 V1a 固定参数响应
None                    # 原先 identity 对照模式
```

`AffineGaussianResponse` 在 participant-plane 中对短轴方向用 `lambda_in = 1.20`、长轴方向用 `lambda_out = 1.05`，并加入 `sigma_evo = 0.5 fm` 的 Gaussian evolution smoothing。它只改变 `emissionDensity` 和 `emissionGeometry`，不会 silently 改变 `participantGeometry` 的事件摘要语义。

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
- `EmissionSite::sourceAnchor`
  源锚点，当前是未 smear 的 participant 坐标，继续写入 `particles.source_x/source_y`。
- `EmissionParameters`
  当前包含 `ParticipantHotspot` 和 `DensityField` backend、`smearSigma`、`nbdMu`、`nbdK`。

默认 V1a 使用 `DensityField` backend，从 `emissionDensity` 抽样发射点，同时保留原始 participant 作为 `sourceAnchor`。`density-evolution = none` 使用 `ParticipantHotspot` backend，以保持旧路径可对照。

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

只要 ROOT schema 没有明确版本化，`ParticleRecord` 的默认分支含义不应改变；事件摘要新增量应继续集中在 `EventInfo` 和 ROOT schema 层同步。
