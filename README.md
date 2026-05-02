# Blast_wave

一个基于 C++17 和 ROOT 的小型 Blast-Wave 事例生成器仓库。当前主线保持三层职责分离：

- `include/` + `src/`：ROOT-free 核心物理与输出契约实现
- `apps/`：ROOT 相关的命令行入口，包括生成器、QA 和独立 `v2{2}(pT)` 分析器
- `tests/`：不依赖 ROOT 文件 I/O 的核心回归测试

## 当前仓库结构

- `include/blastwave/`
  公共头文件与对外数据契约。
- `src/`
  生成器核心、热动量采样器和 ROOT 输出实现。
- `apps/`
  生成、QA 和独立 `v2{2}(pT)` 分析入口。
- `tests/`
  当前的 ROOT-free 单元/回归测试，包括 `v2{2}(pT)` cumulant 回归。
- `config/`
  受版本控制的示例配置文件。
- `scripts/`
  本地运行辅助脚本，不与静态配置混放。
- `reference/legacy-root-macros/`
  历史 ROOT 参考宏，只用于理解物理意图与对照旧思路，不参与构建，也不应直接复制到主实现。
- `docs/`
  人写说明、agent 指南和简明手记。
- `project-state/`
  仓库内工程协作台账。

## 本地产物目录

- `build/`、`bin/`
  本地构建产物。
- `qa/`
  本地 smoke/QA ROOT 输出。
- `res/`
  本地较长运行结果。

这些目录默认不纳入版本控制。

## 入口文档

- 项目说明：[docs/项目说明.md](/Users/allenzhou/Research_software/Blast_wave/docs/项目说明.md)
  当前最详细的运行与物理说明。
- 数学/物理公式流程说明：[docs/数学物理公式流程说明.md](/Users/allenzhou/Research_software/Blast_wave/docs/数学物理公式流程说明.md)
  公式导向地串起 participant 几何、密度演化、发射抽样、热动量、流场 boost 和 flow 观测量。
- Agent 指南：[docs/agent_guide.md](/Users/allenzhou/Research_software/Blast_wave/docs/agent_guide.md)
  只保留 agent 需要的语义边界与同步规则。
- 简明手记：[docs/手记文档.md](/Users/allenzhou/Research_software/Blast_wave/docs/手记文档.md)
  快速查看主链和常见误解。
- 当前协作状态：[project-state/guide.md](/Users/allenzhou/Research_software/Blast_wave/project-state/guide.md)
  查看当前协作基线与文档分工。
