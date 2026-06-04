import re
import httpx

url = "https://gw.alipayobjects.com/render/p/yuyan/180020010001011015/umi.5b445798.js"
js = httpx.get(url, timeout=60).text
for kw in [
    "queryTestPlatformConfigByConfigKey",
    "TestButton",
    "testButton",
    "commitTest",
    "提测",
]:
    if kw in js:
        print("found", kw)
        m = re.search(re.escape(kw), js)
        print(js[m.start() - 80 : m.start() + 400])
