# ruyijs补环境一键过cloudflare/5s盾turnstile

> 来源：微信公众号「如意」 https://mp.weixin.qq.com/s/RxyspP-XHsj0_u1zXNWvWA
> 抓取日期：2026-11-01（web_reader 正文，格式照录，仅调整排版）

ruyijs一键过DataDome完整代码

ruyijs内置火狐60个版本真实TLS指纹——随意玩akamai

## 本项目基于开源项目ff-ruyijs完成（二次编译分发必须开源，禁止任何形式闭源收费，请诸位自觉遵守）：

```
项目开源地址：https://github.com/cutetailbud/ff-ruyijs
和如意用同款大模型？使用 www.fastaitoken.com 站点的大模型完成，优惠码RUYIJS。
```

## ScrapingCourse：Turnstile 一键过代码

使用 ruyijs 打开 `https://www.scrapingcourse.com/login/cf-turnstile`，执行网站原始脚本，等待本次 Turnstile token，提交演示账号，再重新请求 dashboard 验证会话。

本例对应 `inner` 分支当前 runtime，使用默认环境和系统时钟。页面及 iframe 脚本由 SpiderMonkey 执行，请求及 Cookie 由运行时网络层处理。每次启动创建独立 `Context`，token 来自本次页面运行。

## 运行

环境：Windows x64、Python 3.10+。在源码目录中先准备好当前分支的 `dist/js.exe` 及配套 DLL；脚本会加载仓库内的 Python 包，并优先使用这个 runtime。

```
# 在 D:\ruyijs 中运行，使用当前 Clash
python .\examples\scrapingcourse\scrapingcourse.py --proxy http://127.0.0.1:7890 --output .\artifacts\scrapingcourse-example\result.json

# 显式指定 runtime
python .\examples\scrapingcourse\scrapingcourse.py --shell .\dist\js.exe --proxy http://127.0.0.1:7890

# 直连
python .\examples\scrapingcourse\scrapingcourse.py --direct

# 查看参数
python .\examples\scrapingcourse\scrapingcourse.py --help
```

使用包含此例子的包版本时，可通过统一入口启动：

```
python -m ruyijs.examples scrapingcourse --proxy http://127.0.0.1:7890 --output result.json
```

在源码目录通过统一入口运行，可先设置 `$env:PYTHONPATH = "$PWD\python"`。单独复制脚本使用时，需要安装带运行时的 `ruyijs` 包；源码目录的实际测试使用当前 `inner/dist/js.exe`。

| 参数 | 默认值 | 含义 |
| --- | --- | --- |
| `--proxy` | 自动选择 | 优先 HTTPS_PROXY/HTTP_PROXY（含小写），其次本机 7890 端口，最后直连。显式设置时使用指定代理。 |
| `--direct` | 关闭 | 强制直连，与 `--proxy` 互斥。 |
| `--shell` | 自动选择 | 显式路径优先，其次 RUYIJS_SHELL，源码目录 dist/js.exe，最后包自带 runtime。 |
| `--timeout` | 120 秒 | 每个等待阶段的期限；整个流程包含多个阶段。单次 RPC 可能使阶段结束时间晚于此期限。 |
| `--rpc-timeout` | 60 秒 | 单次 runtime 调用期限，包括页面推进和输入。 |
| `--output` | 当前目录 scrapingcourse_result.json | JSON 报告路径；父目录自动创建。 |

退出码 `0` 表示全部验收条件满足，`1` 表示本次流程未通过，参数错误返回 `2`。

## 具体流程

1. 创建 `Context`，选择代理，记录实际 `js.exe` 路径和 SHA256，配置系统时钟。
2. `navigate(LOGIN)` 发起导航。循环调用 `advance(100, max_tasks=100)`，等待 `#email`、`#password` 出现。页面继续加载 Turnstile API、iframe 和挑战资源。
3. 通过 `Context.type()` 输入网站公开的演示账号 `admin@example.com` 和密码 `password`，字符间隔 60 毫秒。
4. 持续推进事件循环，读取隐藏字段 `cf-turnstile-response` 的长度。长度大于 0 才进入提交阶段。长度和内容每次运行都可能变化。
5. `Context.click()` 点击 `#submit-button`。原表单负责提交演示账号、页面隐藏字段及本次验证码结果；网络层处理重定向和会话 Cookie。
6. 等待 HTTPS `/dashboard`、标题 `Success Page - ScrapingCourse.com`、非空 `.product-name` 列表及 `readyState=complete`，同时检查实际 GET dashboard 返回 200。登录 POST 必须返回 302 或 303。
7. 记录已有 dashboard 请求 ID，再次 `navigate(DASHBOARD)`。同时满足页面成功条件和__新的请求 ID 返回 200__后，确认会话有效。这样不会将导航刚排队时残留的旧 DOM 当作第二次成功。
8. 保存步骤、请求摘要、商品、脚本异常、各项验收结果和运行时 hash；返回对应退出码。

请求主链：

