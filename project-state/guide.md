# 项目概览

## 项目目的

`Blast_wave` 是一个基于 C++17 和 ROOT 的小型 blast-wave 事例生成器。它当前面向固定冲量参数 `b` 的 Pb-Pb 风格重离子碰撞玩具模型，重点是保留 Monte Carlo Glauber 的逐事例 participant 几何涨落，并在此基础上生成单一直接 `pi+` 的末态粒子样本。

这个仓库的目标不是提供完整的重离子末态模拟，而是提供一个体量小、物理语义清晰、便于继续扩展和验证的研究代码骨架。更高权威的人类说明文档位于 [docs/项目说明.md](/Users/allenzhou/Research_software/Blast_wave/docs/项目说明.md) 和 [docs/agent_guide.md](/Users/allenzhou/Research_software/Blast_wave/docs/agent_guide.md)；本文件只维护当前仓库基线下的工程协调视图。

## 项目架构

- ROOT-free 核心物理层位于 [include/blastwave/BlastWaveGenerator.h](/Users/allenzhou/Research_software/Blast_wave/include/blastwave/BlastWaveGenerator.h) 及其对应实现：
  - [src/BlastWaveGenerator.cpp](/Users/allenzhou/Research_software/Blast_wave/src/BlastWaveGenerator.cpp)
  - [src/BlastWaveGeneratorGeometry.cpp](/Users/allenzhou/Research_software/Blast_wave/src/BlastWaveGeneratorGeometry.cpp)
  - [src/BlastWaveGeneratorSampling.cpp](/Users/allenzhou/Research_software/Blast_wave/src/BlastWaveGeneratorSampling.cpp)
  - [src/BlastWaveGeneratorValidation.cpp](/Users/allenzhou/Research_software/Blast_wave/src/BlastWaveGeneratorValidation.cpp)
  这里负责几何抽样、source 采样、流场、boost、以及生成器侧校验。
- 事件级介质和发射接口位于：
  - [include/blastwave/DensityFieldModel.h](/Users/allenzhou/Research_software/Blast_wave/include/blastwave/DensityFieldModel.h)
  - [include/blastwave/EventMedium.h](/Users/allenzhou/Research_software/Blast_wave/include/blastwave/EventMedium.h)
  - [include/blastwave/EmissionSampler.h](/Users/allenzhou/Research_software/Blast_wave/include/blastwave/EmissionSampler.h)
  - [src/EventMedium.cpp](/Users/allenzhou/Research_software/Blast_wave/src/EventMedium.cpp)
  - [src/EmissionSampler.cpp](/Users/allenzhou/Research_software/Blast_wave/src/EmissionSampler.cpp)
  `EventMedium` 现在区分 `participantGeometry`、`initialDensity`、`emissionDensity` 和 `emissionGeometry`；当前 `DensityEvolutionMode::None` 让 initial/emission 阶段相同。`EmissionSite` 是后续 density-field emission backend 的接入口，当前 backend 仍保持 participant hotspot smear。
- 流体元速度抽样模块位于 [include/blastwave/FlowFieldModel.h](/Users/allenzhou/Research_software/Blast_wave/include/blastwave/FlowFieldModel.h) 以及：
  - [src/FlowFieldGeometry.cpp](/Users/allenzhou/Research_software/Blast_wave/src/FlowFieldGeometry.cpp)
  - [src/FlowFieldDensity.cpp](/Users/allenzhou/Research_software/Blast_wave/src/FlowFieldDensity.cpp)
  - [src/FlowFieldModel.cpp](/Users/allenzhou/Research_software/Blast_wave/src/FlowFieldModel.cpp)
  当前默认速度抽样器是 emission geometry 的协方差椭圆法向流场；并列可选的 `density-normal` 会根据 `emissionDensity` gradient 取法向，再在平坦区回退到 `emissionGeometry` 协方差法向。公开参数面仍是 `rho0`、`rho2`、`flow-power`、`flow-velocity-sampler`、`flow-density-sigma`、`debug-flow-ellipse`。
- 共享物理工具位于 [include/blastwave/PhysicsUtils.h](/Users/allenzhou/Research_software/Blast_wave/include/blastwave/PhysicsUtils.h) 和 [src/PhysicsUtils.cpp](/Users/allenzhou/Research_software/Blast_wave/src/PhysicsUtils.cpp)。
  这里统一维护 `centrality`、粒子 `phi`、事件级 `v2` 等共享定义，避免生成端与 QA 端重复实现。
- 热动量大小的预计算查表组件位于 [include/blastwave/MaxwellJuttnerMomentumSampler.h](/Users/allenzhou/Research_software/Blast_wave/include/blastwave/MaxwellJuttnerMomentumSampler.h) 和 [src/MaxwellJuttnerMomentumSampler.cpp](/Users/allenzhou/Research_software/Blast_wave/src/MaxwellJuttnerMomentumSampler.cpp)。
  当前默认模式是 `maxwell-juttner`，显式 `gamma` 兼容模式仍然保留。
- ROOT 输出契约位于 [include/blastwave/io/RootOutputSchema.h](/Users/allenzhou/Research_software/Blast_wave/include/blastwave/io/RootOutputSchema.h) 和 [src/RootOutputSchema.cpp](/Users/allenzhou/Research_software/Blast_wave/src/RootOutputSchema.cpp)。
  当前强制输出包含：
  - `events`、`participants`、`particles` 三棵树
  - `events.v2`
  - `events.centrality`
  - `Npart`、`eps2`、`psi2`、`v2`、`cent`、`participant_x-y`、`participant_x-y_canvas`、`x-y`、`px-py`、`pT`、`eta`、`phi`
  仅当 `debug-flow-ellipse = true` 时，才额外输出：
  - `flow_ellipse_debug`
  - `flow_ellipse_participant_norm_x-y`
  仅当 `flow-velocity-sampler = density-normal` 时，才额外输出：
  - `density_normal_event_density_x-y`
