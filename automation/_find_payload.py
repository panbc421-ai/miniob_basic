import re
import httpx

js = httpx.get(
    "https://gw.alipayobjects.com/render/p/yuyan/180020010001011015/umi.5b445798.js",
    timeout=60,
).text
# chunks that might contain submit form
for name in re.findall(r'"(\d+)":"(p__[^"]+)"', js):
    if "Train" in name[1] or "test" in name[1].lower() or "Test" in name[1]:
        print(name)

# search dispatch commit
for m in re.finditer(r"commitTestPlatformTaskRecord", js):
    start = m.start()
    # look backwards for payload object
    chunk = js[max(0, start - 2000) : start + 500]
    if "repo" in chunk or "git" in chunk:
        print("---")
        print(chunk[-800:])
