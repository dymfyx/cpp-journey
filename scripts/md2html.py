#!/usr/bin/env python3
"""把合并后的 Markdown 转换成用于 Edge 打印的 HTML。"""
import sys
import markdown

CSS = r"""
@page { size: A4; margin: 17mm 15mm 15mm 15mm; }
* { box-sizing: border-box; }
body {
  font-family: "Microsoft YaHei", "PingFang SC", "Source Han Sans SC", sans-serif;
  font-size: 10.2pt; line-height: 1.62; color: #1b1b1b; margin: 0;
}
h1 {
  font-size: 19pt; color: #0b4f8a; margin: 0 0 14px;
  padding-bottom: 6px; border-bottom: 2.5px solid #0b4f8a;
  page-break-before: always; page-break-after: avoid;
}
h1.first { page-break-before: auto; }
h2 { font-size: 14.5pt; color: #11365c; margin: 20px 0 8px; page-break-after: avoid; }
h3 { font-size: 12pt; color: #175c9e; margin: 16px 0 6px; page-break-after: avoid; }
h4 { font-size: 10.8pt; color: #333; margin: 12px 0 4px; page-break-after: avoid; }
p { margin: 6px 0; text-align: justify; }
ul, ol { margin: 6px 0 6px 0; padding-left: 22px; }
li { margin: 3px 0; }
code {
  font-family: Consolas, "Cascadia Mono", "Courier New", monospace;
  font-size: 8.9pt; background: #f2f4f7; padding: 1px 4px; border-radius: 3px;
  color: #8a1c1c;
}
pre {
  background: #f7f9fb; border: 1px solid #d9e2ec; border-left: 3px solid #0b4f8a;
  border-radius: 4px; padding: 9px 11px; margin: 8px 0;
  white-space: pre-wrap; word-break: break-word; page-break-inside: avoid;
}
pre code { background: none; padding: 0; color: #1f2933; font-size: 8.6pt; line-height: 1.45; }
table {
  width: 100%; border-collapse: collapse; margin: 9px 0;
  font-size: 9.2pt; page-break-inside: avoid;
}
th, td { border: 1px solid #c8d3df; padding: 5px 7px; vertical-align: top; }
th { background: #eaf1f8; font-weight: 600; text-align: left; }
tr:nth-child(even) td { background: #fbfcfe; }
blockquote {
  margin: 8px 0; padding: 6px 12px; border-left: 3px solid #c9a227;
  background: #fdf9ea; color: #4a3f18;
}
blockquote p { margin: 3px 0; }
hr { border: none; border-top: 1px solid #d5dde6; margin: 16px 0; }
.toc a { text-decoration: none; color: #175c9e; }
strong { color: #111; }
"""


def main() -> None:
    src, dst = sys.argv[1], sys.argv[2]
    with open(src, encoding="utf-8") as f:
        text = f.read()

    body = markdown.markdown(
        text,
        extensions=["tables", "fenced_code", "toc", "sane_lists"],
        extension_configs={"toc": {"toc_depth": "1-3"}},
    )
    # 第一个 h1（封面）不强制分页
    body = body.replace("<h1", '<h1 class="first"', 1) if body.lstrip().startswith("<h1") else body

    html = (
        "<!DOCTYPE html><html lang=\"zh-CN\"><head><meta charset=\"utf-8\">"
        "<title>现代 C++ 学习复盘手册</title>"
        f"<style>{CSS}</style></head><body>{body}</body></html>"
    )
    with open(dst, "w", encoding="utf-8") as f:
        f.write(html)
    print(f"html written: {dst} ({len(html)} chars)")


if __name__ == "__main__":
    main()
