# ScrapingCourse Turnstile 例子在 macOS 上的 600010 排查记录

日期：2026-11-01　分支：`mac-surface-parity`　状态：**未解决**

## 1. 目标与结果

按文章 `docs/articles/2026-11-wechat-scrapingcourse-turnstile.md` 在本机（macOS arm64）运行
`examples/scrapingcourse/scrapingcourse.py`。

**结果：未通过。** 每次都在 `wait_turnstile` 阶段失败：

```
Error: [Cloudflare Turnstile] Error: 600010.
```

`token_generated` 始终为 false，没有走到提交登录。

关键对照：用户在**同一个代理节点**上用真 Firefox 打开登录页，Turnstile 通过。
说明问题出在 ruyijs 的 macOS 引擎，不在出口 IP。具体是哪一项触发的，**仍未找到**。

## 2. 运行环境

| 项 | 值 |
| --- | --- |
| 系统 Python | 3.9.6（不满足 3.10+），改用 `uv run --python 3.12` |
| 运行时 | 仓库内 `dist/js`（macOS 版，非文章里的 `js.exe`） |
| 代理 | Clash Verge，`http://127.0.0.1:7897`（文章写的 7890 本机未监听） |
| 直连 | 本机在北京联通出口，直连 125 秒也加载不出登录页，无法用于对比 |

运行命令：

```bash
PYTHONPATH=python uv run --python 3.12 python examples/scrapingcourse/scrapingcourse.py \
  --proxy http://127.0.0.1:7897 --output artifacts/scrapingcourse-example/result.json
```

## 3. 已排除的原因

| 嫌疑 | 做法 | 结论 |
| --- | --- | --- |
| 出口 IP 信誉 | 换了 CenturyLink、Fourplex、PEG TECH、Contabo 等出口 | 错误码、失败位置不变；且真 Firefox 在 Contabo 出口能通过 |
| 网络指纹（走代理的 CONNECT 隧道） | 用 `tls.peet.ws/api/all` 读 ruyijs 的握手 | HTTP/2 Akamai 指纹 `1:65536;2:0;4:131072;5:16384\|12517377\|0\|m,p,a,s`、优先级、TLS 扩展均符合 Firefox |
| 页面基本表面 | 探针对比 `hasFocus`、插件、屏幕、`oscpu`、`webdriver` 等 | 与真 Mac Firefox 一致 |
| 时区 / 语言 | ruyijs 为 `America/Los_Angeles`、`en-US` | 真 Firefox 是中国时区也通过，不是致命项 |
| JS 异常 | 监听 `message` 与 `error` 事件 | `600010` 是挑战 iframe 自己发出的 `fail` 消息，非引擎异常 |

## 4. 定位手段

- **`message` 监听**：确认挑战 iframe（`non-interactive`）先发 `food` 心跳，再发 `{"event":"fail","code":"600010"}`。
- **`mode="debug"` API 日志**：只看到文档、事件、定时器、`postMessage`、`fetch`、导航、屏幕等访问，没有 canvas、WebGL、音频探测。
  iframe 是跨域的，无法在其中执行代码，所以看不到它具体探测了什么。
- **指纹探针对比**：写了本地探针页 + 小服务，在真 Firefox（157）与 ruyijs（159 模拟）里各跑一遍，自动做差异比对。
  Firefox 本机沙箱里启动不了，探针由用户用自己的 Firefox 打开。

## 5. 发现的差异与对应修复

以下均**已修复但没有让 600010 消失**。

### 5.1 WebGL 数值（D3D 值配 Apple M1 渲染器）

渲染器写 `Apple M1`，但数值是 Windows D3D 的抓包值。在 `__APPLE__` 分支对齐到真 Mac Firefox：

| 项目 | 真 Firefox | 原 ruyijs | 现在 |
| --- | --- | --- | --- |
| MAX_TEXTURE_SIZE / RENDERBUFFER / CUBE_MAP | 8192 | 16384 | 8192 |
| MAX_VERTEX_UNIFORM_VECTORS | 1024 | 4095 | 1024 |
| MAX_VARYING_VECTORS | 32 | 30 | 32 |
| MAX_COMBINED_TEXTURE_IMAGE_UNITS | 80 | 32 | 80 |
| MAX_VIEWPORT_DIMS | 16384 | 32767 | 16384 |
| ALIASED_POINT_SIZE_RANGE | 1 到 64 | 1 到 1024 | 1 到 64 |
| 整数精度 | 24,24,0 | 31,30,0 | 24,24,0 |

