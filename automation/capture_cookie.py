"""一次性抓取 Cookie：会打开浏览器，请手动登录后按 Enter 保存。"""
import asyncio
from pathlib import Path
from playwright.async_api import async_playwright

COOKIE_FILE = Path(__file__).parent / ".oceanbase_cookie"
LOGIN = "https://www.oceanbase.com/ob/login/mobile?goto=https%3A%2F%2Fopen.oceanbase.com%2Ftrain"
TRAIN = "https://open.oceanbase.com/train/TestRecord?questionId=600004"


async def main():
    async with async_playwright() as p:
        browser = await p.chromium.launch(headless=False)
        context = await browser.new_context(locale="zh-CN")
        page = await context.new_page()
        await page.goto(LOGIN)
        print("请在浏览器中完成登录（含验证码），登录成功后回到此窗口按 Enter…")
        input()
        await page.goto(TRAIN, wait_until="networkidle", timeout=120000)
        cookies = await context.cookies()
        cookie_str = "; ".join(f"{c['name']}={c['value']}" for c in cookies)
        COOKIE_FILE.write_text(cookie_str, encoding="utf-8")
        print(f"已保存到 {COOKIE_FILE}（{len(cookie_str)} 字符）")
        await browser.close()


if __name__ == "__main__":
    asyncio.run(main())
