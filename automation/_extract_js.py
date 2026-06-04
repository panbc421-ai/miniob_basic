import re
import httpx
from pathlib import Path

url = "https://gw.alipayobjects.com/render/p/yuyan/180020010001011015/p__Train__TestRecord.aa64b8f9.async.js"
js = httpx.get(url, timeout=30).text
Path("_train.js").write_text(js, encoding="utf-8")

# all /api/... fragments
apis = set(re.findall(r"/api/[a-zA-Z0-9_/]+", js))
for s in sorted(apis):
    print(s)

print("\n--- openwebapi context ---")
for m in re.finditer(r".{0,30}openwebapi.{0,80}", js):
    print(m.group()[:120])
