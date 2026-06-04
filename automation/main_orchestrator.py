"""
main_orchestrator.py — MiniOB AutoPilot
提交部分由人工完成，Agent 负责写代码和分析结果
"""
import asyncio
import time
import json
import subprocess
from pathlib import Path
from analyzer import ResultAnalyzer

PROGRESS_FILE = Path(".progress.json")
COOLDOWN = 600


def load_progress() -> dict:
    return json.loads(PROGRESS_FILE.read_text())


def save_progress(progress: dict):
    PROGRESS_FILE.write_text(json.dumps(progress, ensure_ascii=False, indent=2))


def get_next_question(progress: dict):
    for q in progress["questions"]:
        if q["status"] != "passed":
            return q
    return None


def seconds_until_next_submit(progress: dict) -> float:
    last = progress["submission"].get("last_submit_time")
    if not last:
        return 0
    return max(0, COOLDOWN - (time.time() - last))


def git_commit_push(message: str) -> str:
    subprocess.run(["git", "add", "."])
    subprocess.run(["git", "commit", "-m", message])
    subprocess.run(["git", "push"])
    result = subprocess.run(["git", "rev-parse", "HEAD"], capture_output=True, text=True)
    return result.stdout.strip()


def print_banner(q: dict, progress: dict):
    completed = sum(1 for x in progress["questions"] if x["status"] == "passed")
    print(f"\n{'='*50}")
    print(f"📊 进度: {completed}/25 题已通过")
    print(f"📝 当前: [{q['id']}] {q['name']}")
    print(f"{'='*50}")


async def main():
    print("🚀 MiniOB AutoPilot 启动（手动提交模式）")
    analyzer = ResultAnalyzer()

    while True:
        progress = load_progress()

        # 冷却检查
        wait = seconds_until_next_submit(progress)
        if wait > 0:
            print(f"⏳ 距离下次可提交还需等待 {wait:.0f} 秒...")
            await asyncio.sleep(min(wait, 30))
            continue

        # 获取下一题
        q = get_next_question(progress)
        if q is None:
            print("🎉 全部 25 道题目已完成！")
            break

        print_banner(q, progress)

        # 等待代码开发完成
        print(f"\n💻 请完成题目 [{q['id']}] {q['name']} 的代码开发")
        print("   开发完成后按 Enter 继续...")
        input()

        # Git 提交推送
        commit_msg = f"feat({q['name']}): 完成第{q['id']}题"
        print(f"📤 正在 git commit & push...")
        commit_id = git_commit_push(commit_msg)
        print(f"✅ 推送成功，commit: {commit_id[:8]}")

        # 提示手动提交
        print(f"""
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
🌐 请手动提交到训练营：
   网址：https://open.oceanbase.com/train/detail/5?questionId=600004
   仓库：{progress['git']['repo']}
   分支：main
   Commit ID：{commit_id}
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
提交成功后按 Enter 开始等待结果...
        """)
        input()

        # 记录提交时间（触发冷却）
        progress["submission"]["last_submit_time"] = time.time()
        progress["submission"]["total_attempts"] += 1
        progress["submission"]["last_commit_id"] = commit_id
        q["status"] = "submitted"
        save_progress(progress)

        # 等待测试完成
        print("⏳ 等待 3 分钟让测试跑完...")
        for i in range(18):
            await asyncio.sleep(10)
            print(f"   已等待 {(i+1)*10} 秒...")

        # 询问结果
        print("""
测试结果出来了吗？请输入：
  1 = 通过了 ✅
  2 = 失败了 ❌（需要粘贴失败信息）
  3 = 还在跑，继续等
        """)
        choice = input("请输入 1/2/3: ").strip()

        if choice == "1":
            q["status"] = "passed"
            q["passed_time"] = time.strftime("%Y-%m-%dT%H:%M:%SZ")
            save_progress(progress)
            print(f"🎉 {q['name']} 通过！继续下一题...")

        elif choice == "2":
            error_info = input("粘贴失败信息（可选，直接回车跳过）: ").strip()
            if error_info:
                analysis = analyzer.analyze(error_info)
                print("\n🔍 分析结果：")
                for s in analysis["fix_suggestions"]:
                    print(f"   • {s}")
            q["status"] = "pending"
            q["retry_count"] = q.get("retry_count", 0) + 1
            save_progress(progress)
            print(f"⏳ 等待冷却后重试...")
            await asyncio.sleep(COOLDOWN)

        elif choice == "3":
            print("继续等待...")
            await asyncio.sleep(60)

        await asyncio.sleep(2)


if __name__ == "__main__":
    asyncio.run(main())
