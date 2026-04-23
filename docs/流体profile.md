任务说明

实现一个“基于 smeared density 的横向流场模块”，目标是对每个流体元位置 (x,y) 输出横向流快度 \rho_t 与横向 boost 方向角 \phi_b。

已有资产

* 已有协方差矩阵/全局二阶矩实现
* 已有基于协方差矩阵定义广义半径
* 这些资产应尽量复用

实现要求

1. 用点源 (x_i,y_i,w_i) 和高斯核 G_\sigma 构造二维 smeared density：
    \rho(x,y)=\sum_i w_i G_\sigma(x-x_i,y-y_i)
2. 用解析表达式计算 density gradient：
    \partial_x\rho,\ \partial_y\rho
3. 流方向定义为 outward density normal：
    \hat{\mathbf n}=-\nabla\rho/|\nabla\rho|
4. 若梯度模长小于阈值，则 fallback 到协方差定义的广义径向法向：
    \mathbf q=\Sigma^{-1}\mathbf x,\qquad
    \hat{\mathbf n}=\mathbf q/|\mathbf q|
5. 复用已有协方差矩阵定义广义半径：
    \tilde r^2=\mathbf x^T\Sigma^{-1}\mathbf x
6. 横向流快度定义为：
    \rho_t=\rho_0 \tilde r^\alpha
    默认 \alpha=1
7. 横向 boost 方向角：
    \phi_b=\operatorname{atan2}(n_y,n_x)
8. 横向速度大小与分量：
    v_T=\tanh\rho_t,\quad
    v_x=v_T\cos\phi_b,\quad
    v_y=v_T\sin\phi_b

设计要求

* 模块化拆分为：
    * density evaluator
    * global geometry
    * transverse flow field
* 所有数值路径避免 NaN
* 对近中心、近零梯度区有稳定 fallback
* 首版不要额外叠加解析 \rho_2\cos2\phi 项

输出

对任意 (x,y) 查询，返回：

* \rho_t
* \phi_b
* v_T
* v_x,v_y

验证

* 绘制 density 等高线与流方向箭头图
* 绘制 v_T 或 \rho_t 的二维分布
* 检查方向场平滑、整体向外、中心无异常