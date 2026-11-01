"""ScrapingCourse（Cloudflare Turnstile 登录演示）：
ruyijs 原生环境等待本次 Turnstile token，提交演示账号并复核 dashboard 会话。

移植自作者公众号示例（2026-11-01，Windows inner 分支 runtime 实测通过）；
本文件与 examples/truepeople 同一验收风格，判定取自 net_log 与页面自身状态。

python scrapingcourse.py --proxy http://127.0.0.1:7897 --output scrapingcourse_result.json
python scrapingcourse.py --direct

流程：navigate 登录页 → advance 循环等表单 → type 演示账号（60ms/字符）
→ 等隐藏字段 cf-turnstile-response 出现 token → click 提交
→ POST 302/303 → dashboard 200 → 再次导航且新请求 ID 200 → passed。

⚠️ Turnstile 也看出口 IP 信誉：循环失败先换 --proxy 出口（truepeople 的
边缘冷却结论在此同样适用）。
"""

import argparse
from datetime import datetime
import hashlib
import json
import math
import os
from pathlib import Path
import re
import socket
import sys
from time import monotonic

ROOT = Path(__file__).resolve().parents[2]
sys.dont_write_bytecode = True
if (ROOT / "dist" / ("js.exe" if os.name == "nt" else "js")).is_file() or os.environ.get("RUYIJS_SHELL"):
    sys.path.insert(0, str(ROOT / "python"))
from ruyijs import Context, JSError

LOGIN = "https://www.scrapingcourse.com/login/cf-turnstile"
DASHBOARD = "https://www.scrapingcourse.com/dashboard"
STATE = """({
  url: location.href, title: document.title, ready: document.readyState,
  token_length: (document.querySelector('[name="cf-turnstile-response"]')?.value || '').length,
  email_ready: !!document.querySelector('#email'),
  password_ready: !!document.querySelector('#password'),
  products: Array.from(document.querySelectorAll('.product-name'), n => n.textContent.trim()),
  page_messages: Array.from(document.querySelectorAll('.invalid-feedback,[role="alert"],.alert-danger'),
                           n => n.textContent.trim())
})"""


def auto_proxy():
    for name in ("HTTPS_PROXY", "https_proxy", "HTTP_PROXY", "http_proxy"):
        if os.environ.get(name):
            return os.environ[name]
    for port in (7897, 7890):
        try:
            with socket.create_connection(("127.0.0.1", port), timeout=.3):
                return f"http://127.0.0.1:{port}"
        except OSError:
            continue
    return None


def digest(path):
    result = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            result.update(chunk)
    return result.hexdigest()


def is_dashboard(state):
    return (state["url"] == DASHBOARD
            and state["title"] == "Success Page - ScrapingCourse.com"
            and bool(state["products"]))


def dashboard_requests(network):
    return [r for r in network if r["method"] == "GET"
            and r["url"] == DASHBOARD and r["status"] == 200]


def login_requests(network):
    return [r for r in network if r["method"] == "POST" and r["url"] == LOGIN]


def error_record(stage, error):
    record = {"stage": stage, "type": type(error).__name__, "message": str(error)}
    if isinstance(error, JSError):
        record["stack"] = error.stack
    return record


def emit(report, started, stage, **fields):
    row = {"stage": stage, "elapsed_seconds": round(monotonic() - started, 3), **fields}
    report["steps"].append(row)
    print(json.dumps(row, ensure_ascii=True), flush=True)


def wait_for(context, report, started, stage, timeout, predicate):
    deadline = monotonic() + timeout
    next_log = 0
    while monotonic() < deadline:
        try:
            context.advance(100, max_tasks=100)
        except JSError as error:
            record = error_record(stage, error)
            if record not in report["script_errors"]:
                report["script_errors"].append(record)
            if re.search(r"\[Cloudflare Turnstile\].*\b(?:600010|600040|300030)\b", str(error)):
                raise
        state = context.eval(STATE)
        report["final_state"] = state
        if predicate(state):
            emit(report, started, stage, state=state)
            return state
        if monotonic() >= next_log:
            emit(report, started, stage, state=state)
            next_log = monotonic() + 10
    raise TimeoutError(f"{stage}: no accepted result within {timeout:g} seconds")


