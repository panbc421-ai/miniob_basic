import re
import httpx

js = httpx.get(
    "https://gw.alipayobjects.com/render/p/yuyan/180020010001011015/umi.5b445798.js",
    timeout=60,
).text
# module export Or
for pat in [
    r'Or\s*=\s*"([^"]+)"',
    r"TECH_CAMP\s*=\s*'([^']+)'",
    r"openwebapi\.oceanbase\.com[^'\"]{0,40}",
]:
    ms = re.findall(pat, js)
    if ms:
        print(pat, list(dict.fromkeys(ms))[:8])

idx = js.find("TECH_CAMP")
print("TECH_CAMP ctx:", js[idx : idx + 200] if idx >= 0 else "none")