```
GET  /login/cf-turnstile                    → 登录页面
GET  challenges.cloudflare.com/...          → Turnstile API/iframe/脚本
POST challenges.cloudflare.com/.../fo/...   → 挑战交互（次数和地址由当前脚本决定）
页面 cf-turnstile-response 写入本次 token
POST /login/cf-turnstile                    → 302 或 303
GET  /dashboard                             → 200 + 商品列表
GET  /dashboard（显式再次访问）              → 新请求 200 + 商品列表
```

已观测到登录 302 先指向 HTTP dashboard，再由 301 升级到 HTTPS；脚本跟随实际重定向，最终验收 HTTPS URL。单独出现挑战 HTTP 200 或非空 token 不构成登录成功。

## 输出与判据

JSON 报告包含以下字段：

| 字段 | 说明 |
| --- | --- |
| `passed` | 全部 checks 为 true，且测试前后 runtime SHA256 相同。 |
| `checks` | 演示凭据已输入、新 token 已生成、表单已提交、登录已接受、dashboard 已重新验证。 |
| `stage` / `failure` | 当前/终止阶段及 Python 或 JS 异常；等待超时也记录在这里。 |
| `token_length` | 提交前观测到的 token 长度。 |
| `first_dashboard` / `final_state` / `products` | 首次和最终页面状态、商品名称。 |
| `network` | id、method、url、status、error 请求摘要。 |
| `steps` | 各阶段状态及相对耗时；等待期间约每 10 秒输出进度。 |
| `script_errors` | 推进事件循环时捕获的 JS 错误，保留 stack；普通页面错误不替代服务器验收。 |
| `console` / `evidence_errors` | 页面控制台与采集证据时的异常。 |
| `shell_sha256` / `shell_sha256_after` | 实际运行引擎的身份和稳定性。 |

遇到 Turnstile `600010`、`600040`、`300030` 时，本例结束该轮并保留异常。其他 JS 异常记录后继续推进，以最终网络和页面条件判定。报告只记录 token 长度和请求摘要；原始请求体不写入示例报告。

## 已有实测

2026-11-01 通过统一入口运行本例，在 `inner` 当前构建、Clash `http://127.0.0.1:7890`、默认环境下的实时验证：

| 项目 | 观测结果 |
| --- | --- |
| runtime SHA256 | `08237528443188fa56b5f9363970ce9b50bd427ccd8a80329bc421f468dab918` |
| 本次 token 长度 | 709 字符 |
| 登录 POST | 302 |
| 首次 HTTPS dashboard | 200 |
| 再次 HTTPS dashboard | 200 |
| 商品数量 | 12 |
| JS 异常 | 0 |
| 总耗时 | 约 27.0 秒 |

这是该次运行的结果；代理出口、网络与网站脚本变化会影响后续运行，脚本每次重新验证并报告实际结果。

## 完整代码

以下代码与同目录 `scrapingcourse.py` 一致，可完整复制保存后运行。

```python
"""Run ScrapingCourse's Turnstile demo login and verify a fresh dashboard request."""
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
if (ROOT / "python/ruyijs/__init__.py").is_file():
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
    try:
        with socket.create_connection(("127.0.0.1", 7890), timeout=0.3):
            return "http://127.0.0.1:7890"
    except OSError:
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
    parser = argparse.ArgumentParser(description=__doc__)
    proxy = parser.add_mutually_exclusive_group()
    proxy.add_argument("--proxy", help="HTTP(S) proxy; default: environment, localhost:7890, then direct")
    proxy.add_argument("--direct", action="store_true", help="Use a direct connection")
    parser.add_argument("--shell", type=Path, help="Path to js.exe; source tree defaults to dist/js.exe")
    parser.add_argument("--timeout", type=float, default=120, help="Deadline for each wait stage, seconds")
    parser.add_argument("--rpc-timeout", type=float, default=60, help="Timeout for each runtime call, seconds")
    parser.add_argument("--output", type=Path, default=Path("scrapingcourse_result.json"), help="JSON report path")
    args = parser.parse_args()
    for name in ("timeout", "rpc_timeout"):
        if not math.isfinite(getattr(args, name)) or getattr(args, name) <= 0:
            parser.error(f"--{name.replace('_', '-')} must be a positive finite number")
    if args.shell is None and not os.environ.get("RUYIJS_SHELL") and (ROOT / "dist/js.exe").is_file():
        args.shell = ROOT / "dist/js.exe"
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
                      "elapsed_seconds": report["elapsed_seconds"], "output": str(args.output)}, ensure_ascii=True))
    return 0 if report["passed"] else 1

if __name__ == "__main__":
    raise SystemExit(main())
```

## 💬 联系方式

二次分发、商业授权、技术交流，欢迎添加作者微信，或扫码加入交流群。

欢迎使用DeepSeek、GLM、Kimi等国内顶尖大模型

```
https://www.fastaitoken.com/优惠码：RUYIJS  注册自带1$
```
