import re
import httpx

html = httpx.get(
    "https://open.oceanbase.com/train/TestRecord?questionId=600004",
    headers={"User-Agent": "Mozilla/5.0"},
    timeout=30,
).text
scripts = re.findall(r'src="([^"]+\.js)"', html)
print("scripts", len(scripts))
for s in scripts[:20]:
    print(s)