文件：`src/web_graphics/model.cpp`、`model_defaults.cpp`、`results.cpp`。
先用运行时 `configure_environment({"webgl":{"limits":...}})` 验证数值生效，再改 C++ 作为默认值。

### 5.2 canvas 字体回退

`font_parse.cpp` 的 `FirstFamily` 只保留字体列表第一项，第一个字体不存在时后面的 `monospace` 丢失，
最后落到无衬线字体（DOM 路径正常，只有 canvas 受影响）。改为保留整个列表。
`Segoe UI,monospace`、`Apple Color Emoji,monospace` 现为 563.550（修复前 647.833；真 Firefox 约 563.333）。

文件：`src/web_canvas/font_parse.cpp`。

### 5.3 资源计时（Resource Timing）

macOS 网络路径不提供请求、响应时间点，`resource_timing_ports.cpp` 把它们都退化成 `fetchStart`，
首字节耗时为 0。对比真 Firefox 后，在“未测到首字节”且非逻辑时钟时：
`requestStart = fetchStart + 2`，`responseStart` 取响应结束前按体积估算的一小段，
DNS、连接、TLS 起点等于 `requestStart`。Windows 实测值与逻辑时钟输出不受影响。

文件：`src/runtime/resource_timing_ports.cpp`。

## 6. 仓库状态

**未提交的修改（5 个 C++ 文件）：**

- `src/web_graphics/model.cpp`
- `src/web_graphics/model_defaults.cpp`
- `src/web_graphics/results.cpp`
- `src/web_canvas/font_parse.cpp`
- `src/runtime/resource_timing_ports.cpp`

**未跟踪文件（非本次排查产生）：** `.zcode/`、`docs/articles/`、`docs/upstream-mac-pr-draft.md`；
`uv.lock` 是 `uv run` 生成的。

**构建：**

```bash
MACOSX_DEPLOYMENT_TARGET=26.0 uv run --python 3.12 python -B tools/build.py \
  --donor-obj ../firefox-159/obj-js-standalone \
  --firefox-gfx-obj ../firefox-159/obj-necko \
  --firefox-font-prefs ../firefox-159/obj-necko/dist/bin/greprefs.js \
  --necko-nss-lib vendor/firefox-downloads/Firefox159.app/Contents/MacOS \
  --profile firefox-environment-macos.json
```

注意：`--necko-nss-lib` 必须指向 `Firefox159.app/Contents/MacOS`，指向 `obj-necko/dist/bin` 会在
`libnecko_core` 链接阶段失败（`dist/js` 本身仍会更新）。

**备份：** 旧 `dist/js` 在 `/tmp/probe/js.bak`；探针脚本与对比数据在 `/tmp/probe/`；
各次运行报告在 `artifacts/scrapingcourse-example/`。

## 7. 尚未验证的嫌疑

1. **缺失的 API 成员**：`WebSocket`（`send`、`readyState`、`url` 等原型成员）、`RTCPeerConnection`（约 20 个成员）、
   `HTMLMediaElement`/`HTMLVideoElement`、部分 SVG 与 Element 方法（`requestFullscreen`、`setPointerCapture` 等）。
2. **canvas 绘制图像哈希**与真 Firefox 不一致（音频指纹有差异，属正常波动）。
3. **`serif` 字体解析**：量出来与无衬线相同（647.833），而 `Times New Roman` 为 620，真机应落到 Times。
4. **行为类检测**：鼠标事件、`isTrusted`、时间节奏等，目前没有证据排除。
5. **iframe 是否继承运行时覆盖**：`configure_environment` 对跨域挑战 iframe 是否生效未确认。

## 8. 复现与对照方法

- 对照实验：用真 Firefox 走同一代理打开
  `https://www.scrapingcourse.com/login/cf-turnstile`，看 Turnstile 是否自动通过。
- 指纹差异：`/tmp/probe/probe.js`（服务 `/tmp/probe/server.py`，端口 8765）+ `/tmp/probe/diff.py`。
- 失败原因：监听页面 `message` 事件，iframe 发出 `fail` 时 `code` 为 `600010`。

## 9. 后续：iframe 覆盖传播实验（2026-11-01 晚，§7.5 已结案）

问题：`configure_environment` 与启动 profile 对跨域挑战 iframe 是否生效？

方法：`/tmp/probe/iframe_server.py`（双端口 8767/8768 = 两个源）+ `/tmp/probe/run_iframe.py`。
顶层页嵌同源 + 跨源 iframe，三方各自跑同一份探针（UA/平台/语言/时区/屏幕/WebGL/canvas 文本/音频），
`postMessage` 聚合后由 Python 判读。Run A = 编译默认，Run B = `configure_environment` 覆盖
（`webgl.limits` 3379→4096、36347→512）。

