# Blast-Wave 文档索引

本目录只维护当前说明和历史计划两层。当前契约优先看 `docs/项目说明.md`、`docs/数学物理公式流程说明.md`、`docs/手记文档.md` 和 `project-state/`；历史计划只用于追溯设计来源。

## 当前说明

- `项目说明.md`
  - 运行方式、配置契约、ROOT 输出结构、QA 行为和面向使用者的物理说明。
  - 当 CLI、配置项、输出 schema、QA 语义或用户运行方式变化时优先更新。
- `数学物理公式流程说明.md`
  - 公式导向的 participant 几何、密度演化、发射抽样、热动量、流场 boost 和 flow 观测量说明。
  - 当算法、公式、物理解释、模型链顺序或代码位置变化时优先更新。
- `手记文档.md`
  - 当前主链和容易混淆概念的简明提醒。
  - 只放高频语义提醒，不承载完整参数表或完整历史。

## 协作台账

- `../project-state/guide.md`
  - 当前协作视图、文档角色和维护规则。
- `../project-state/current-status.md`
  - 当前运行基线、验证图景和 caveat。
- `../project-state/handoff.md`
  - 最新可执行 handoff。
- `../project-state/tests.md`
  - 耐久验证证据摘要。
- `../project-state/decisions.md`
  - 长期有效的决策。
- `../project-state/changelog.md`
  - 简明历史条目。
- `../project-state/issues.md` 与 `../project-state/work-items.md`
  - 仍需关注的问题和后续工作。
- `../project-state/doc-sync-map.yml`
  - 按当前 `sync-project-knowledge` skill 规范维护的文档同步路由。

## 历史计划

- `PLAN/`
  - 历史设计稿、旧 handoff 和已执行计划归档。
  - 默认不作为当前契约来源；若历史计划里的结论仍有效，应迁移或总结到当前说明、`project-state/` 或决策台账中。

## 维护规则

- 不把当前说明写成按时间追加的 patch note；新内容应合并进对应章节。
- 不在多个文档重复完整参数表、完整 ROOT schema 或完整验证命令。
- 修改算法、配置、输出 schema、QA、测试策略或用户运行方式时，按 `project-state/doc-sync-map.yml` 找到需要检查和更新的文档。
- `AGENTS.md` 是当前 agent 工作规则来源；`docs/PLAN/agent_guide.md` 只是历史归档。