- 生成端入口位于 [apps/generate_blastwave_events.cpp](/Users/allenzhou/Research_software/Blast_wave/apps/generate_blastwave_events.cpp)，其 app 层实现拆分在：
  - [apps/generate_blastwave/RunOptions.cpp](/Users/allenzhou/Research_software/Blast_wave/apps/generate_blastwave/RunOptions.cpp)
  - [apps/generate_blastwave/RootEventFileWriter.cpp](/Users/allenzhou/Research_software/Blast_wave/apps/generate_blastwave/RootEventFileWriter.cpp)
  这里负责 CLI/config 解析、进度条和 ROOT 写出。
- 独立 QA 读取端位于 [apps/qa_blastwave_output.cpp](/Users/allenzhou/Research_software/Blast_wave/apps/qa_blastwave_output.cpp)。
  它负责重新读取 ROOT 文件并检查树结构、participant 契约、质量壳、事件级 `v2`、`centrality` 映射，以及可选 flow-ellipse debug 对象的附加约束。
- 受版本控制的配置文件位于 [config/test_b8.cfg](/Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg) 和 [config/b8.cfg](/Users/allenzhou/Research_software/Blast_wave/config/b8.cfg)。
  运行辅助脚本位于 [scripts/run_example_config.sh](/Users/allenzhou/Research_software/Blast_wave/scripts/run_example_config.sh)，从 `config/` 中读取示例配置。
  该脚本现在会在进入 `alienv` 后核对当前 `ROOTSYS` 与缓存/二进制绑定的 ROOT 前缀；若发现 build/runtime ROOT 不一致，会先重配并重建 `generate_blastwave_events`，避免 mixed-ROOT 噪声。
- 历史 ROOT 参考宏位于 [reference/legacy-root-macros/README.md](/Users/allenzhou/Research_software/Blast_wave/reference/legacy-root-macros/README.md)。
  这些宏只用于理解旧思路，不参与当前构建与运行入口。

## 使用方法

### 编译

项目默认依赖本机 O2Physics/ROOT 环境：

```bash
/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc 'cmake --preset default -S /Users/allenzhou/Research_software/Blast_wave'"
/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc 'cmake --build /Users/allenzhou/Research_software/Blast_wave/build'"
```

ROOT-free 回归测试可直接在 build 目录执行：

```bash
cd /Users/allenzhou/Research_software/Blast_wave/build && ctest --output-on-failure
```

### 生成

直接用命令行参数：

```bash
/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events --nevents 100 --b 8 --output /Users/allenzhou/Research_software/Blast_wave/qa/example.root'"
```

用配置文件：

```bash
/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/generate_blastwave_events /Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg --output /Users/allenzhou/Research_software/Blast_wave/qa/test_b8_cfg_smoke.root'"
```

当前程序支持三种入口：

- `generate_blastwave_events [options]`
- `generate_blastwave_events --config <path> [options]`
- `generate_blastwave_events <config-path> [options]`

显式 CLI 参数优先级最高，其次是配置文件，最后才是内建默认值。配置文件里的相对 `output` 路径会相对配置文件所在目录解析。

### 校验

```bash
/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /Users/allenzhou/Research_software/Blast_wave/qa/test_b8_cfg_smoke.root --output /Users/allenzhou/Research_software/Blast_wave/qa/test_b8_cfg_smoke_validation.root --expect-nevents 5000'"
```

如果运行成功，QA 程序会打印 `validation_passed ...`，并生成新的 QA ROOT 文件。

## 当前协调重点

- 当前默认速度抽样器已经不是实验室原点径向流，而是协方差椭圆法向流场。
- 当前内部介质接口已经统一为 `EventMedium`；不要重新引入旧的 flow-context 命名或 writer 侧密度重建。
- `events.eps2` / `events.psi2` 仍来自 `participantGeometry`；未来 density evolution 只能改变 `emissionDensity` / `emissionGeometry`，除非明确版本化事件摘要语义。
- 当前流体元速度抽样公开参数面是：
  - `rho0`
  - `rho2`
  - `flow-power`
  - `flow-velocity-sampler`
  - `flow-density-sigma`
  - `debug-flow-ellipse`
  旧参数 `vmax`、`kappa2`、`r-ref` 会直接报错并给出迁移提示。
- 当前强制 ROOT 契约已经包含：
  - `events.centrality`
  - `events.v2`
  - `v2`
  - `cent`
- 当前可选 debug ROOT 契约是：
  - `flow_ellipse_debug`
  - `flow_ellipse_participant_norm_x-y`
  QA 对这两者采用“存在则验证，不存在则忽略”的策略。
- 当前 sampler-specific ROOT 契约是：
  - `density_normal_event_density_x-y`
  QA 对该对象也采用“存在则验证，不存在则忽略”的策略，但当前 writer 只会在 `density-normal` 抽样器下写出它。
- `project-state/` 是当前采用的协调台账路径；不要再使用旧写法 `.project-state/`。
- `config/test_b8.cfg` 现在仍是高权威文档、运行脚本和耐久验证记录一致使用的规范示例路径。
- 在 Codex 里跑 ROOT smoke 时，沙箱内 `alienv`/ROOT PCM 访问仍然不可靠；若出现模块/PCM 报错，应改用沙箱外命令重跑，那个结果才是权威验证。
- `qa/` 目录里保留了多代历史样本文件；其中一部分可能早于 `participants`、`centrality`、`v2` 或 flow-ellipse debug 扩展，不应默认当作当前契约样本。
