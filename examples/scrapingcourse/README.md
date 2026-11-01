# ScrapingCourse（Cloudflare Turnstile 登录）

ruyijs 原生环境通过 [ScrapingCourse](https://www.scrapingcourse.com/login/cf-turnstile)
的 Turnstile 登录演示：等待本次 token、提交演示账号、复核 dashboard 会话。

```bash
python scrapingcourse.py --proxy http://127.0.0.1:7897 --output scrapingcourse_result.json
python scrapingcourse.py --direct
```

对应作者公众号示例（2026-11-01，Windows `inner` 分支实测：token 709 字符、
登录 302、dashboard 200×2、总耗时约 27s）。本例为零定制移植——默认环境 +
系统时钟，脚本不注入任何 API。

## 流程与判定

```
GET  /login/cf-turnstile                    → 登录页
GET  challenges.cloudflare.com/...          → Turnstile API/iframe/脚本
POST challenges.cloudflare.com/.../fo/...   → 挑战交互（脚本自行决定）
隐藏字段 cf-turnstile-response 写入 token   → 长度 > 0 才提交
POST /login/cf-turnstile                    → 302/303
GET  /dashboard                             → 200 + 商品列表
GET  /dashboard（显式再导航）                → 新请求 ID 200 → passed
```

不把"挑战 200 / token 非空"当成功，只认服务端验收；二次导航要求
**新请求 ID** 返回 200，避免把导航排队时残留的旧 DOM 当作第二次成功。
Turnstile 错误码 `600010/600040/300030` 判死保留现场。

## 注意

- 出口 IP 信誉同样影响 Turnstile：循环失败先换 `--proxy` 出口
  （见 [truepeople](../truepeople/README.md) 的边缘冷却结论）。
- 登录 302 可能先指向 HTTP dashboard 再 301 升级 HTTPS，脚本跟随实际重定向，
  最终验收 HTTPS URL。
- 报告记录 runtime SHA256 前后对照（`runtime_unchanged`），引擎中途被换会判失败。
