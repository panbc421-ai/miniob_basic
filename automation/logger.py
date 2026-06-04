"""
logger.py — 运行日志工具
"""
import json
import time
from pathlib import Path

LOG_DIR = Path("logs")


class AutoPilotLogger:

    def __init__(self):
        LOG_DIR.mkdir(exist_ok=True)

    def _append(self, filename: str, entry: dict):
        with open(LOG_DIR / filename, "a") as f:
            f.write(json.dumps(entry, ensure_ascii=False) + "\n")

    def log_submission(self, question_name: str, commit_id: str, result: dict):
        self._append("submissions.jsonl", {
            "timestamp": time.strftime("%Y-%m-%dT%H:%M:%SZ"),
            "question": question_name,
            "commit_id": commit_id,
            "result": result,
            "cooldown_until": time.time() + 600,
        })

    def log_development(self, question_name: str, modified_files: list[str]):
        self._append("development.jsonl", {
            "timestamp": time.strftime("%Y-%m-%dT%H:%M:%SZ"),
            "question": question_name,
            "modified_files": modified_files,
        })
