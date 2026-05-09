# 方案二：基线快度 + 几何诱导修正项

> 运行时映射：`flow-velocity-sampler=affine-effective` + `affine-effective-mode=additive-rho`（默认）。

## 目标

在保留现有发射方向定义和 baseline 快度框架的前提下，引入一个非显式谐波的几何修正项，使横向流快度写成：

\[
\rho(\tilde r,\phi) = \rho_{\rm iso}(\tilde r) + \delta\rho(\tilde r,\phi)
\]

其中：
- \(\rho_{\rm iso}\) 是各向同性基线快度；
- \(\delta\rho\) 是由 affine 末态几何自动生成的修正项；
- 不显式引入 `rho2 * cos(2phi)`；
- 修正项只改变流速大小，不必重写方向模块。

该方案主要面向已有以下结构的代码：
- 已有 baseline \(\rho(\tilde r)\)
- 已有流体元方向定义（如径向、法向、梯度方向）
- 希望尽量少改原代码

---

## 物理定位

本方案不直接生成新的完整速度场，而是做以下分解：

\[
\text{真实几何诱导的快度}
=
\text{各向同性基线}
+
\text{角向修正}
\]

这里的几何修正项来源于：
- freeze-out 主轴尺度差异
- 累计几何膨胀量差异
- 位置方位角与主轴方向的关系

因此，虽然数学上它仍包含主导二阶结构，但代码层面不需要手工写入固定谐波项。

---

## 输入

### 来自现有几何模块的输入
- `Psi2`：freeze-out 主轴方向
- `sigma_in_0`, `sigma_out_0`
- `sigma_in_f`, `sigma_out_f`
- `R_in_f`, `R_out_f`
- `rho_f(x, y)`

### 来自现有发射模块的输入
- baseline 各向同性快度定义 `rho_iso_base(tilde_r)`  
  或可替代为新的 `rho_iso(tilde_r)`
- 现有方向模块  
  例如：
  - 径向方向
  - 椭圆法向方向
  - 密度梯度方向

### 全局控制参数
- `DeltaTauRef`
- `kappa_flow`
- `u_max`
- `radial_profile(tilde_r)`

---

## 核心变量

### 1. 实际几何增长因子

定义：

\[
G_{\rm in} = \frac{\sigma_{\rm in}^f}{\sigma_{\rm in}^0}, \qquad
G_{\rm out} = \frac{\sigma_{\rm out}^f}{\sigma_{\rm out}^0}
\]

以及累计膨胀量：

\[
\Lambda_{\rm in} = \ln G_{\rm in}, \qquad
\Lambda_{\rm out} = \ln G_{\rm out}
\]

对应有效主轴膨胀率：

\[
H_{\rm in}^{\rm eff} = \frac{\Lambda_{\rm in}}{\Delta\tau_{\rm ref}},
\qquad
H_{\rm out}^{\rm eff} = \frac{\Lambda_{\rm out}}{\Delta\tau_{\rm ref}}
\]

---

### 2. 椭圆约化半径

在主轴系中定义：

