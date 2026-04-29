# Affine 累积几何方案：基于实际几何增长的有效各向异性流场闭合

## 目标

在保留现有 **affine 膨胀 + smoothing** 主流程不变的前提下，新增一个“**几何 -> 有效流场**”闭合层，使模型满足以下目标：

1. 将 affine 方案解释为 **累计几何效果**，而不是显式的瞬时流体动力学求解。
2. 使用 **实际初末态几何宽度变化** 构造一个有效横向流场。
3. 在不破坏已有 freeze-out density 形状的前提下，引入可控的 **各向异性流补偿**。
4. 让后续粒子抽样与 boost 仍使用原有代码框架，只替换/新增局域流速场定义。
5. 保证平均膨胀和各向异性补偿可分离调节，便于标定。

---

## 物理解释

本方案不把输入的 affine 参数直接解释为瞬时速度或真实演化时间，而是将其视为：

- 从初态几何到末态几何的 **累计变形**
- 该累计变形所对应的 **有效积分膨胀量**
- 再通过一个约定的时间尺度，将其映射为 **有效横向流速大小**

这里反推出的不是唯一真实流场，而是一个与现有 affine 几何相容、可用于发射抽样的 **effective flow closure**。

---

## 输入

需要从现有代码中读取或计算以下量：

### 几何输入
- 初态主轴系方向 `Psi2`
- 初态主轴宽度  
  - `sigma_in_0`
  - `sigma_out_0`
- 末态（经过 affine + smoothing 后）的主轴宽度  
  - `sigma_in_f`
  - `sigma_out_f`

### freeze-out 输入
- smoothed freeze-out density `rho_f(x, y)`
- 末态主轴尺度  
  - `R_in_f`
  - `R_out_f`

### 全局配置参数
- `DeltaTauRef`：参考时间尺度  
  - 只作为“累计膨胀量 -> 有效膨胀率”的单位换算参数  
  - 不是事件内反解量
- `kappa_flow`：整体流强度系数
- `kappa_aniso`：各向异性补偿系数
- `u_max`：局域横向速度上限（避免超光速）
- `radial_profile(tilde_r)`：径向强度轮廓函数，建议单调递增
- `shell_weight(x, y)`：可选的壳层发射权重

---

## 核心变量定义

### 1. 实际几何增长因子

不要直接使用输入的 `affine-lambda-in/out`，而是使用 **最终真实输出宽度** 定义增长因子：

\[
G_{\rm in} = \frac{\sigma_{\rm in}^f}{\sigma_{\rm in}^0}, \qquad
G_{\rm out} = \frac{\sigma_{\rm out}^f}{\sigma_{\rm out}^0}
\]

理由：
- smoothing 会改变最终二阶矩
- 实际输出宽度比输入 affine 参数更接近模型最终几何结果
- 补偿应建立在“真实传播后的几何”上，而不是建立在“输入意图”上

---

### 2. 积分膨胀量

定义：

\[
\Lambda_{\rm in} = \ln G_{\rm in}, \qquad
\Lambda_{\rm out} = \ln G_{\rm out}
\]

并进一步定义：

\[
\bar{\Lambda} = \frac{\Lambda_{\rm in} + \Lambda_{\rm out}}{2}
\]

\[
\delta\Lambda = \frac{\Lambda_{\rm in} - \Lambda_{\rm out}}{2}
\]

解释：
- `barΛ` 表示平均累计膨胀量
- `deltaΛ` 表示累计几何各向异性

---

### 3. 各向异性补偿后的有效膨胀率

用一个单独参数 `kappa_aniso` 调节各向异性部分，而尽量不改变平均膨胀：

\[
H_{\rm in}^{\rm eff} =
\frac{\bar{\Lambda} + \kappa_{\rm aniso}\,\delta\Lambda}{\Delta\tau_{\rm ref}}
\]

\[
H_{\rm out}^{\rm eff} =
\frac{\bar{\Lambda} - \kappa_{\rm aniso}\,\delta\Lambda}{\Delta\tau_{\rm ref}}
\]

其中：

- `kappa_aniso = 1`：直接使用几何传播得到的各向异性
- `kappa_aniso > 1`：增强各向异性流补偿
- `kappa_aniso < 1`：减弱各向异性流补偿

这一步是本方案的关键：  
**只放大/缩小各向异性部分，不直接重写整个流场。**

---

## 流场构造

### 1. 坐标系

先将末态位置 \((x, y)\) 旋转到 freeze-out 主轴系 \((x', y')\)：

- `x'`：in-plane 主轴方向
- `y'`：out-of-plane 主轴方向

---

### 2. 约化半径

定义椭圆约化半径：

