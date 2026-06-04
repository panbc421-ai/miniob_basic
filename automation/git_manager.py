"""
git_manager.py — Git 仓库操作封装
"""
import subprocess
from pathlib import Path


class GitManager:

    def __init__(self, repo_path: str = "."):
        self.repo_path = Path(repo_path)

    def _run(self, *args, check=True) -> subprocess.CompletedProcess:
        return subprocess.run(
            list(args),
            cwd=self.repo_path,
            capture_output=True,
            text=True,
            check=check,
        )

    def get_current_commit(self) -> str:
        return self._run("git", "rev-parse", "HEAD").stdout.strip()

    def commit_and_push(self, message: str):
        self._run("git", "add", ".")
        result = self._run("git", "commit", "-m", message, check=False)
        if result.returncode != 0 and "nothing to commit" not in result.stdout:
            print(f"⚠️  git commit 警告：{result.stderr}")
        # 推送，冲突时先 rebase
        push = self._run("git", "push", check=False)
        if push.returncode != 0:
            print("⚠️  push 失败，尝试 rebase...")
            self._run("git", "pull", "--rebase")
            self._run("git", "push")

    def get_commit_history(self, n: int = 5) -> str:
        return self._run("git", "log", f"-{n}", "--oneline").stdout
