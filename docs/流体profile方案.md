# 新增可切换的 `density-normal` 流体元速度模型

## Summary

- 在保留当前默认 `covariance-ellipse` 流场的前提下，新增一套基于 smeared participant density 的 `density-normal` 横向流模型，公开为可切换的 `flow-model`，避免直接替换当前稳定基线。
- 结构上不把新逻辑塞回生成器；保持生成器只做“构建事件级流场上下文 + 查询单点流速”，把几何恢复、density/gradient 计算、流场分派拆到 ROOT-free 核心层。
- 文档里的物理意图保留，但按代码现状修正两个关键点：`Σ^{-1}` fallback 必须使用相对事件质心的坐标 `delta = x - center`，且 `flow-density-sigma` 不与当前 `smearSigma` 直接绑死，避免把“发射点展宽”和“流场重建核宽度”耦合成一个参数。

## Key Changes

### 公共接口与配置

- 在 `BlastWaveConfig` 增加：
  - `flowModel`，取值 `covariance-ellipse | density-normal`，默认 `covariance-ellipse`
  - `flowDensitySigma`，默认 `0.5 fm`，独立于 `smearSigma`
- CLI/config 新增：
  - `--flow-model <covariance-ellipse|density-normal>`
  - `--flow-density-sigma <fm>`
  - 配置键 `flow-model`、`flow-density-sigma`
- 现有 `rho0`、`rho2`、`flow-power` 保留：
  - `covariance-ellipse` 继续使用 `rho0 + rho2 * eps2 * cos(2phiB)`
  - `density-normal` 首版只使用 `rho0 * rTilde^flowPower`，`rho2` 在该模型下解析但不参与计算，并在文档中明确为该模型暂未使用

### 核心结构

- 保持 `FlowFieldModel.h` 作为对外 umbrella 入口，避免上层 include 面扩散。
- 在核心实现里拆出三层职责：
  - 全局几何层：从 `WeightedTransversePoint` 计算质心、协方差椭圆、逆协方差缓存、`eps2/psi2`
  - Density evaluator：保存点云和 `flowDensitySigma`，提供 `density + gradient` 查询
  - Flow dispatcher：根据 `flowModel` 统一返回 `FlowFieldSample`
- 事件级上下文改为统一的 flow context，至少包含：
  - `FlowEllipseInfo`
  - 逆协方差/广义半径计算所需缓存
  - density evaluator 状态
- 生成器流程保持薄耦合：
  - `sampleParticipants()` 后构建一次事件级 flow context
  - `sampleFlowVelocity()` 只调用统一 `evaluateFlowField(...)`

### `density-normal` 计算规则

- density 定义：`rho(x,y) = sum_i w_i G_sigma(x-xi, y-yi)`，当前继续使用 `w_i = 1`
- gradient 使用高斯解析导数，禁止数值差分。
- 法向定义：
  - 优先 `n = -grad(rho) / |grad(rho)|`
  - 若出现非有限值，或满足平坦区判据 `|grad rho| * sigma / max(rho, eps) < 1e-6`，回退到 `q = Sigma^{-1} delta` 的单位向量
  - 若 fallback 也退化到零向量或事件椭圆无效，则返回零横向流，禁止造出任意方向
- 半径与流速：
  - `rTilde = sqrt(delta^T Sigma^{-1} delta)`
  - `rhoRaw = rho0 * pow(rTilde, flowPower)`
  - `phiB = atan2(ny, nx)`
  - `betaT = min(0.95, tanh(max(0, rhoRaw)))`
  - `betaX = betaT * nx`, `betaY = betaT * ny`
- `events.eps2/psi2` 继续保持当前协方差定义，不随流场模型切换改变语义。

## Validation

- ROOT-free 测试拆成“density 数学正确性”和“flow dispatch 回归”两类：
  - 单高斯/对称点云下，gradient 方向与解析预期一致
  - 近平坦区和精确中心不出现 NaN，且按规则 fallback 或归零
  - `density-normal` 的 `rTilde` 与协方差几何 helper 一致
  - 现有 `covariance-ellipse` 行为回归不变
  - CLI/config 能正确解析新旧两种模型，默认值保持当前行为
- ROOT/运行验证首版只做最小闭环：
  - 现有 mandatory ROOT contract 不变
  - `debug-flow-ellipse` 语义不变，两种模型下仍可输出椭圆 debug
  - 不在首版新增 density contour / arrow ROOT payload；这部分留作后续专门 QA 可视化扩展
- 实施后验证顺序固定为：
  - `ctest --output-on-failure`
  - authoritative O2Physics 生成+QA smoke 各跑一组：`covariance-ellipse` 与 `density-normal`

## Assumptions

- 默认模型不变，避免破坏现有示例配置、QA 基线和现有文档主叙述。
- `flow-density-sigma` 与 `smearSigma` 分离；两者数值默认都可为 `0.5`，但物理职责不同。
- 首版不引入 `rho2 cos(2phi)` 到 `density-normal`，也不做网格插值/缓存表；每次查询直接对 participant 点云求和，复杂度以当前 `Npart` 规模可接受为前提。
- 实施时需要同步更新人写文档与 `project-state/` 台账，至少覆盖算法语义、配置契约、测试记录和当前状态。