\[
\tilde r^2 =
\frac{x'^2}{(R_{\rm in}^f)^2}
+
\frac{y'^2}{(R_{\rm out}^f)^2}
\]

说明：
- `tilde_r` 用于控制流速大小的径向轮廓
- 主轴尺度使用末态 freeze-out 几何

---

### 3. 有效线性流场

在主轴系中构造一个自相似、各向异性线性流场：

\[
u_{x'} =
\kappa_{\rm flow}\,
f(\tilde r)\,
H_{\rm in}^{\rm eff}\,
x'
\]

\[
u_{y'} =
\kappa_{\rm flow}\,
f(\tilde r)\,
H_{\rm out}^{\rm eff}\,
y'
\]

其中：

- `f(tilde_r)` 对应 `radial_profile(tilde_r)`
- 建议满足  
  - \(f(0)=0\)
  - 随 \(\tilde r\) 单调增加
  - 外层更强，中心更弱

推荐目标：
- 保持外层主导各向异性
- 避免慢速内核稀释角分布
- 不强制引入额外固定阶数结构

---

### 4. 速度裁剪

定义横向速度大小：

\[
u_T = \sqrt{u_{x'}^2 + u_{y'}^2}
\]

若 \(u_T > u_{\max}\)，则按比例缩放 \((u_{x'}, u_{y'})\)，保证：

\[
u_T < 1
\]

---

### 5. 横向流快度

将速度转换为横向流快度：

\[
\rho(x, y) = \operatorname{arctanh}(u_T)
\]

后续粒子抽样和 boost 使用该局域速度场。

---

## 可选：兼容传统 Blast-Wave 参数接口

若原代码某部分仍需要 \((\rho_0, \rho_2)\) 形式，可从表面速度构造有效参数。

先定义主轴表面速度：

\[
v_{\rm in}^{\rm surf}
=
\min\!\left(
\kappa_{\rm flow}\,H_{\rm in}^{\rm eff}\,R_{\rm in}^f,
u_{\max}
\right)
\]

\[
v_{\rm out}^{\rm surf}
=
\min\!\left(
\kappa_{\rm flow}\,H_{\rm out}^{\rm eff}\,R_{\rm out}^f,
u_{\max}
\right)
\]

再定义：

\[
\rho_{\rm in} = \operatorname{arctanh}(v_{\rm in}^{\rm surf})
\]

\[
\rho_{\rm out} = \operatorname{arctanh}(v_{\rm out}^{\rm surf})
\]

最后得到：

\[
\rho_0^{\rm eff} = \frac{\rho_{\rm in} + \rho_{\rm out}}{2}
\]

\[
\rho_2^{\rm eff} = \frac{\rho_{\rm in} - \rho_{\rm out}}{4}
\]

说明：
- 这一层仅用于兼容旧接口
- 主推荐实现仍是直接使用局域速度场 \((u_{x'}, u_{y'})\)

---

## 发射权重（建议加入）

为了避免中心慢速区域稀释各向异性，建议在原有发射抽样上加入一个壳层偏置：

### 方案 A：仅用径向增强
发射权重乘以：
\[
w_{\rm shell} = g(\tilde r)
\]
其中 \(g(\tilde r)\) 随 \(\tilde r\) 单调增加。

### 方案 B：使用 freeze-out density 壳层
围绕某个 freeze-out 密度阈值增加发射权重，只增强外层而不显著改变末态整体几何。

目标：
- 强化快流外层的贡献
- 减少内核近各向同性发射对 \(\phi\) 分布的稀释
- 不改变 affine 传播后的整体密度拓扑

---

## 标定策略

### 固定项
- `DeltaTauRef` 作为全局约定值固定
- 不对每个事件单独反解时间尺度
- 不从 `lambda_in/out` 直接生成流场

### 待调参数
- `kappa_flow`：整体流强度
- `kappa_aniso`：各向异性流补偿强度
- `radial_profile`
- `shell_weight`

### 推荐调参顺序
1. 先固定 `DeltaTauRef`
2. 调 `kappa_flow` 使平均横向流强度落入目标区间
3. 调 `kappa_aniso` 使 \(v_2\)、\(\phi\) 分布、freeze-out 偏心率同时更接近目标
4. 最后调 `shell_weight` 抑制内核洗平效应

---

## 与原代码耦合方式

本方案应作为 **affine + smoothing 之后、粒子发射之前** 的一个独立闭合层插入。

### 需要复用的现有结果
- 主轴方向
- 初末态主轴宽度
- smoothed freeze-out density
- 原有粒子发射与 boost 流程

### 需要新增的模块
1. 主轴宽度测量模块
2. 有效膨胀量计算模块
3. 局域有效流场构造模块
4. 可选壳层权重模块

### 不建议修改的部分
- affine 几何传播主流程
- smoothing 主流程
- 原有热抽样框架

本方案应尽量以“新增闭合层”的方式并入，而不是重写原有演化模块。

---

## 验证目标

实现后至少检查以下量：

### 几何侧
- freeze-out 主轴宽度
- freeze-out 空间偏心率
- affine 传播后的整体几何是否保持稳定

### 动量侧
- 平均 \(p_T\)
- \(v_2(p_T)\)
- \(dN/d(\phi-\Psi_2)\) 的二阶调制是否与计算得到的 \(v_2\) 一致

### 一致性
- 从 \(\phi\) 直方图拟合得到的二阶系数应与直接计算的 \(v_2\) 相符
- 局域速度场无超光速点
- 主轴方向定义在几何与流场中保持一致

---

## 成功判据

若实现正确，应满足：

1. affine 方案仍主要决定 freeze-out 几何
2. 流场补偿主要改变动量各向异性，而不是重新塑造几何
3. 调 `kappa_aniso` 时，平均膨胀不应被大幅破坏
4. 加入壳层权重后，\(\phi\) 分布调制应增强，但几何主轴与整体拓扑不应明显漂移

---

## 总结

本方案的核心思想是：

- 将 affine 结果视为 **累计几何变形**
- 使用实际输出宽度构造 **积分膨胀量**
- 通过一个固定参考时间将其映射为 **有效膨胀率**
- 再用一个可控的各向异性补偿参数，将几何各向异性转换为可用于抽样的 **有效横向流场**

这不是对真实流体动力学的唯一反演，而是一个与现有 affine 代码兼容、可调、物理解释清晰的 **effective closure**。