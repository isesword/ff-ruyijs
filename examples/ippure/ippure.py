"""IPPure（ippure.com/cloudflare）Cloudflare 环境评分：

第三方客观环境打分器 —— Cloudflare Bot Management 风险指数（0-100，经
cf.999831.xyz iframe 采集 + HMAC/AES 签名网关）、Turnstile 双模式
（NonInteractive / Managed widget）与 Google reCAPTCHA v3 评分（0-1）。
站点本体是 VitePress + Rocket Loader 变体脚本，全部经 ES module 管线执行。

python ippure.py --proxy http://127.0.0.1:7897 --output ippure_result.json
python ippure.py --direct

流程：navigate → VitePress 模块挂载 → IPSummary 自动出风险指数
→ 触发「开始校验」（reCAPTCHA v3 execute + 网关复核）
→ 等待 Turnstile 两张卡的 verdict（token / 前端异常码）→ 报告。

判定口径（与 truepeople/scrapingcourse 一致，全部取自页面自身状态）：
- risk_index           页面渲染的 风险指数 N/100（>50 高 / >10 中 / 低）
- recaptcha_score      Google 返回的 0-1 分及文案带（≥0.8 极度安全 …）
- turnstile            每张卡的 当前状态/服务端验证 两行（600010 系已知
                        挑战面差距，见 docs/scrapingcourse-turnstile-600010
                        -investigation.md；本示例如实记录，不据此判 pass）

⚠️ 出口 IP 信誉参与全部三项评分：被循环质询先换 --proxy 出口再怀疑环境。
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

URL = "https://ippure.com/cloudflare"
MOUNTED = "!!document.querySelector('.turnstile-tester')"
RISK_BADGE = "(document.querySelector('.cf-score-badge')||{}).textContent"
TURNSTILE_READY = "typeof window.turnstile==='object'"
STATUS_ROWS = ("Array.from(document.querySelectorAll('.status-row'),"
               "function(r){return r.textContent.trim().replace(/\\s+/g,' ').slice(0,90)})")


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


def read_page(context):
    """页面自身状态的快照：三项评分各取所需，不注入任何探针。"""
    return context.eval("""({
      url: location.href, title: document.title, ready: document.readyState,
      risk_badge: (document.querySelector('.cf-score-badge')||{}).textContent || null,
      ip: (document.querySelector('.cf-ip-value')||{}).textContent || null,
      geo: (document.querySelector('.cf-country-name')||{}).textContent || null,
      cards: Array.from(document.querySelectorAll('.mode-card'), function(card){
        return {
          badge: (card.querySelector('.mode-badge')||{textContent:''}).textContent.trim(),
          rows: Array.from(card.querySelectorAll('.status-row'),
                function(r){ return r.textContent.trim().replace(/\\s+/g,' ').slice(0,90); }),
          banner: (card.querySelector('.banner-content')||{textContent:''}).textContent.trim().slice(0,160),
          errors: (card.querySelector('.error-list')||{textContent:''}).textContent.trim().slice(0,200),
          token_length: (card.querySelector('[name=cf-turnstile-response]')||{value:''}).value.length
        };
      }),
      recaptcha_score: (function(){
        var box = document.querySelector('.score-dashboard'); if (!box) return null;
        var text = box.textContent.replace(/\\s+/g, ' ');
        var score = text.match(/([0-9.]+)\\/\\s*1\\.0/);
        var verdict = text.match(/[\\u2192\\u2705\\u26a0\\ufe0f\\ud83d\\udeab]([^]*?)$/);
        return { score: score ? parseFloat(score[1]) : null, text: text.slice(0, 160) };
      })(),
      challenge_error: (function(){
        var m = document.body.textContent.match(/\\b(600010|600040|300030|110200|102500)\\b/);
        return m ? m[1] : null;
      })()
    })""")


def parse_risk(badge):
    if not badge:
        return None
    match = re.search(r"(\d+)\s*/\s*100", badge)
    return int(match.group(1)) if match else None


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
            context.advance(100, max_tasks=300)
        except JSError as error:
            record = error_record(stage, error)
            if record not in report["script_errors"] and len(report["script_errors"]) < 20:
                report["script_errors"].append(record)
        state = read_page(context)
        report["final_state"] = state
        if predicate(state):
            emit(report, started, stage, state=state)
            return state
        if monotonic() >= next_log:
            emit(report, started, stage, state=state)
            next_log = monotonic() + 15
    raise TimeoutError(f"{stage}: 条件未在 {timeout:g} 秒内成立")


def run(args, report, started):
    with Context(shell=args.shell, proxy=report["proxy"], timeout=args.rpc_timeout) as context:
        report["shell"] = str(context.shell)
        report["shell_sha256"] = digest(context.shell)
        context.configure_clock("system")
        # 与真 Firefox 159/mac 一致的环境偏好（IPPure 暗色模式探测项）。
        context.configure_fingerprint({"mediaFeatures": {"prefersColorScheme": "dark"}})

        report["stage"] = "load_page"
        context.navigate(URL)
        wait_for(context, report, started, report["stage"], args.timeout, lambda s: bool(s["cards"]))
        emit(report, started, "module_mount", title=report["final_state"]["title"])

        report["stage"] = "risk_index"
        wait_for(context, report, started, report["stage"], args.timeout,
                 lambda s: bool(parse_risk(s.get("risk_badge") or "")))
        state = report["final_state"]
        report["risk_index"] = parse_risk(state["risk_badge"])
        report["risk_band"] = ("high" if report["risk_index"] > 50 else
                               "medium" if report["risk_index"] > 10 else "low")
        report["exit_ip"] = state.get("ip")
        report["geo_country"] = state.get("geo")
        emit(report, started, "risk_index", risk_index=report["risk_index"],
             band=report["risk_band"], ip=report["exit_ip"])

        report["stage"] = "turnstile_ready"
        wait_for(context, report, started, report["stage"], args.timeout,
                 lambda s: any(card["badge"] in ("NonInteractive", "Managed") for card in s["cards"]))
        emit(report, started, "turnstile_ready")

        report["stage"] = "recaptcha_v3"
        start_button = ("(function(){var b=Array.from(document.querySelectorAll('button'))"
                        ".find(function(x){return x.textContent.trim()==='开始校验'&&x.disabled===false});"
                        "if(b){b.click();return true;}return false;})()")
        if context.eval(start_button):
            emit(report, started, "recaptcha_started")
        wait_for(context, report, started, report["stage"], args.timeout,
                 lambda s: bool(s.get("recaptcha_score") and s["recaptcha_score"]["score"] is not None))
        score = report["final_state"]["recaptcha_score"]
        report["recaptcha_score"] = score["score"]
        report["recaptcha_text"] = score["text"]
        emit(report, started, "recaptcha_score", score=score["score"], text=score["text"][:120])

        report["stage"] = "turnstile_verdict"
        # 出 token、异常态或超时为止：widget 需要时间走完 fo 握手 + 挑战。
        wait_for(context, report, started, report["stage"], args.turnstile_timeout,
                 lambda s: any(card["token_length"] > 0 or "前端异常" in r or "Verified" in r
                               for card in s["cards"] for r in card["rows"]))
        state = report["final_state"]
        report["turnstile_cards"] = [
            {"badge": card["badge"], "rows": card["rows"], "banner": card["banner"],
             "token_length": card["token_length"], "errors": card["errors"]}
            for card in state["cards"] if card["badge"]]
        report["challenge_error"] = state.get("challenge_error")
        emit(report, started, "turnstile_verdict", cards=report["turnstile_cards"],
             challenge_error=report["challenge_error"])

        # 判定：三项客观分里，风险指数与 reCAPTCHA 是可量化的环境分；Turnstile
        # 的 600010 系已知挑战面差距（见模块 docstring），出现时不算 pass。
        tokens = [card["token_length"] for card in state["cards"] if card["badge"]]
        report["passed"] = (report["risk_index"] is not None
                            and report["recaptcha_score"] is not None
                            and report["recaptcha_score"] >= 0.5
                            and any(t > 0 for t in tokens)
                            and report["challenge_error"] is None)
        report["stage"] = "done"


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    proxy = parser.add_mutually_exclusive_group()
    proxy.add_argument("--proxy", help="HTTP(S) 代理 URL（默认 env，再探测本地端口，最后直连）")
    proxy.add_argument("--direct", action="store_true", help="不使用代理")
    parser.add_argument("--shell", type=Path, help="runtime 路径；源码目录默认 dist/js")
    parser.add_argument("--url", default=URL, help="IPPure 检测页 URL")
    parser.add_argument("--timeout", type=float, default=120, help="每个等待阶段的期限，秒")
    parser.add_argument("--turnstile-timeout", type=float, default=90,
                        help="Turnstile verdict 的额外期限，秒")
    parser.add_argument("--rpc-timeout", type=float, default=60, help="单次 runtime 调用期限，秒")
    parser.add_argument("--output", type=Path, default=Path("ippure_result.json"),
                        help="JSON 报告路径")
    args = parser.parse_args()
    for name in ("timeout", "rpc_timeout", "turnstile_timeout"):
        if not math.isfinite(getattr(args, name)) or getattr(args, name) <= 0:
            parser.error(f"--{name.replace('_', '-')} must be a positive finite number")
    if args.shell is None and not os.environ.get("RUYIJS_SHELL"):
        default_shell = ROOT / "dist" / ("js.exe" if os.name == "nt" else "js")
        if default_shell.is_file():
            args.shell = default_shell
    args.output = args.output.resolve()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    report = {
        "url": args.url, "started_at": datetime.now().astimezone().isoformat(),
        "proxy": None if args.direct else (args.proxy or auto_proxy()),
        "clock": "system", "environment": "defaults", "passed": False, "stage": "startup",
        "risk_index": None, "recaptcha_score": None, "turnstile_cards": [],
        "script_errors": [], "steps": [],
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
                      "risk_index": report["risk_index"],
                      "recaptcha_score": report["recaptcha_score"],
                      "elapsed_seconds": report["elapsed_seconds"], "output": str(args.output)},
                     ensure_ascii=True))
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    sys.exit(main())