$$
[
\tilde r^2 =
\frac{x'^2}{(R_{\rm in}^f)^2}
+
\frac{y'^2}{(R_{\rm out}^f)^2}
]
$$

其中 \((x',y')\) 为旋转到主轴系后的坐标。

---

## 基线快度

本方案允许两种选择：

### 选择 A：复用现有 baseline
直接使用原代码已有的各向同性快度：
$[
\rho_{\rm iso}(\tilde r)=\rho_{\rm iso}^{\rm base}(\tilde r)
]$

### 选择 B：用几何平均定义新的 baseline
定义平均膨胀率与平均尺度：

$[
\bar H=\frac{H_{\rm in}^{\rm eff}+H_{\rm out}^{\rm eff}}{2}
]$

$[
\bar R=\sqrt{R_{\rm in}^f R_{\rm out}^f}
]$

再定义：

$[
u_{\rm iso}(\tilde r)=k_{\rm flow}\, f(\tilde r)\,\bar H\,\tilde r\,\bar R
]$

$[
\rho_{\rm iso}(\tilde r)=\operatorname{arctanh}(u_{\rm iso})
]$

该 baseline 只保留平均膨胀，不包含方位角依赖。

---

## 几何诱导快度

在主轴系中，对给定 \((\tilde r,\phi_s)\) 定义几何诱导的有效速度大小：

\[
u_{\rm geom}(\tilde r,\phi_s)=
k_{\rm flow}\, f(\tilde r)\,\tilde r
\sqrt{
(H_{\rm in}^{\rm eff}R_{\rm in}^f\cos\phi_s)^2
+
(H_{\rm out}^{\rm eff}R_{\rm out}^f\sin\phi_s)^2
}
\]

若 \(u_{\rm geom}>u_{\max}\)，则截断到上限。

再定义：

\[
\rho_{\rm geom}(\tilde r,\phi_s)=\operatorname{arctanh}(u_{\rm geom})
\]

这一步得到的是几何自动诱导出的总快度，而不是手工规定的谐波修正。

---

## 修正项定义

最终定义修正项：

\[
\delta\rho(\tilde r,\phi_s)=
\rho_{\rm geom}(\tilde r,\phi_s)
-
\rho_{\rm iso}(\tilde r)
\]

于是总快度为：

\[
\rho(\tilde r,\phi_s)=\rho_{\rm iso}(\tilde r)+\delta\rho(\tilde r,\phi_s)
\]

该写法的意义是：

- 代码层面只暴露“基线 + 修正项”接口；
- 物理层面修正项来自几何自动生成；
- 不需要在任何位置显式写入固定的 `cos(2phi)`。

---

## 方向处理

本方案默认**不重写现有方向模块**。

也就是说：

- 方向仍由原代码给出；
- 修正项只改变流快度大小；
- 适合已有以下结构的代码：
  - 方向来自密度梯度
  - 方向来自椭圆法向
  - 方向来自某个原有的几何规则

因此本方案的重点是：

\[
\text{只补大小，不重写方向}
\]

这也是它和完整流场生成方案最大的区别。

---

## 与现有代码的耦合

本方案适合最小侵入式接入。

### 插入位置
- affine + smoothing 之后
- 粒子热抽样之前

### 接口形式
若原代码已有：
- `rho = rho_base(tilde_r)`
则将其替换为：
- `rho = rho_iso(tilde_r) + delta_rho(tilde_r, phi_s)`

若方向模块独立存在，则保持不变。

### 保持不变的部分
- freeze-out density 生成
- 发射方向模块
- 热抽样
- boost 流程

---

## 优势

1. **最小侵入式**
   - 不需要重写整个速度场模块
   - 容易接入已有 baseline 快度框架

2. **方向与大小可分离**
   - 保留原有方向定义
   - 仅对大小做几何修正

3. **不显式写谐波**
   - 修正项来自几何自动生成
   - 代码结构上避免固定阶数参数接口

4. **便于调试**
   - baseline 与修正项可单独输出与检查
   - 易于判断问题来自：
     - baseline 太弱
     - 修正项太小
     - 或方向模块本身不合理

---

## 边界

1. 该方案不直接给出新的完整局域速度矢量场；
2. 若 baseline 和方向模块本身物理结构不合理，单独修正 \(\rho\) 不能完全解决问题；
3. 对纯椭圆 affine 几何，修正项虽不是显式 cos 项，但主导模态仍主要是二阶；
4. 更高阶结构不会凭空产生，除非方向或几何本身含有更高阶信息。

---

## 标定重点

### 几何侧
- freeze-out 主轴宽度
- freeze-out 空间偏心率

### 动量侧
- 平均 \(\rho\) 强度
- \(v_2(p_T)\)
- \(dN/d(\phi-\Psi_2)\)

### 接口一致性
- baseline 单独输出是否合理
- \(\delta\rho\) 的符号与大小是否与主轴差异一致
- 修正后总快度是否存在非物理振荡
- 无超光速点

---

## 成功标准

实现后应满足：

1. baseline 快度仍决定平均径向流；
2. 修正项只负责将 affine 几何差异转成角向快度变化；
3. 代码层面不出现显式 `rho2 * cos(2phi)`；
4. 在尽量少改原代码的前提下，\(\phi\) 调制和 \(v_2\) 能得到增强或更合理的匹配。

---

## 总结

本方案的本质是：

- 先保留一个各向同性 baseline 快度；
- 再利用 affine 几何的主轴差异生成一个几何诱导修正项；
- 最终用
  \[
  \rho = \rho_{\rm iso} + \delta\rho
  \]
  的形式接入现有代码。

它不是新的完整流场生成模块，而是一个适合旧代码接口的非显式谐波修正层。
