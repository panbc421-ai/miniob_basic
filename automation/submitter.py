"""
submitter.py — OceanBase 训练营自动提交与结果读取
接口已通过抓包确认，无需浏览器
"""
import time
import requests
import json
import subprocess

# ============ 配置区 ============
REPO_URL  = "https://github.com/neellygainforthuj330-bot/miniob_basic"
BRANCH    = "main"
QUESTION_ID = 600004
CAMP_ID     = 5

COOKIE = "_ga=GA1.1.1853480079.1779329818; ob_session_30_d1ed32b2-d338-4bc6-8de3-70056270f7d8=d1ed32b2-d338-4bc6-8de3-70056270f7d8.1779851532707.1779853338063; ofx_session_key=39b390e2-b322-48e4-9447-463a3a44a24a_v1; ofx_utm_first_item={\"platform\":\"referrer\",\"referrer\":\"https://cn.bing.com/\",\"priority\":1,\"utm_source\":\"https://cn.bing.com/\",\"storedTime\":1779864807693}; ofx_utm_last_item={\"platform\":\"referrer\",\"referrer\":\"https://cn.bing.com/\",\"priority\":1,\"utm_source\":\"https://cn.bing.com/\",\"storedTime\":1779864807694}; Hm_lvt_c6b821bb340e61f0d507b9bef3bd4d5b=1780540645,1780544648,1780547514,1780550217; HMACCOUNT=A08AF6D9D610C692; inviteCode=; numberRegisterFlag=; emailRegisterFlag=; orgId=; linksUserId=null; canUseLinksWorkOrder=false; canUseWorkOrder=false; isLinksWorkOrderAdmin=false; linksWorkOrderEntityId=null; downloadCenterWhiteUser=false; RDS-APP-ID=obiamweb_pwdLogin; RDS-TOKEN=db5563fbb3f1b5d863d6937389927b716d88cc28378eed92a9f47373c2e69516; RDS-BIZ-NO=N3pcMDF7bZkEfY8RtA83JWTjtptiKsRbper3nwNbwN8kQw8E2Me2Hy5F6DPdNKXB; preOrderPageData=; ob_session_30_39b390e2-b322-48e4-9447-463a3a44a24a=39b390e2-b322-48e4-9447-463a3a44a24a.1780558636545.1780561485448; spanner=uRjlhvubpAw53xyJwTlk9f1VdeJegJOs; Hm_lpvt_c6b821bb340e61f0d507b9bef3bd4d5b=1780560463; _ga_T35KTM57DZ=GS2.1.s1780558454$o61$g1$t1780560462$j60$l0$h0"

BASE = "https://openwebapi.oceanbase.com/api/techcamp/testplatform"

HEADERS = {
    "Cookie": COOKIE,
    "Content-Type": "application/json",
    "Referer": "https://open.oceanbase.com/",
    "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36",
    "Origin": "https://open.oceanbase.com",
}
# ================================


def get_current_commit() -> str:
    """获取当前 git commit id"""
    result = subprocess.run(
        ["git", "rev-parse", "HEAD"],
        capture_output=True, text=True
    )
    return result.stdout.strip()


class OceanBaseSubmitter:

    def submit(self, branch: str = BRANCH) -> float:
        """提交代码到训练营"""
        commit_id = get_current_commit()
        print(f"📤 提交中... commit: {commit_id[:8]}")

        payload = {
            "questionId": QUESTION_ID,
            "repoUrl": REPO_URL,
            "commitId": commit_id,
            "branch": branch,
        }

        resp = requests.post(
            f"{BASE}/taskrecord/commitTestPlatformTaskRecord",
            headers=HEADERS,
            json=payload,
            timeout=30,
        )
        print(f"提交状态: {resp.status_code}")
        data = resp.json()
        print(f"提交响应: {json.dumps(data, ensure_ascii=False)[:300]}")

        if data.get("code") == 200:
            print("✅ 提交成功！")
        else:
            print(f"❌ 提交失败: {data.get('message')}")

        return time.time()

    def get_test_result(self) -> dict:
        """读取最近一次测试结果"""
        resp = requests.get(
            f"{BASE}/taskrecord/queryTestPlatformTaskRecordList",
            headers=HEADERS,
            params={"questionId": QUESTION_ID},
            timeout=30,
        )
        data = resp.json()
        print(f"结果原始数据: {json.dumps(data, ensure_ascii=False)[:1000]}")
        return self._parse_result(data)

    def _parse_result(self, data: dict) -> dict:
        """解析测试结果"""
        try:
            records = data.get("data", {}).get("list", [])
            if not records:
                return {"total_score": 0, "details": {}, "status": "no_record"}

            latest = records[0]
            score = latest.get("score", 0)
            status = latest.get("status", "")  # PASS / FAIL / RUNNING
            details_raw = latest.get("questionScoreList", [])

            details = {}
            for item in details_raw:
                name = item.get("questionName", "")
                s = item.get("status", "")
                details[name] = s

            print(f"📊 得分: {score}, 状态: {status}")
            return {
                "total_score": score,
                "status": status,
                "details": details,
            }
        except Exception as e:
            print(f"解析失败: {e}")
            return {"total_score": 0, "details": {}}


if __name__ == "__main__":
    s = OceanBaseSubmitter()
    print("=== 测试读取结果 ===")
    result = s.get_test_result()
    print(f"\n最终结果: {result}")
