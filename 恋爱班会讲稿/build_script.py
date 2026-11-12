# -*- coding: utf-8 -*-
"""Build the 4th-speaker (D) script DOCX with PPT page markers."""
import re, sys, io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding="utf-8", errors="replace")

from docx import Document
from docx.shared import Pt, Cm, RGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH, WD_LINE_SPACING
from docx.enum.section import WD_SECTION
from docx.oxml.ns import qn
from docx.oxml import OxmlElement

OUT = r"C:\Users\dym54\Desktop\c++项目\恋爱班会讲稿\第四部分_可持续比浪漫更难_演讲人四_逐页标注.docx"

TITLE = "第四部分　行动：可持续比浪漫更难"

SECTIONS = [
    ("PPT 第 17 页", "P4-1 · 观点页", "15:00–15:40", [
        "谢谢C。约定已经写下来了——可它凭什么撑过期末周？这就是最后一部分要回答的事，也是最难的一段。",
        "结论只有一句：可持续，比浪漫更难。",
        "一份约定写在纸上，只要五分钟；要撑过一学期，靠的却是一些很便宜的重复动作。所以这一段，我们不谈坚持，也不谈自律，更不要求谁做得更好，只谈一个每周二十分钟的小循环。",
        "为什么偏偏是二十分钟？因为再长一点，它就会被当成一项任务，而任务总会被推迟。二十分钟刚好短到可以放在周日晚上，短到不用专门安排。关系能不能维持，靠的从来不是一次长谈，而是很多次很短的对话。",
    ]),
    ("PPT 第 18 页", "P4-2 · 图表页", "15:40–16:40", [
        "这个小循环，一共四步。",
        "第一步，只说事实。这周有几次约定没做到，只讲次数和时间，不讲「你总是」，也不讲「你从来」。「你总是迟到」是一句总结，「这周有两次晚了四十分钟」是一件事实。总结会让人想反驳，事实让人没法反驳，只能面对。",
        "第二步，说感受，不评价对方。「我等到十一点，有点慌」，这是感受；「你根本不在乎我」，这是评价。这两句话的差别，决定这场对话是继续，还是当场结束。",
        "第三步，只改一条，最多一条。而且这一条最好是具体的动作，不是态度。「以后多在乎我一点」没法执行，「改约提前两小时说」可以执行。",
        "为什么不一次改三条？因为改得越多，越难执行。三条一起改，通常三条都撑不过两周；只改一条，成功率反而高得多。",
        "第四步，写进下周的日历。没写进日历的约定，基本等于没约定。",
        "这四步是流程示意，不是心理干预方案，也不替代任何专业支持。你们要是有更合适的节奏，用自己的就好——有人适合每周，有人适合每月，也有人只在考试月用一次。用哪一版不重要，重要的是有一个固定的时间，让这些话有机会说出口。",
    ]),
    ("PPT 第 19 页", "P4-3 · 案例页", "16:40–17:40", [
        "再看一个正向的情境。还是前面案例里的那两位同学——同学甲和同学乙，还是化名。",
        "期中之后，他们开始每周花二十分钟复盘：第一次只谈事实，不追责，也只改了一条——临时改约，提前两小时说。",
        "到了考试月，他们启用了「降级相处」：见面从出去逛，改成一起自习。时间变短了，但预期是清楚的——几点开始，几点结束，谁先到。",
        "「降级」这两个字，容易被听成退步。其实不是。降级降的是形式，不是分量。原来一次约会三小时，考试月只剩一小时，可这一小时是确定的、不用猜的。一段关系在忙的时候最需要的，恰恰就是这种确定。",
        "这不是要推荐某种相处模式，也不是暗示这样才对。它只说明一件事：把期待降级，并且说出口，往往比硬撑原来的安排更容易维持。每对人都可以有完全不同的版本——有人每周复盘，有人每月一次，也有人根本不需要表。",
    ]),
    ("PPT 第 20 页", "P4-4 · 互动页", "17:40–18:30", [
        "那「这周我要改的一条」，大家最常写的都是什么？其实翻来覆去只有三句。",
        "第一句，先问一句「这周哪天有空」；第二句，把改约提前说；第三句，留一个晚上给自己。",
        "这三句都很小，小到不需要下决心。想不出别的，从第一句开始，也算数。",
        "小，本身就是设计。一个太大的一周目标，第二周就会放弃；一个小到不可能失败的动作，才有可能变成习惯。关系里的改变，很少靠一次决心，基本都靠这种小到不显眼的重复。",
    ]),
    ("PPT 第 21 页", "P4-5 · 收束＋行动卡", "18:30–19:30", [
        "最后，把二十分钟收成一张行动卡，一共五条。",
        "第一，今晚共建一份共同日历。第二，定一个月度约会预算和分担方式。第三，写下回复时效和临时改约的说法。第四，考试月启用降级相处。这四条，对应的就是前面三张表加一个循环，是从前面一路走下来自然的结果，不是另外一套新要求。",
        "第五，如果情绪低落持续了一段时间，已经影响到上课或者睡眠，可以联系校心理咨询中心，或者找辅导员。那里有专业的人可以聊，而且这条路径本来就是学校配好的，不需要额外争取。",
        "这里不做任何判断，也不劝谁开始或结束一段关系。第五条的作用只有一个：让人知道还有这条路，而且这条路是现成的。",
    ]),
    ("PPT 第 22 页", "尾页 · 收束", "19:30–20:00", [
        "回到开头那个问题：时间、钱和期待都有限，一段关系怎么才能谈得清楚？",
        "我们的答案，不是少谈一点，也不是更努力一点，而是把恋爱放进课表。",
        ("QUOTE", "在恋爱里，我们常以为时间越长，代表感情越深。但今天我想说的是——就像圣埃克苏佩里说的：「爱不是互相凝视，而是一起注视同一个方向。」所以，我们不必去卷谁陪谁熬夜更久，而是要比拼：我们能不能一起把未来的日子，规划得更清晰、安排得更妥当。这，才是成年人最高级的浪漫。"),
        ("QUOTE", "不过，浪漫最终要落到地上。三毛说：「爱情如果不落到穿衣、吃饭、数钱、睡觉这些实实在在的生活中去，是不会长久的。」"),
        "屏幕上的这句话，也是我们这二十分钟最想留下的：不比谁陪得更久，比谁把日子安排得更清楚。",
        "谢谢大家。",
    ]),
]