**修复前：**

| 面 | top | 同源 iframe | 跨源 iframe |
| --- | --- | --- | --- |
| navigator/时区/屏幕/canvas 文本/音频 | mac 值 | mac 值 | mac 值 |
| webgl.limits（编译值 8192/1024） | 8192 | 8192 | 8192 |
| webgl.limits（Run B 覆盖 4096/512） | **4096** | 8192 | 8192 |
| webgl.renderer | **Apple M1** | **ANGLE (NVIDIA, GTX 980 Direct3D11)** | **ANGLE (NVIDIA, GTX 980 Direct3D11)** |

结论三层：

1. **编译进 C++ 的默认值（model.cpp 等）本来就抵达 iframe**——§5.1 的数值修复对挑战 iframe 有效。
2. **环境层（profile + `configure_environment`）只进顶层文档**：`browser_environment/child.cpp` 的
   子 Environment 构造不接管缓冲组（`webglGroup` 等六个 + `owners[]` 五个槽），只有 `Succession`
   （后继顶层文档）接管。iframe 的 WebGL owner 落回 `domain::GlParameters` 默认值
   （`platform_apis.h` 的 Windows GTX 1080 抓包，经 sanitize 桶到 GTX 980）——
   挑战 iframe 自身看到"MacIntel 平台 + Windows D3D11 显卡"的自相矛盾。真 Firefox 的 GPU 是
   整机一份，所有 frame 同串；worker 路径（`worker_mode.cpp`）也早就继承创建者的缓冲组，
  唯独 child 漏了，属疏忽。
3. **修复后（child.cpp 接管全部缓冲组 + Succession 补 `mediaGroup`，已重建 dist/js）：
   Run A/Run B 全部 SAME，覆盖值 4096/512 三方可见。**

**但 Turnstile 仍 600010**（`artifacts/scrapingcourse-example/result-after-iframefix.json`，
同一 `api.js` Error 构造签名）——renderer 矛盾是真 bug 已修，但不是 600010 的触发点。
§7.5 结案：配置会进挑战 iframe（编译值一直有效，环境层现已修复）。

后续嫌疑剩余（按性价比）：§7.1 API 成员缺失（WebSocket/RTCPeerConnection/HTMLMediaElement，
真 FF159 对照枚举一次出清单）> serif 解析 > 音频 ULP（blink FFTFrame）> 行为类。

旧 runtime 备份：`/tmp/probe/js.pre-iframefix`（含 5 文件修复、不含 child.cpp 修复）。
注意：重建后 `dist/necko_tls/libnecko_core.dylib` 会重新出现，须再次改名为 `.pr10-pending`。

## 10. 后续：API 表面枚举（§7.1 结案）

方法：`/tmp/probe/surface.js`（window 自有名 + 108 个构造器的 prototype/static + 9 类实例成员）
在 ruyijs（`run_surface_ruyi.py`，load_page + eval）与真 FF159（`surface_server.py`:8769 + 新装 profile
headed 实测，结果 POST 回 `surface_firefox.json`）各跑一遍，`surface_diff.py` 做**集合 + 顺序**双重对比。

**结论：§7.1 名单上的嫌疑全部不成立。** WebSocket/RTCPeerConnection/HTMLMediaElement/HTMLVideoElement、
SVG、Element（`requestFullscreen`、`setPointerCapture`）等全部 108 个 prototype、9 类实例成员
（含 `new WebSocket`、`new RTCPeerConnection`、`createDataChannel`、AudioContext）
成员集合与顺序**全部一致**；navigator/screen/document 也一致。窗口多出/漏出成员为 0。

真实差距收敛为三类：

| 类 | 内容 | 量 |
| --- | --- | --- |
| ctor static 顺序 | 19 个构造器是 `[prototype,length,name]`（Window/Navigator/Screen/Performance/SubtleCrypto/Storage/Location/History/Headers/Request/AbortController/MutationObserver/IntersectionObserver/ResizeObserver/StorageManager/ReportingObserver + 4 个 RTC 事件类）；Response/AbortSignal/PerformanceObserver 顺序也不同 | ~22 |
| ctor static 缺成员 | `HTMLMediaElement` 缺 12 个常量（NETWORK_EMPTY…HAVE_ENOUGH_DATA）、`SVGSVGElement` 缺 3 个 SVG_ZOOMANDPAN_*、`Document` 缺静态 `parseHTML`、`WebGL2RenderingContext` 的 `TIMEOUT_IGNORED` 位置错 | 4 |
| window 全局 | 缺 `Serial`/`SerialPort`/`ViewTimeline`/`ScrollTimeline`；`Object.getOwnPropertyNames(window)` 顺序 983/983 全错位（FF 为 xpconnect 排布：`undefined, globalThis, Boolean, JSON…` 打头，`Object/Function/Array/eval` 落尾；我们是 SM 裸全局顺序） | 4 + 全局顺序 |

