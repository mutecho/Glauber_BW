# 项目概览

## 项目目的

`Blast_wave` 是一个基于 C++17 和 ROOT 的小型 blast-wave 事例生成器。它当前面向固定冲量参数 `b` 的 Pb-Pb 风格重离子碰撞玩具模型，重点是保留 Monte Carlo Glauber 的逐事例 participant 几何涨落，并在此基础上生成单一直接 `pi+` 的末态粒子样本。

这个仓库的目标不是提供完整的重离子末态模拟，而是提供一个体量小、物理语义清晰、便于继续扩展和验证的研究代码骨架。更高权威的人类说明文档位于 [docs/项目说明.md](/Users/allenzhou/Research_software/Blast_wave/docs/项目说明.md) 和 [docs/agent_guide.md](/Users/allenzhou/Research_software/Blast_wave/docs/agent_guide.md)；本文件只维护当前仓库基线下的工程协调视图。

## 项目架构

- 核心物理层位于 [include/blastwave/BlastWaveGenerator.h](/Users/allenzhou/Research_software/Blast_wave/include/blastwave/BlastWaveGenerator.h) 和 [src/BlastWaveGenerator.cpp](/Users/allenzhou/Research_software/Blast_wave/src/BlastWaveGenerator.cpp)。
  这里实现 Woods-Saxon 核几何、participant 抽样、`eps2`/`psi2` 计算、热点发射、热动量抽样、流场和 Lorentz boost。该层保持 ROOT 无关。
- 热动量大小的预计算查表组件位于 [include/blastwave/MaxwellJuttnerMomentumSampler.h](/Users/allenzhou/Research_software/Blast_wave/include/blastwave/MaxwellJuttnerMomentumSampler.h) 和 [src/MaxwellJuttnerMomentumSampler.cpp](/Users/allenzhou/Research_software/Blast_wave/src/MaxwellJuttnerMomentumSampler.cpp)。
  当前默认模式是 `maxwell-juttner`，显式 `gamma` 兼容模式仍然保留。
- ROOT 输出契约位于 [include/blastwave/io/RootOutputSchema.h](/Users/allenzhou/Research_software/Blast_wave/include/blastwave/io/RootOutputSchema.h) 和 [src/RootOutputSchema.cpp](/Users/allenzhou/Research_software/Blast_wave/src/RootOutputSchema.cpp)。
  当前公开输出包含 `events`、`participants`、`particles` 三棵树，以及 `Npart`、`eps2`、`psi2`、`cent`、`participant_x-y`、`participant_x-y_canvas` 等对象。
- 生成端入口位于 [apps/generate_blastwave_events.cpp](/Users/allenzhou/Research_software/Blast_wave/apps/generate_blastwave_events.cpp)。
  它负责解析 CLI 与配置文件、驱动生成器，并把结果写成 ROOT 文件。
- 独立 QA 读取端位于 [apps/qa_blastwave_output.cpp](/Users/allenzhou/Research_software/Blast_wave/apps/qa_blastwave_output.cpp)。
  它负责重新读取 ROOT 文件并检查树结构、participant 契约、质量壳、`centrality` 映射和固定 `b` 下的中心度一致性。
- 受版本控制的配置文件位于 [config/test_b8.cfg](/Users/allenzhou/Research_software/Blast_wave/config/test_b8.cfg) 和 [config/b8.cfg](/Users/allenzhou/Research_software/Blast_wave/config/b8.cfg)。
  运行辅助脚本位于 [scripts/run_example_config.sh](/Users/allenzhou/Research_software/Blast_wave/scripts/run_example_config.sh)，从 `config/` 中读取示例配置。
- 历史 ROOT 参考宏位于 [reference/legacy-root-macros/README.md](/Users/allenzhou/Research_software/Blast_wave/reference/legacy-root-macros/README.md)。
  这些宏只用于理解旧思路，不参与当前构建与运行入口。

## 使用方法

### 编译

项目默认依赖本机 O2Physics/ROOT 环境：

```bash
/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc 'cmake --preset default -S /Users/allenzhou/Research_software/Blast_wave'"
/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc 'cmake --build /Users/allenzhou/Research_software/Blast_wave/build'"
```

ROOT-free core 回归测试可直接在 build 目录执行：

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

显式 CLI 参数优先级最高，其次是配置文件，最后才是内建默认值。当前实现还约定：配置文件里的相对 `output` 路径会相对配置文件所在目录解析，因此直接运行 `config/test_b8.cfg` 时，如果不额外传 `--output`，输出会默认落在 `config/` 目录下。

### 校验

```bash
/bin/zsh -lc "alienv setenv O2Physics/latest-master-o2 -c sh -lc '/Users/allenzhou/Research_software/Blast_wave/bin/qa_blastwave_output --input /Users/allenzhou/Research_software/Blast_wave/qa/test_b8_cfg_smoke.root --output /Users/allenzhou/Research_software/Blast_wave/qa/test_b8_cfg_smoke_validation.root --expect-nevents 5000'"
```

如果运行成功，QA 程序会打印 `validation_passed ...`，并生成新的 QA ROOT 文件。

## 当前协调重点

- 当前基线已经包含 `events.centrality` 分支和 `cent` 直方图，并把默认热动量模式切换为 `maxwell-juttner`。
- `project-state/` 是当前采用的协调台账路径；不要再使用旧写法 `.project-state/`。
- `config/test_b8.cfg` 现在已经是高权威文档、运行脚本和耐久验证记录一致使用的规范示例路径。
- 在 Codex 里跑 ROOT smoke 时，需要注意沙箱内的 `alienv`/ROOT PCM 访问仍然不可靠；若出现模块/PCM 报错，应改用沙箱外命令重跑，那个结果才是权威验证。