_total = sum(len(re.findall(r"[\u4e00-\u9fff]", x[1] if isinstance(x, tuple) else x))
             for _, _, _, ps in SECTIONS for x in ps)
_m, _s = int(_total / 265), round((_total / 265 - int(_total / 265)) * 60)
SUB = f"演讲人四（D）｜对应 PPT 第 17–22 页｜全稿 {_total} 字，口播约 {_m} 分 {_s} 秒｜纯念稿"

NOTES = [
    f"时间：本段约 {_m} 分 {_s} 秒（比原稿多出的部分来自新加的两句引语）。若前面三位准点交棒，请在 15:00 接棒、19:30 前后进入尾页、20:00 收尾；若已超时，可先删第 18 页「为什么不一次改三条」的展开，或第 19 页最后一句「也有人根本不需要表」。",
    "口径：案例均为人名化名、情境合成，不说真实聊天记录；四步复盘是流程示意，不是心理干预方案；第五条只给校内资源路径，不做判断、不劝分劝合。",
    "新增两句：圣埃克苏佩里那句为流传较广的引语，若担心出处严谨，可说「有一句话常被归到圣埃克苏佩里名下」；三毛那句已出现在第 22 页版面上，念到「屏幕上的这句话」时抬手示意即可。",
    "交棒：从 C 接过「约定写好了，凭什么能撑过期末周？」；结束时与第 22 页金句同时收，不要拖。",
    "名字：第 9 页案例屏幕上写的是小红、小曦，原稿通篇用「同学甲／同学乙」。第 19 页开头已改成「前面案例里的那两位同学」，观众不会对不上；若要更贴合屏幕，可说成「就是刚才那两位同学，小红和小曦」。",
]


def set_font(run, cn="宋体", size=12, bold=False, color=None, italic=False):
    run.font.name = cn
    run.font.size = Pt(size)
    run.bold = bold
    run.italic = italic
    if color:
        run.font.color.rgb = RGBColor.from_string(color)
    rPr = run._element.get_or_add_rPr()
    rFonts = rPr.find(qn("w:rFonts"))
    if rFonts is None:
        rFonts = OxmlElement("w:rFonts")
        rPr.append(rFonts)
    rFonts.set(qn("w:ascii"), cn)
    rFonts.set(qn("w:hAnsi"), cn)
    rFonts.set(qn("w:eastAsia"), cn)
    rFonts.set(qn("w:cs"), cn)


def shade(p, fill):
    pPr = p._p.get_or_add_pPr()
    shd = OxmlElement("w:shd")
    shd.set(qn("w:val"), "clear")
    shd.set(qn("w:color"), "auto")
    shd.set(qn("w:fill"), fill)
    pPr.append(shd)


def borders(p, edges):
    """edges: dict side -> (sz_eighths, color)"""
    pPr = p._p.get_or_add_pPr()
    pBdr = OxmlElement("w:pBdr")
    for side in ("top", "left", "bottom", "right"):
        if side in edges:
            sz, color = edges[side]
            el = OxmlElement("w:" + side)
            el.set(qn("w:val"), "single")
            el.set(qn("w:sz"), str(sz))
            el.set(qn("w:space"), "4")
            el.set(qn("w:color"), color)
            pBdr.append(el)
    pPr.append(pBdr)


def para(doc, space_after=6, space_before=0, line=1.55, first_indent=None,
         left_indent=None, right_indent=None, align=None):
    p = doc.add_paragraph()
    pf = p.paragraph_format
    pf.space_after = Pt(space_after)
    pf.space_before = Pt(space_before)
    pf.line_spacing = line
    if first_indent is not None:
        pf.first_line_indent = Cm(first_indent)
    if left_indent is not None:
        pf.left_indent = Cm(left_indent)
    if right_indent is not None:
        pf.right_indent = Cm(right_indent)
    if align is not None:
        p.alignment = align
    return p