def run(args, report, started):
    with Context(shell=args.shell, proxy=report["proxy"], timeout=args.rpc_timeout) as context:
        report["shell"] = str(context.shell)
        report["shell_sha256"] = digest(context.shell)
        context.configure_clock("system")
        try:
            report["stage"] = "load_login"
            context.navigate(LOGIN)
            wait_for(context, report, started, report["stage"], args.timeout,
                     lambda s: s["email_ready"] and s["password_ready"])

            report["stage"] = "fill_demo_credentials"
            context.type("admin@example.com", target="document.querySelector('#email')", interval_ms=60)
            context.type("password", target="document.querySelector('#password')", interval_ms=60)
            report["checks"]["credentials_entered"] = True

            report["stage"] = "wait_turnstile"
            state = wait_for(context, report, started, report["stage"], args.timeout,
                             lambda s: s["token_length"] > 0)
            report["token_length"] = state["token_length"]
            report["checks"]["token_generated"] = True

            report["stage"] = "submit_login"
            context.click("document.querySelector('#submit-button')")
            report["checks"]["submitted"] = True
            state = wait_for(context, report, started, report["stage"], args.timeout,
                             lambda s: is_dashboard(s) and s["ready"] == "complete"
                             and bool(dashboard_requests(context.net_log())))
            network = context.net_log()
            report["checks"]["login_accepted"] = any(
                r["status"] in (302, 303) for r in login_requests(network))
            report["first_dashboard"] = state

            # Require a new completed network record; a stale DOM after navigate is insufficient.
            previous_ids = {r["id"] for r in dashboard_requests(network)}
            report["stage"] = "refetch_dashboard"
            context.navigate(DASHBOARD)
            state = wait_for(context, report, started, report["stage"], args.timeout,
                             lambda s: is_dashboard(s) and s["ready"] == "complete" and any(
                                 r["id"] not in previous_ids for r in dashboard_requests(context.net_log())))
            report["checks"]["fresh_dashboard_verified"] = True
            report["products"] = state["products"]
            report["passed"] = all(report["checks"].values())
            report["stage"] = "passed" if report["passed"] else "acceptance_failed"
        finally:
            if not context.closed:
                for name, getter in (("network", context.net_log), ("console", context.console_log)):
                    try:
                        rows = getter()
                        if name == "network":
                            report[name] = [{k: r.get(k) for k in ("id", "method", "url", "status", "error")}
                                            for r in rows]
                        else:
                            report[name] = rows
                    except Exception as error:
                        report["evidence_errors"].append(error_record(name, error))
            report["shell_sha256_after"] = digest(context.shell)
            report["runtime_unchanged"] = report["shell_sha256"] == report["shell_sha256_after"]
            report["passed"] = report["passed"] and report["runtime_unchanged"]


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    proxy = parser.add_mutually_exclusive_group()
    proxy.add_argument("--proxy", help="HTTP(S) 代理 URL（默认 env，再探测本地端口，最后直连）")
    proxy.add_argument("--direct", action="store_true", help="不使用代理")
    parser.add_argument("--shell", type=Path, help="runtime 路径；源码目录默认 dist/js")
    parser.add_argument("--timeout", type=float, default=120, help="每个等待阶段的期限，秒")
    parser.add_argument("--rpc-timeout", type=float, default=60, help="单次 runtime 调用期限，秒")
    parser.add_argument("--output", type=Path, default=Path("scrapingcourse_result.json"),
                        help="JSON 报告路径")
    args = parser.parse_args()
    for name in ("timeout", "rpc_timeout"):
        if not math.isfinite(getattr(args, name)) or getattr(args, name) <= 0:
            parser.error(f"--{name.replace('_', '-')} must be a positive finite number")
    if args.shell is None and not os.environ.get("RUYIJS_SHELL"):
        default_shell = ROOT / "dist" / ("js.exe" if os.name == "nt" else "js")
        if default_shell.is_file():
            args.shell = default_shell
    args.output = args.output.resolve()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    report = {
        "url": LOGIN, "started_at": datetime.now().astimezone().isoformat(),
        "proxy": None if args.direct else (args.proxy or auto_proxy()),
        "clock": "system", "environment": "defaults", "passed": False, "stage": "startup",
        "checks": dict.fromkeys(("credentials_entered", "token_generated", "submitted",
                                 "login_accepted", "fresh_dashboard_verified"), False),
        "script_errors": [], "evidence_errors": [], "steps": [],
    }
    started = monotonic()
    try:
        run(args, report, started)
    except Exception as error:
        report["passed"] = False
        report["failure"] = error_record(report["stage"], error)
    report["elapsed_seconds"] = round(monotonic() - started, 3)
    args.output.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", "utf-8")
    print(json.dumps({"passed": report["passed"], "stage": report["stage"],
                      "elapsed_seconds": report["elapsed_seconds"], "output": str(args.output)},
                     ensure_ascii=True))
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    sys.exit(main())
