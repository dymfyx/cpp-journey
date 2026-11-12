# JSONTestSuite（官方解析测试集）

本目录是 [nst/JSONTestSuite](https://github.com/nst/JSONTestSuite) 仓库 `test_parsing/` 的原样拷贝，
用于验证 `jq-lite` 是否符合 RFC 8259。上游使用 MIT 许可证，见仓库内 `LICENSE`。

- `y_*.json`（95 个）：**必须解析成功**（y = yes）
- `n_*.json`（188 个）：**必须解析失败**（n = no）
- `i_*.json`（35 个）：实现自由决定（i = implementation-defined），只统计不判错

重新获取（WSL 连不上 github.com，用 Windows PowerShell）：

```powershell
pwsh -File scripts/fetch-jsontestsuite.ps1
```

运行：

```bash
./build/jq_jsontestsuite                       # 默认读本目录
ctest --test-dir build -R JSONTestSuite -V     # 走 ctest
```

当前结果：`y_ 95/95`、`n_ 188/188`、`i_ 接受 24 / 拒绝 11`。

`n_structure_100000_opening_arrays.json`（10 万层 `[`）和
`n_structure_open_array_object.json`（25 万字节的嵌套对象数组）会触发递归过深，
由 `src/parser.cpp` 里的 `Depth_guard`（最大 512 层）抛 `ParseError` 拦下——
没有这个守卫时 ASan 会直接报 `stack-overflow` 并 abort。
