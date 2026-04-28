# EventMedium 与发射接口设计

## 目标

本次重构只建立后续扩展需要的内部语言，不改变当前物理输出：

- 当前行为仍是 participant hotspot 抽 multiplicity、横向 Gaussian smear、再查询流体速度并 boost。
- 不新增 CLI/config key。
- 不改变 ROOT 输出树、分支或默认直方图契约。
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
  发射阶段 density 对应的几何代理，当前与 `participantGeometry` 相同。

当前唯一 `DensityEvolutionMode` 是 `None`，所以：

```text
initialDensity == emissionDensity
participantGeometry == emissionGeometry
```

未来若实现密度场膨胀，只应改变 `emissionDensity` 和必要的 `emissionGeometry`；不要 silently 改变 `participantGeometry` 的事件摘要语义。

## 密度场接口

`DensityFieldModel` 负责 Gaussian point-cloud density：

- `WeightedTransversePoint`
- `DensityField`
- `DensityFieldSample`
- `buildGaussianPointCloudDensityField()`
- `evaluateDensityField()`

`evaluateDensityField()` 返回解析 density 和 gradient。`density-normal` flow、可选 ROOT density snapshot、未来 density-field emission 都应共享这个接口，避免各处重复重建密度。

## 发射接口

`EmissionSampler` 把横向发射点抽样从 generator 主循环中拆出：

- `EmissionSite::position`
  实际横向发射坐标，写入 `particles.x/y`。
- `EmissionSite::sourceAnchor`
  源锚点，当前是未 smear 的 participant 坐标，继续写入 `particles.source_x/source_y`。
- `EmissionParameters`
  当前包含 `ParticipantHotspot` backend、`smearSigma`、`nbdMu`、`nbdK`。

未来若加入从膨胀后 density field 抽样发射点，应新增 `EmissionSamplerMode` backend，并保持 `EmissionSite` 作为 generator 主循环的输入。

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

这使得后续两个并行方向可以独立演进：

- density evolution：替换 `buildEventMedium()` 里的 `emissionDensity/emissionGeometry`
- emission sampling：新增 `sampleEmissionSites()` backend

只要 ROOT schema 没有明确版本化，`ParticleRecord` 和 `RootEventFileWriter` 不应改变默认分支含义。