修复抓手（现成基础设施）：`src/runtime/binding_order_table.cpp` 已有"按 donor 顺序重定义 prototype
own keys"的 pass（`tools/binding_order.py` 生成表）——prototype 顺序 108/108 全对即其成果。
同理可扩展：(a) 对构造器对象做 static 重排；(b) 对 window 全局做访问器保形重排（需保留
getter/setter，不能固化值，比 prototype 版风险高一档）；(c) 缺成员按 7538291 的
no-legacy-code 先例补齐。

对 600010 的意义：这些是探测面上**新的、此前未测过的**差异源（尤其 window 顺序 100% 错位，
一次 `getOwnPropertyNames(window)` 即全露）。Windows inner 通过不排除它已修这些——inner 私有无法验证。
数据存档：`/tmp/probe/surface_{ruyijs,firefox}.json`。

## 11. 后续：表面修复落地（§10 全部修完，600010 仍在）

按 §10 的清单修完并重建（本轮未提交，`mac-surface-parity`）：

1. **138 个构造器的 ctor static lead 翻正**（`[prototype,length,name]` → donor `[length,name,prototype]`）：
   - 新 helper：`runtime/page_helpers.h` 的 `LeadFunctionKeys`（惰性 length/name 在 permanent
     prototype 键之前解析——`web_kit.cpp` 已有手法的推广），约 28 个手搓创建点逐一插入；
   - interaction 层 4 个 `JS_InitClass` 调用点改为新 `interaction::InitClassOrdered`（手动按正确序
     创建 prototype/ctor；`prototype` 属性不可配置，事后无法重排，只能改创建序）；
   - `AbortSignal`/`TaskSignal`/`PerformanceNavigation`/`XMLHttpRequest` 的 statics/常量移到
     prototype 键之后；`PerformanceObserver.supportedEntryTypes` 同理。
2. **proto 常量的 PERMANENT 修正**：XHR/HTMLTrackElement 的 prototype 常量原带 PERMANENT，
     binding-order pass 因不可配置而跳过排序；改为可配置（donor 如此），pass 排序后经
     `SealConstants` 封只读——与 `event.cpp` 既有手法一致。
3. **补静态成员**：`HTMLMediaElement` 9 常量、`SVGSVGElement` 3 常量、`Document.parseHTML`
   静态（与 parseHTMLUnsafe 同路径）、WebGL2 `TIMEOUT_IGNORED` 归位（constants_3.inc，
   ctor-only 标记避免误写 prototype）。
4. **补成员**：`FontFaceSet` 的 `onloading/onloadingdone/onloadingerror`（getter null/setter 忽略，
   无字体加载事件）、`HTMLTrackElement.track`（null getter）。
5. **形状修正**：`NodeFilter` 去掉不该有的 prototype（donor 是无 prototype 的普通函数，
   常量在函数自身），`DOMQuad.fromRect/fromQuad` 换序。
6. **4 个 window 全局**：新 `browser_environment/window_extras.cpp`——`Serial`（链
   EventTarget.prototype）/`SerialPort`/`ViewTimeline`/`ScrollTimeline` 的成员名与顺序按 donor
   枚举面齐平；**无实现**（调用抛 TypeError），只保证枚举/存在性保真，文件头注明。

**验证**：`/tmp/probe/surface_diff.py` 全量 1523 行对比，**1521 ok**；集合差 0、多出成员 0。
剩 2 项已知微差：(a) `window_own` 全局枚举顺序（round ③ 范围，需访问器保形重排）；(b)
`FontFaceSet.proto` 的 `keys` 位置（donor 在 constructor 之后，binding-order pass 恒置
constructor 末尾，现有机制无法表达，接受）。`tests/test_macos_runtime.py` 18/18 PASS，
iframe 传播实验回归无退化。

**Turnstile：仍 600010**（`result-after-surface.json`，同签名）。结论：API 表面枚举层已与
真 FF159 收敛到 diff-engine 分辨率内（除上述 2 项），600010 的触发点在表面之外——剩余嫌疑
按序：音频表生成 ULP（blink FFTFrame 移植）、canvas stroke AA、行为/时序类信号、或某项
未入枚举面的动态探测。
