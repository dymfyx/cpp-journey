# -*- coding: utf-8 -*-
"""Dump text from the source DOCX script and PPTX deck."""
import sys, io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding="utf-8", errors="replace")

DOCX = r"C:\Users\dym54\Documents\xwechat_files\wxid_ksdgcdrq1vb522_b4a3\msg\file\2026-11\大学生恋爱_四人念稿版(3).docx"
PPTX = r"C:\Users\dym54\Documents\xwechat_files\wxid_ksdgcdrq1vb522_b4a3\msg\file\2026-11\大学生恋爱_方案一_PPT(7).pptx"

print("=" * 30, "DOCX", "=" * 30)
from docx import Document
d = Document(DOCX)
for i, p in enumerate(d.paragraphs):
    t = p.text.strip()
    print(f"[P{i:03d}] style={p.style.name!r} :: {t}")

print()
print("=" * 30, "PPTX", "=" * 30)
from pptx import Presentation
pr = Presentation(PPTX)
for idx, slide in enumerate(pr.slides, start=1):
    print(f"\n--- slide {idx} (layout={slide.slide_layout.name}) ---")
    def walk(shapes, depth=0):
        for sh in shapes:
            pad = "  " * depth
            kind = sh.shape_type
            try:
                if sh.has_text_frame and sh.text_frame.text.strip():
                    txt = sh.text_frame.text.replace("\n", " | ")
                    print(f"{pad}<{sh.name}> [{kind}] {txt}")
            except Exception:
                pass
            if sh.has_table:
                for r in sh.table.rows:
                    print(f"{pad}  ROW: " + " || ".join(c.text for c in r.cells))
            if sh.shape_type == 6:  # group
                walk(sh.shapes, depth + 1)
    walk(slide.shapes)
    if slide.has_notes_slide:
        nt = slide.notes_slide.notes_text_frame.text.strip() if slide.notes_slide.notes_text_frame else ""
        if nt:
            print(f"  NOTES: {nt}")