doc = Document()
sec = doc.sections[0]
sec.page_width, sec.page_height = Cm(21.0), Cm(29.7)
sec.left_margin = sec.right_margin = Cm(2.6)
sec.top_margin = Cm(2.2)
sec.bottom_margin = Cm(2.0)

normal = doc.styles["Normal"]
normal.font.name = "宋体"
normal.font.size = Pt(12)
normal.element.rPr.rFonts.set(qn("w:eastAsia"), "宋体")

# ---------- title ----------
p = para(doc, space_after=2, line=1.3, align=WD_ALIGN_PARAGRAPH.LEFT)
set_font(p.add_run(TITLE), cn="微软雅黑", size=20, bold=True, color="1F3864")

p = para(doc, space_after=10, line=1.3)
set_font(p.add_run(SUB), cn="微软雅黑", size=10, color="595959")

p = para(doc, space_after=12, line=1.0)
borders(p, {"bottom": (10, "1F3864")})

p = para(doc, space_after=4, line=1.4)
set_font(p.add_run("看稿说明"), cn="微软雅黑", size=11, bold=True, color="1F3864")
p = para(doc, space_after=16, line=1.5)
set_font(p.add_run("每一节的蓝色标题，就是你在那一页要说的话，看到标题就翻页，不用背页码。"
                   "灰色底纹段是两句引语，可放慢、可抬手示意屏幕。"), cn="宋体", size=10.5, color="404040")

# ---------- sections ----------
for i, (page, code, timer, paras) in enumerate(SECTIONS):
    p = para(doc, space_before=(0 if i == 0 else 16), space_after=8, line=1.2)
    p.paragraph_format.keep_with_next = True
    if page == "PPT 第 22 页":
        p.paragraph_format.page_break_before = True
    set_font(p.add_run(f"▍{page}"), cn="微软雅黑", size=14, bold=True, color="1F3864")
    set_font(p.add_run(f"　{code}"), cn="微软雅黑", size=10.5, bold=True, color="2E75B6")
    set_font(p.add_run(f"　·　{timer}"), cn="微软雅黑", size=10, color="808080")
    shade(p, "EAF1F8")
    borders(p, {"left": (18, "2E75B6")})

    for item in paras:
        if isinstance(item, tuple) and item[0] == "QUOTE":
            q = para(doc, space_before=4, space_after=8, line=1.6,
                     left_indent=0.7, right_indent=0.5)
            q.paragraph_format.keep_together = True
            set_font(q.add_run(item[1]), cn="楷体", size=11.5, color="333333")
            shade(q, "F2F2F2")
            borders(q, {"left": (12, "BFBFBF")})
        else:
            b = para(doc, space_after=6, line=1.6, first_indent=0.0)
            set_font(b.add_run(item), cn="宋体", size=12, color="1A1A1A")

# ---------- notes ----------
p = para(doc, space_before=18, space_after=8, line=1.2)
border = {"top": (8, "BFBFBF")}
borders(p, border)
set_font(p.add_run("备注（不念）"), cn="微软雅黑", size=10.5, bold=True, color="808080")
for n in NOTES:
    p = para(doc, space_after=4, line=1.45)
    p.paragraph_format.left_indent = Cm(0.65)
    p.paragraph_format.first_line_indent = Cm(-0.4)
    set_font(p.add_run("· " + n), cn="宋体", size=9.5, color="595959")

doc.save(OUT)

# ---------- plain text copy ----------
TXT = OUT.replace(".docx", ".txt")
lines = [TITLE, SUB, ""]
for page, code, timer, paras in SECTIONS:
    lines.append(f"【{page} · {code} · {timer}】")
    for item in paras:
        lines.append(item[1] if isinstance(item, tuple) else item)
    lines.append("")
lines.append("———— 备注（不念） ————")
lines += ["· " + n for n in NOTES]
open(TXT, "w", encoding="utf-8-sig").write("\n".join(lines) + "\n")
print("saved txt:", TXT)

# ---------- self check ----------
d2 = Document(OUT)
body = "".join(p.text for p in d2.paragraphs)
cjk = len(re.findall(r"[\u4e00-\u9fff]", body))
print("saved:", OUT)
print("paragraphs:", len(d2.paragraphs))
print("CJK chars total:", cjk)
per = []
for page, code, timer, paras in SECTIONS:
    t = "".join(x[1] if isinstance(x, tuple) else x for x in paras)
    per.append((page, len(re.findall(r"[\u4e00-\u9fff]", t))))
for page, c in per:
    print(f"  {page}: {c} 汉字  ≈ {c/265*60:.0f} 秒")
print("script-only CJK:", sum(c for _, c in per), "≈", f"{sum(c for _, c in per)/265:.1f} 分钟")
missing = [p for p in ["PPT 第 17 页", "PPT 第 18 页", "PPT 第 19 页", "PPT 第 20 页",
                       "PPT 第 21 页", "PPT 第 22 页", "圣埃克苏佩里", "三毛"] if p not in body]
print("missing markers:", missing)
