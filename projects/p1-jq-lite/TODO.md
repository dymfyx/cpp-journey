# P1 收尾清单（P2 期间穿插做，别忘）

P1 的「主线」已完成（68 测试、JSONTestSuite、fuzz、benchmark、install/export、CI）。下面这些是当时说好「后面补」的，按优先级排：

## 1. 流式输入（SAX / 增量解析）· 优先级最高

- [ ] 新增 `jq::Parser` 的事件式（SAX）接口：`on_null/on_bool/on_number/on_string/on_array_begin/...`，或一个 `parse_stream(std::istream&, Handler&)`；
- [ ] 大小写：一次只吃固定大小的缓冲区，**内存占用与文件大小无关**；
- [ ] NDJSON（每行一个 JSON）支持：逐行解析、逐条回调，作为 CLI 的 `--stream` 子命令；
- [ ] 验收：在 1 GB 生成文件上跑 `jq_lite --stream`，峰值内存 < 10 MB（用 `/usr/bin/time -v` 记录）；NDJSON 每条都能正确输出；
- [ ] 与现有 DOM 接口共存：`parse()` 内部可以复用 SAX 层来构建 DOM（这次重构正好把解析器拆成「词法 + 事件 + DOM 构建」三层）。

## 2. tokenizer.json 实战

- [ ] 用 jq_lite 解析 HuggingFace 的 `tokenizer.json`，打印词表大小、查某个 token 的 id、读 merges 数量；
- [ ] 做成 `examples/tokenizer_stats.cpp`，输出写进 README；
- [ ] 这是 P5 MiniInfer 的 tokenizer 数据前置，格式先摸清。

## 3. 复盘博客

- [ ] 写一篇项目复盘（做了什么、踩了什么坑、to_chars 优化、ASan 组合删除实验），发到仓库 `docs/` 或掘金/知乎；
- [ ] 博客链接写进 README。

## 4. 可选

- [ ] CLI 接入 JSON Pointer（库函数 `evaluate_pointer` 已有，只在 `main.cpp` 加 `--pointer` 分支）；
- [ ] fuzz 跑满 24 小时（`./build/jq_fuzz 10000000 <seed>`），结果写进 README；
- [ ] 给仓库加 MIT License 文件。
