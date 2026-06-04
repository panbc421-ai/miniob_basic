import re
from pathlib import Path

js = Path("_train.js").read_text(encoding="utf-8")
for m in re.finditer(r"testplatForm/[a-zA-Z]+", js):
    print(m.group())
for m in re.finditer(r'type:"[^"]+"', js):
    s = m.group()
    if "test" in s.lower() or "commit" in s.lower() or "plat" in s.lower():
        print(s)
