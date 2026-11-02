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

## 12. 后续：window 顺序否决 + 音频 ULP 真根因（同晚）

**window 全局顺序（§10 遗留项 a）：pass 方案否决。** 投影实验：7 个不可配置键
（undefined/NaN/Infinity + unforgeable 的 window/document/location/top）物理上不可移动；
donor 把 NaN/Infinity 插在序列中部（50/51）、四个 unforgeable 插在尾部（975-978），
我们这 7 键聚在头部，可配置序列即使按 donor 序重排也整体错位——投影匹配 **9/987（1%）**。
100% 对齐需要 SM 全局惰性解析序（NaN/Infinity 位置引擎不可控）+ unforgeable 定义点手术，
且 donor 相对序（window, document, location, top）与现状（window, top, location, document）
不同。收益/风险比不成立，**记录为接受项**：全序对比型探测本就会在其它差异上失败，
presence/排序型探测与顺序无关。

**音频 1 ULP 的真根因（修正此前"blink FFTFrame 未移植"的判断）。** 我们的
`domain/audio/periodic_wave_table.h` 早已是 ffvpx 仿真（ff_tx_rdft_c2r 折叠 + split-radix
逆变换），但仿真的是 **x86 `fft_sr_float_fma3` codelet** 的舍入。FF159 的 aarch64 codelet 表
（`media/ffvpx/libavutil/aarch64/tx_float_init.c`）显示 mac arm64 实际派发到 **NEON 手写汇编**
`ff_tx_fft_sr_float_neon`（64..131072 全长覆盖），融合与排序与 FMA3 不同——这就是
sample-7 1 ULP 的来源。**这也解释了 Windows 过/mac 挂的不对称**：Windows 上 FF 跑
fma3 codelet，我们仿真正确；mac 上 FF 换 NEON 后端，我们没跟上。

下一步（半天到一天）：把 `tx_float_neon.S` 的 `ff_tx_fft_sr_float_neon`（含 fmla/fmul
顺序与 lane 排布）逐指令翻译为 C++/NEON 内建，替换 `split_radix_fft.h` 的 butterfly，
用逐样本对拍（engine vs 真 FF159 经 audio probe）验证 bit-exact。Windows 路径不受影响
（派发按平台）。

**对拍 rig 已就绪**（`/tmp/probe/`：`audio_server.py`:8769 + `run_audio_ruyi.py` +
`audio_diff.py`；FF159 headed 采集同轮完成）。triangle 10000Hz 的确定性靶值：

- FF159 sum = `28074.93179304037`，我们 = `28074.931769745912`
- 首个分歧 `head[7]`：FF `0xBD7C05BF` vs 我们 `0xBC7C05BF`（同指数，尾数 1 ULP）
- head 64 样本 51 个 bit 相同

翻译完成后 `audio_diff.py` 应报告 head 64/64 bit-identical 且 sum 相等。我们的现有实现
（`split_radix_fft.h`）是 C 模板（fma3 codelet）的忠实仿真，结构可直接复用：NEON 版差异
集中在叶子核（NEON 用 FFT8_X2/FFT16 4-lane 宏，lane 序经 zip/trn/rev64/ext 重排）与
SR_COMBINE 族（fmla/fmls 顺序 + v31=(-1,1,-1,1) 符号乘），主循环按 32/64/128/256/512
递归重入、存储按 stp 交错布局。


## 13. 落地：音频表生成与真 Firefox bit-exact（同晚）

按 §12 的方向落地，方案从"逐 lane 仿真 NEON"切换为"链接真 ffvpx"：

- 新 `vendor/ffvpx-tx-native/`：冻结 Firefox 159 ffvpx 的 av_tx 源码
  （`manifest.json` 逐文件 SHA-256 校验），含 aarch64 NEON 汇编变换与
  `config_darwin_aarch64.h`。LGPL-2.1-or-later，以**独立可重链接目标文件**参与
  链接（Mozilla ffvpx 同款安排；引擎源码零头文件耦合，仅 `av_tx_init`/
  `av_tx_uninit` 自声明 C 接口），见目录内 README。
- `tools/ffvpx_tx_native.py`：仅 macOS arm64 启用，编译 tx/bprint/log/mem/
  error/cpu/mathematics/time + NEON 汇编为独立对象；其他平台保持
  `split_radix_fft.h` 仿真。
- `periodic_wave_table.h`：arm64 mac 上 `BandLimitedWave` 直接以 FFTBlock 的
  参数（scale 1.0、复数元素 stride）驱动真 RDFT；仿真路径保留为回退。

**验证**：harness 内 our-fold vs oracle-fold **0 差异（2049/2049）**；引擎音频
triangle 10kHz sum = `28074.93179304037` 与真 FF159 **完全相等**，head 64/64
样本 bit-identical。`tests/test_macos_runtime.py` 18/18 PASS，iframe 实验无退化。
（勘误：此前 suspicion 表的"仿真 fold"与 oracle 有别的假象，一部分来自
harness 喂入了 partials 以上的 bin——仿真与 oracle 的差异本来就只在 subtx。）

**Turnstile：仍 600010**（`result-after-audio.json`）。至此枚举表面与音频表面
都已收敛到与真 FF159 不可区分，剩余嫌疑：canvas stroke AA（Skia 路径）、
行为/时序类信号（timer 节奏、isTrusted 事件流）、或挑战内部某项未观测到的
动态探测。


## 14. 原文对照 + DOM 原型链根断裂（同夜，链根已修）

与公众号原文逐行对照：**脚本层零差异**（作者流程即我们移植的流程，无指纹配置、
无隐藏步骤，"默认环境和系统时钟"）；作者一键过的本钱是**他打磨多年的 Windows
`inner` runtime**（SHA256 08237528…）。差距全部在 runtime 表面，不在脚本。

**新实锤：DOM 接口原型链根断裂。** identity 探针（51 项：DOM Collection /
RegExp / Function / Node identity + console 语义，双侧同页对拍）暴露：在
ruyijs 里对任何 DOM 原型调 `hasOwnProperty` 直接 **TypeError** —— 页面脚本
当场崩死。根因：`interaction::InitClassOrdered` 的 `parent_prototype = null`
分支给了空原型，而 `JS_InitClass(nullptr)` 的语义是把链根接到
**%Object.prototype%**。修法：null → `JS::GetRealmObjectPrototype(cx)`。
真 FF159 判据（chain 断言实测）：`EventTarget.prototype` 的 proto 是
Object.prototype（true）、`Node.prototype.hasOwnProperty` 是 function。
注：`Function.prototype` 的 proto 在 FF159 里是 **Object.prototype 而非
自身**（与 V8 不同，勿以 V8 直觉硬编码）。

修复后探针 **51/51 项与 FF159 完全一致（0 differences）**；tests 18/18。
**Turnstile：仍 600010**（`result-after-protoroot.json`）。剩余嫌疑不变：
stroke AA、行为/时序、未观测动态探测。

对拍资产：`/tmp/probe/cprobe_server.py`:8769 + FF159/ruyijs 双侧 JSON +
直接 diff 脚本（见会话）；探针可继续加项。


## 15. stroke AA 复测：11/11 逐像素一致（同夜，负结果）

逐用例像素对拍（`/tmp/probe/sprobe_server.py`:8769，FF159 与 ruyijs 双侧同页）：
line45 / line15 / rect / circle / arcHalf / bezier / quad / dash / thin(0.5px) /
roundCap / miterJoin —— **11/11 用例的 lit 像素数、alpha 总和、全帧 FNV hash
完全一致**。原因：`skia_native` 链接的就是 FF objdir 的预编译 Skia 对象，而早先
观测的 alpha 差异已被后续表面修复顺带消除。

至此已知表面差距全部收敛或排除。剩余嫌疑：行为/时序类（advance 泵的 timer
节奏 vs 真 FF 帧驱动）、window 全局枚举序（已论证接受）、未观测的动态探测。


## 16. 行为/时序对拍（2026-11-02，三个软信号偏差）

`/tmp/probe/tprobe_server.py`:8769 双侧同页（FF159 headed vs ruyijs advance 泵）：

| 项 | FF159 | ruyijs | 判定 |
| --- | --- | --- | --- |
| setTimeout(0..100ms) 实际漂移 | 2/2/5/10/50/100 | 0/2/5/12/53/102 | setTimeout(0) 我们 0ms 立即触发，FF ≥2ms（可测、可修） |
| setInterval 20ms ×20 | 19–21ms 稳定 ±1 | 前 10 拍 22–26，后 20–24 | advance 泵的 RPC 间隙造成 interval 方差偏大（架构性软信号） |
| performance.now 200 快读 | 1 个唯一值（1ms 网格强截断） | 2 个唯一值、跨格 | 网格截断方向微妙差异（CF 经典探测；修前需精测 FF 网格语义） |
| 嵌套 clamp / clockDrift / rAF(250ms) / timer 触发序 | 3–5ms / 1ms / 17 / 同序 | 4–7ms / 0ms / 16 / 同序 | ✓ 一致 |

结论：时序不是功能性断裂，是**统计性软信号偏差**（风险分加权项，非硬门槛）。
最值得修的是 setTimeout(0) 最小延迟；interval 方差源于 advance 泵架构。


## 17. setTimeout(0) 最小延迟已修（同日）

`Model::Schedule`：顶层 Timeout 的 duration clamp 到 ≥2ms（FF159 实测
setTimeout(0..1) 均读 ~2ms；此前我们 0ms 同轮触发是可探测签名）。复测：
我们读 3ms（2ms 网格 + 泵间隙），FF 2ms——量级一致 ✓。tests 18/18。

构建勘误：`ffvpx_tx_native` 的 stamp 缓存分支未用带目录前缀的对象名，
缓存 miss 时 fall-through 会把旧对象重复 append（12+14=26）导致链接
duplicate——缓存与编译两侧已统一 `obj_for()` 命名。


## 18. VM 采集面穷举对拍（2026-11-02）：基本字段全一致（同日）

新版挑战（api.js `g/c787ce48cf6b`，invisible 无 iframe 模式，VM 跑在页面 realm）
的 access log 实测采集面：**navigator 21 字段 + screen 10 + location + viewport +
storage + locale + history + 8 matchMedia + visibilityState + speech + css.supports
color**——没有 canvas/WebGL/audio。

穷举对拍（ruyijs 全字段值 vs FF159 DevTools 实测）：**全部一致或自洽**。
误判排除记录：oscpu（FF159 = "Intel Mac OS X 10.15"，与我们一致）、
doNotTrack（FF 默认 "unspecified"，一致）、speechSynthesis 首查 voices=0（一致）。
自洽身份差异（非穿帮）：hardwareConcurrency 8 vs 宿主 12、screen 1440×900/24bit
vs 宿主 1512×982/30bit——"一台 M1 基础款 mac"是真实存在的配置。

**结论：600010 判分的依据不在任何可枚举的"值"里。** 剩余嫌疑收窄至
(a) 引擎实现的行为特征（advance 泵响应模式、异常形状等静态对拍不可及项）、
(b) 网络层服务端信号（头顺序/Cookie/缓存语义/连接复用）、
(c) CF 每日轮换的新探测。盲找不可持续，向作者索取 Windows 验收清单为
当前唯一高确定性路径。


## 19. 上游 README 对照 + 宿主一致身份实验（2026-11-02）

**上游 main 的 truepeople README（作者 20261031 亲笔）记录了他自己在 upstream 上
打同一场仗**：truepeople 的 CF 托管质询，mac 同样卡在"质询信号评分"——网络全
200、SW 健康、指纹变体（mac 基线/Windows 档案/只换 navigator）全部无效，边缘
不下发 cf_clearance；真 FF159 geckodriver 同出口跑同质询**也未通过**。作者的
结论提出"**派生面边界**"概念：声明面（UA/navigator/screen/tz/locale）可任意
设定且已自洽；**派生面**（canvas 像素=CoreText+Skia、字体度量、时序）由宿主
实现派生，与 CF 认识的基线的差才是评分来源。他列的"下一步"（逐信号与真 FF
对照 canvas 哈希/字体枚举/时序）——**本分支已全部完成**（bit-exact / 11/11 /
三软信号量化）。

**宿主一致身份实验**：profile 改为宿主真值（hardwareConcurrency 12、screen
1512×982 colorDepth 30、tz Asia/Shanghai——消除"声明 8 核 1440×900 vs 派生
12 核渲染性能"的交叉矛盾嫌疑，即 Camoufox-Docker 同款检测）后住宅直连重跑：
**仍 600010**。身份-派生交叉矛盾假设亦排除。该宿主一致 profile 保留（派生
信号与声明同源，理论更优）。

**最终定位**：声明面 ✅、派生可对拍面 ✅、IP ✅、身份交叉 ✅——600010 的
残余原因是**派生面里无法静态对拍的行为特征**（CoreText/Skia-arm64 光栅化的
非确定性部分、GC/JIT 节奏等）或 **inner 分支的私有环境实现**（"补环境"的
"补"字所在，upstream 无此代码）。作者 20261031 的 README 证明他自己也还在
这条边上权衡——**这正是联系作者的充分理由：双方被同一 verdict 卡着，且
inner 的"补环境"内容只有他知道**。


## 20. Windows-bias 系统性审计（2026-11-02，第一轮）

问题定位反思：引擎源码以 Windows 为第一平台开发校准，mac 是移植——**隐含的
Windows 行为假设**无法靠对拍发现（对拍只能覆盖"知道该看哪"的面）。第一轮 grep
审计结果：

| 项 | 状态 |
| --- | --- |
| 平台条件编译（`_WIN32` 分支） | 均为合法平台分支（随机源 BCrypt/getentropy 等），无行为差 |
| timer 精度 `kTimerResolutionUSec=1000`（1ms） | ✓ 与 FF reduceTimerPrecision 默认一致 |
| **AudioContext 延迟** | ❌ **引擎默认 `baseLatency=0` / `outputLatency=0.04`（state.h）——Windows 采样值**；mac 真值需 CoreAudio 实测（FF159 采集反复失败：headed 会话无音频设备，AudioContext 挂起）——**待办**：真机 DevTools 读一次真值填进 mac profile 的 audio 组 |
| mediaDevices/clipboard/notification | ⚠️ 有实现，未做值级对拍（第二轮） |
| socket process（Windows 有、mac 无） | ❌ 架构级：连接复用/时序模式差异的潜在来源，评估成本高 |
| **macOS timer coalescing / App Nap** | ⚠️ interval 方差的潜在根源（进程 QoS 未设置高精度定时器策略），第二轮验证 |

审计方法论沉淀：引擎里每个"值"有三个可能来源——donor Firefox（同源 ✓）、
profile 声明（可设定 ✓）、**平台采样（Windows 校准的硬编码）**。第三类就是
Windows-bias 的藏身处，逐个 grep 常量名可系统性清除。


### §20 勘误（FF159 DevTools 真值回传）

AudioContext 延迟实测：FF159 = `baseLatency 0 / outputLatency 0 / 48000Hz`
（本机 headed 会话无音频输出设备），与我们引擎默认值**完全一致**——"Windows
采样默认值"嫌疑被真值推翻。审计方法论强化：**审计发现的嫌疑也必须真值验证**，
"平台值理论上应该不同"的推理不能替代实测。


### §20 第二轮（同日）：mediaDevices 摄像头缺失已修

值级对拍（enumerateDevices/constraints/notification/permissions/clipboard，13 项）
捕获真实差异：**`enumerateDevices` 缺 videoinput**——FF159 mac 报内置麦克风 +
摄像头两个未授权设备（label/id 全空），我们只有 audioinput。设备数是经典指纹项
（"这台机器没有摄像头"）。修复：默认设备列表补 videoinput（state.h）。复测
**12/13 一致**（唯一残差是基准文件里 FF 消息被截断的假差，实际 13/13）；
tests 18/18。其余项（constraints 集合、notification、permissions、clipboard
拒绝消息）全部一致。


## 21. 第三轮审计：WebRTC 面（2026-11-02，FF 侧采集受阻待续）

`/tmp/probe/rtc_probe_server.py`:8769（createOffer SDP 行结构 + 5s ICE gather +
MutationObserver 式事件采集）。ruyijs 侧输出已归档（`rtc_ruyi.json`）：SDP 17 行
（o=mozilla...THIS_IS_SDPARTA-99.0、ice-options:trickle、BUNDLE 0、
datachannel m-line）、5 个 mDNS host candidates（2×UDP + 2×TCP tcptype active
+ 1 个非候选末项）、gathering complete。

**FF159 侧采集 3 次失败**（页面无回传：无 JSON 无 error——页面脚本疑似未执行，
原因待查：可能 rtc 页面 RTCPeerConnection 构造在此 profile 挂起，或加载竞态）。
待续：改用 DevTools 手跑（用户协作模式，如 §18 的 navfields）拿到 FF 结构真值
后对拍；重点核对 SDPARTA 版本号、candidate 数量与 TCP/UDP 配比、ufrag/pwd 长度。


### §21 收尾：ICE candidates 接口数已修（同日）

FF159 真机实测（单接口 mac）：**1 个 mDNS 主机名 × (UDP priority 2122252543 +
TCP active 2105524479) = 2 条候选 + 空结尾，foundation 0/1，编号 candidate:0/1**。
我们原实现写死模拟"两个本地接口"（2 UUID × 4 条）。已按实测修正
`web_rtc/ice_gathering.cpp`（单 mDNS 名、foundation 0/1、priority 对齐）。
复测：SDP 17 行结构一致（ice-pwd 随机值除外）、**candidates 3 = 3 完全同构**、
gathering complete。tests 18/18。

WebRTC 面审计关闭。Windows-bias 审计累计：音频（真值一致）、摄像头（已补）、
timer 精度（一致）、ICE 接口数（已修）——剩余大项仅 socket process 架构与
macOS QoS 实验。


## 22. 全修复后双路验证（§21 收尾，2026-11-02 深夜）

全部 24 轮修复落地后双路实测：
- Contabo 代理（`212.56.47.88`）：挑战交互正常（2× POST /fo/ 200），**仍 600010**
  （`result-after-ice.json`）
- 住宅直连（北京联通）：**同样 wait_turnstile 失败**（`result-ice-direct.json`）

至此本轮系统性攻坚的最终账本：声明面、枚举面（1500+ 项）、原型链、音频
（bit-exact）、canvas（fill/stroke bit-exact）、设备枚举（含摄像头）、ICE 接口数、
setTimeout 签名、时序三软信号、IP（DC/住宅）、身份交叉——**全部收敛或排除**。
残余差距锁定在：不可静态对拍的派生行为（光栅化非确定性、GC/JIT 节奏、事件循环
微观调度）、或 inner 分支的私有环境实现。后续突破依赖 inner 的信息（只有作者
持有）或 CF 探测的持续逆向。


### §20 自查修复（同日）：代码审查发现的三处问题

1. **window_extras 的 ViewTimeline/ScrollTimeline 原型链断在 null**——DefineStub
   对 nullptr parent 直接给了 null [[Prototype]]。补 `JS::GetRealmObjectPrototype`
   回退（与 InitClassOrdered 同款），并把 ViewTimeline 链到 ScrollTimeline。
   实测链：ViewTimeline -> ScrollTimeline -> Object（缺 AnimationTimeline 层，
   FF 有——低优先遗留）。
2. **getUserMedia({video}) 错误类自相矛盾**：设备列表补了摄像头之后，无权限请求
   却报 NotFoundError（"找不到设备"）。已改为 NotAllowedError（FF 无权限语义）。
3. **DefineStub 签名重构**：输出 prototype/function 句柄，供 ViewTimeline 引用
   ScrollTimeline.prototype。tests 18/18。


### §20 补全确认（FF159 DevTools 真值，用户回传）

AnimationTimeline 补全后 FF159 实测对照：`AnimationTimeline -> Object` ✓、
`ViewTimeline -> ScrollTimeline -> AnimationTimeline -> Object` ✓ —— **链结构
与真 Firefox 完全一致**。getUserMedia({video}) 在 FF 侧走权限流程（1.5s 内无
拒绝记录），我们报 NotAllowedError 为合理近似（CF 探测通常只比对错误名）。
timeline 面审计关闭。


## 23. VM 采集面复测：与已覆盖面完全重合（2026-11-03）

新挑战版本 access log（19667 行，`access_log_v2.jsonl`）与昨日逐 kind/字段 diff：
**完全重合，零新增**。11 类（navigator 21 / screen 10 / location 9 / viewport 3 /
storage 3 / locale 2 / history / matchMedia 8 / visibilityState / speech 3 /
css.supports color）——**全部已在 §18/§20 与真 FF159 穷举对拍一致或自洽**。

逻辑闭环达成：VM 只读这些字段 → 字段值全部一致 → 同 IP 同窗 FF 过、我们不过
→ **判定不在采集值，而在引擎响应读取的行为特征或 /fo/ 之外的深层服务端信号**。
这两者都无法静态对拍（前者是引擎实现微观行为，后者是 CF 服务端黑盒）。
静态可查的检测点地图已 100% 覆盖并收敛。


### §20 时间系统校准（同日，多源交叉）

`/tmp/probe/time_probe_server.py`（tp2 页）+ Chrome DevTools MCP 参考跑：
**Chrome（无 RFP 缩减参考）**：now() 精度 0.1ms（1000 读 4 唯一值）、
scheduleGap [3, 1.5, 5]、5s 推进比值 5002/5002.4、eventTimeStamp 与 now 差 0。
**ruyijs**：timeOrigin Unix ms 量级正确、`timeOrigin + now - Date.now() = 0`
（三者完美对齐）、now() 1000 读 unique=1（1ms RFP 网格生效）、scheduleGap0=3
（clamp 生效）、**Date vs 宿主墙钟偏差 -0.3ms**。FF159 侧：tprobe 实测
setTimeoutDrift 2/2/5/10/50/100、now 200 读 unique=1（同 1ms 网格）——**与
ruyijs 一致**。FF159 tp2 页级精确对拍因页面回传不稳定未完成（无音频设备的
headed 会话限制），时间系统判定为**已校准同步**（三源交叉：FF tprobe 网格 =
ruyijs 网格；Date 墙钟偏差 0.3ms；推进比值正常）。


## 23. 时区/出口匹配规范（2026-11-03）

时间系统实测结论（§20 时间校准）：请求头无时间泄露、Date UTC 与宿主墙钟偏差
0.3ms、JS 本地时间显示由 profile tz 决定。**tz 必须与出口 IP 地区自洽**——
tz/IP 矛盾（上海时间 + 美国 IP）是真实可探测的不一致。

两个 profile 版本：
- `profiles/firefox-environment-macos.json` —— **tz=America/Los_Angeles**，
  配美国出口（Clash 代理，默认场景）。构建默认使用。
- `profiles/firefox-environment-macos-cn.json` —— **tz=Asia/Shanghai**，
  配住宅直连/中国出口。构建时 `--profile profiles/firefox-environment-macos-cn.json`。

两版共享宿主一致身份（12 核 / 1512×982@30 / en-US）。切出口 = 切 profile 重建。
注意：profile JSON 不接受未知键（"invalid Firefox environment baseline groups"
校验），使用条件以本节为准，不写入 profile 文件。
`_usage` 键标注使用条件（引擎忽略未知键）。


## 24. 六层检测体系对照终表（2026-11-03，来源：segmentfault CF 对抗综述）

| 层 | 文章要点 | 我们的覆盖 |
| --- | --- | --- |
| L1 IP 信誉 | DC 低/住宅高 | ✅ 双极实测（DC+住宅均 600010 → 排除） |
| L2 TLS/JA3 | 最易忽视，改 UA 无用 | ✅ peet.ws 实测一致（真 NSS/Necko） |
| L3 请求头完整性/顺序 | 顺序不符先暴露 | ✅ 同一 Gecko 栈生成（天然一致）；echo 抓头无时间泄露、无缺失头 |
| L4 JS Challenge | 混淆 JS 环境检测 | ✅ VM 扫描完整执行（access log 全量确认） |
| L5 浏览器指纹 | webdriver/canvas/WebGL/字体/插件/分辨率 | ✅ 超出文章深度：bit-exact 级（音频/canvas/枚举 1500+） |
| L6 行为分析 | 鼠标轨迹/点击偏移/停留 | ⚠️ mousemove 轨迹未模拟（文章自注"多数站不开启"）；type 60ms 间隔有 |

**文章的定位是外部工具（requests/Playwright）从零攻坚**；我们的引擎已内嵌真
SpiderMonkey + 真 Necko + FF Skia——L2/L3/L4 天然同源，L5 已磨到 bit-exact，
L1 已双极排除。**与文章方案对比表中"官方 API"之外的最强状态等价，且静态可
对拍的检测点已 100% 覆盖**。残余仍在派生行为深处与 inner 私有实现。


### §24 L6 落地：真人鼠标轨迹与点击偏移（2026-11-03）

`InputControl.glide(target, duration_ms, offset)` + `click(glide=True, offset)`：
- glide：smoothstep 弧线（弦长的 12% 控制点）+ 落点区抖动衰减，62 个
  pointermove/mousemove 对、间隔 9.6ms±（帧节奏），movementX/Y = 相邻差值；
- click：down/up/click 全落在轨迹终点（小数偏移坐标，不再是整数正中心），
  落点前 40–90ms 瞄准停顿；
- C++ `Click` 增加 offset_x/offset_y（RPC 字段，±2000 有限值），落点不再取整。

实测事件流（mp 探针）：67 事件 / 62 move / 坐标 (126,77)→(252,167) 渐变弧线 /
间隔 min 0 max 69 avg 9.6 / down=click 落点一致。tests 18/18。
**Turnstile：仍 600010**（`result-humanized.json`）——L6 轨迹已真人化，verdict
不变，行为层的增益需 CF 侧开启行为分析才可观测。


### §23 补充（同日）：根因确认为引擎缺 ES module script 执行

IPPure 页是 VitePress（全 ES module 构建）。实测引擎内：
- `<script type="module">` 最小用例**不执行**（经典 script 正常）
- 页面 net_log 中 **0 个 /assets/ JS 请求**（app module 与全部 chunk 从未加载）
- Rocket Loader 加载了但只处理经典脚本，跳过 module

**影响面**：所有以 ES module 为入口的现代站点（Vite/VitePress/webpack-ESM/Next
等）在本引擎上 JS 整体不工作——页面为纯静态骨架。这是引擎的功能缺口（非
表面差异），也是 IPPure 检测组件静默的原因。修复需实现 SM159 的 module 加载
（JS::CompileModule/Instantiate/Evaluate + 动态 import 钩子，中等规模工程），
完成后现代 ESM 站点整体可用，IPPure 检测也随之工作（可作为环境分的直接试金石）。


### §23 续（同日）：parser src module 已通，IPPure 残余卡点待查

验证矩阵（最小用例实测）：
- parser-inserted src module（`<script type="module" src>`）：✅ 执行
- 内联 module script：❌ 未实现（Source 文档化的边界："Inline module evaluation
  and dynamic module preparation remain outside this owner pass"）
- **动态插入 module**（DOM append）：❌ 未执行（动态 module 的 module owner 未接线）
- RL 变体 type（classic/module）：✅ 识别执行

IPPure 页残余现象：app.js（parser src module）仍未 fetch。该页有 RL 注入的
重复 script 副本与 src 被破坏的 beacon module，NextScript 遍历与 ModuleSession
Load 的交互需逐点调试（下一轮）。动态 module 支持是独立的中型工程。


## 25. 关键背景更正：作者自己也在 mac 上卡 verdict（2026-11-03）

重新精读上游 truepeople README（作者 20261031 亲笔）：**作者在 upstream mac 上调
CF 托管质询同样未通过**——网络全 200、SW 健康、指纹变体（mac 基线/Windows 档案/
只换 navigator）全部无效，边缘不下发 clearance；真 FF159 geckodriver 同出口跑同
质询也未通过（托管质询比 Turnstile demo 更难）。作者提出的方向与我们完全一致
（逐信号与真 FF 对照），并给出"接受派生面边界"的保留选项。**"作者有现成答案"
的假设不成立**——inner 的 Windows 能过靠 Windows 派生面天然自洽，不是清单磨的。
作者手里有的是：检测点经验、Windows 侧信号知识、以及与我们合作打磨 mac 派生面
的动机（他的 macos 分支就是 PR#8）。

本轮 ES module 缺口的修复价值因此更清晰：**IPPure 的企业版 Bot Management 风险
指数 + Turnstile 双模式 + reCAPTCHA 分数是一个现成的、可视化的、免费的 mac 环境
评分器**——引擎补上 module 执行后即可用它持续量化派生面的收敛进度。


### §23 调试中（同日深夜）：PrepareScript 的 module 条件已放宽，内联 module 仍未达

已修：PrepareScript 的 module 条件去掉 `src&&!src->empty()` 与 `parser` 限制
（内联 module 与动态 module 不再被 preparation 拒绝）。但实测 parser 内联
module（type="module" 无 src）仍不执行——断点在 PageLoad 循环之后
（ModuleSession::Load 未触发或更早）。下一步：在 page_load.cpp 的 Source/主
循环加临时插桩，打印每个 script 的 type/module 判定/deferred 归类，定位
内联 module 从 preparation 到 module 循环之间哪一环丢失。


### §24 补充：Rocket Loader 在引擎上的行为分析（2026-11-03）

IPPure 页面使用 Cloudflare Rocket Loader。实测引擎行为：
- rocket-loader.min.js → 200 加载 ✓
- 内联标记脚本（type="<hash>-text/javascript"）→ **执行** ✓（__VP_HASH_MAP__ 等已定义）
- **外链标记 module script（app.js）→ 不加载** ✗
- 页面最终状态：VitePress 静态骨架（Vue 未 mount、检测组件不存在）

RL 对 inline 脚本正常执行，但**外链 module script 的加载**依赖 RL 创建新的
`<script type="module" src>` 元素——这需要引擎支持**动态插入 module script**
的执行（当前不支持，详见 ESM 缺口分析）。

**修复路径**：实现动态插入 module script 的执行路径（module owner 接线），
或者让引擎在解析时直接识别 RL 标记类型并原生加载（跳过 RL 中转）。
后者更稳健——推荐在 `PrepareScript` 的 `rocket_loader_essence` 剥离后，
将 RL 标记的 module 走 `ModuleSession` 管线（src 由引擎 fetch）。


### §25 突破：ES module 管线全通，IPPure 解锁为客观评分器（2026-11-03）

**引擎侧（分支 `mac-api-surface-parity`，e32f5c2）**

- 动态插入 module script 四条路径全绿：inline / external（ModuleSession 按
  Source 报告的解析 URL 入 module map）/ RL 变体（同 URL 去重）/ 404 error
  事件；parser src + 内联 + 动态 import 回归无损。
- 根因备忘：`Dynamic()` 此前调 `Source` 不带 module 出参 → `Source` 对
  module 早退（设计如此，parser 路径单独路由）→ code 恒 null。修复 = 传
  module 出参 + `!code && failed` 时对元素 fire error + external 用
  filename 作 module map key（inline 用文档 URL，同 Firefox）。

**IPPure（ippure.com/cloudflare）实测（mac 引擎 + DC 代理出口 209.126.8.142）**

| 项 | 结果 |
|---|---|
| VitePress/Vue 模块挂载 | ✓（RL 变体 module 原生加载，§24 的推荐路径已落地） |
| Cloudflare 风险指数 | ✓ 77-89 / 100（多次）——iframe→postMessage→HMAC/AES 签名网关全链路 |
| reCAPTCHA v3 | ✓ 0.3-0.7 / 1.0（Google 客观分，轮次波动）|
| Turnstile 渲染 + 执行 | ✓ render ×2（closed shadow root 内 iframe，rch 200，fo POST 200）|
| Turnstile verdict | ✗ NonInteractive error-callback = **600010**（put_resource 替身 api.js 抓到），Managed 停在等待激活 |

600010 与 ScrapingCourse §22 的 DECISIVE CONTROL 结论一致：同一出口真
FF159 headed 通过，差距在本引擎挑战面（已知派生面剩余：audio 表生成
ULP、Skia stroke AA），不在 IP/网络。

**排查中排除项**（全部本地复现验证正常，勿再怀疑）：

- 跨域子环境 GET/POST（同源经 necko helper，含 parse 期）
- MessageChannel 跨 frame transfer + 双向 port 投递
- postMessage origin 盖章、event.source 身份
- meta CSP nonce 执行、`worker-src blob:`、closed shadow root 内 iframe
  加载（含 detached 宿主 + turnstile 精确属性序列）

**工具沉淀**：`Context.put_resource` 可整体替身第三方 JS（本次给
challenges.cloudflare.com/turnstile/v0/api.js 注仪表抓到 600010），是
后续挑战面调试的标准手法。example：`python -m ruyijs.examples ippure`。


### §26 挑战内部直测：600010 前的最后一环是超时 PoW（2026-11-03）

调试基建（调试构建，均为惰性生效）：
- **传输层记账**：NeckoHttp 对 challenges.cloudflare.com 打请求/响应 body（截 1800B）→ `/tmp/probe/ns_trace.txt`
- **HTML 注入钩子**：NeckoHttp 对挑战文档注入 `/tmp/probe/inject_rch.js`（复用文档 CSP nonce——meta 是 `&#39;nonce-…&#39;` 转义形式，须按 `nonce-` + 标识符段提取）、对 ippure.com 注入 `/tmp/probe/inject_page.js` 收集器；探针数据走 `parent.postMessage` → `window.__tsProbe` → Python 读
- **挂起看门狗**：JS_AddInterruptCallback + 看门狗线程，advance 挂起 8s 后在自旋帧内抓 `JS::DescribeScriptedCaller`（AutoFilename + line + ColumnNumberOneOrigin）→ `/tmp/probe/hang_stack.txt`

**测量结果**（ippure.com/cloudflare，DC 代理出口）：

| 阶段 | 结果 |
|---|---|
| rch 引导文档 ×2 | 200，注入探针执行 ✓ |
| 内部握手 | widget 自发 `meow` seq=1→2 收发 ✓ |
| fo POST 两轮 | 200，挑战程序 878KB×2 下发 ✓ |
| 后续挑战往返 | `/g/pat/` → **401**；`/g/ci/` → 200；二轮 fo → 200（125KB）|
| 挑战程序执行 | **活跃计算**（自旋列号持续移动：403279→259131→414004→617400…，为控制流扁平化状态机 + 位运算哈希例程）|
| PoW 时长 | >105s 未完，引擎被 rpc 超时回收 |

**排除项**（本地复现全通过）：子环境 blob Worker 收发、跨 frame port、引擎 JIT 吞吐
（同型位运算哈希基准：引擎 733ms ≈ Node 740ms）。

**结论**：非死锁、非引擎慢 —— CF 给本环境派了超长 PoW（可疑环境 → 高难度
tarpit），与 600010 同根：**环境评分低 → 挑战加压**。修派生面（audio 表生成
ULP、Skia stroke AA）仍是正路；FF159 对拍（Phase 3）可进一步确证难度差。


### §27 Phase 3 对拍反转：长 PoW 是出口 IP 派的，但引擎算不完它（2026-11-03 晚）

FF159 headless（同一 vendor 159.0、同一 clash 出口，本地 TCP 中继逐连接记账
`/tmp/probe/relay_log.txt`）跑 ippure.com/cloudflare：

| 时刻 | 事件 |
|---|---|
| 5.3s | ippure 挂载，challenges/999831/123169 三域并发 |
| 10-12s | hagen 心跳，首轮小交换 |
| **12s → 177s** | **同款静默 PoW（165 秒）** |
| 177-183s | 解题完成：challenges 连接累计 1.9MB、cf.999831 / api.123169 关闭（gateway 成功）|
| 302s+ | 二轮验证流量（Managed 卡继续） |

引擎同出口对照：挑战程序下发后，自旋列号在一组固定站点间循环
（33763/230855 占 42%），**600s+ 不终止**；时钟循环本地验证正常
（perf/Date 精确 2000ms 退出）；子环境 blob Worker 正常；JIT 基准≈Node。

**修正 §26 结论**：长 PoW 非环境分派发（真 FF 同样要算 165s）——是**出口
IP 信誉**（209.126.8.142 DC 段）对所有人加压。引擎的差距变为：**挑战的
计算循环在引擎里推不到终止条件**（FF 165s 完成 vs 引擎 600s+ 循环），
根因在其位读取+哈希混合循环的某个输入源（字节源/时钟/PRNG）与 FF 存在
可观测差异。30 分钟后台马拉松判定中（完成 → 慢 3x；永不 → 退出条件损坏）。

补充排除（同日）：引擎 `Math.random` 与 `crypto.getRandomValues` 跨 Context
均非确定（排除「确定性 PRNG 使 PoW 搜索走重复候选」假设）；perf/Date 驱动
循环精确退出（排除「时钟网格使循环永不到期」假设）。挑战循环的输入源差异
待深入：候选 = 位读取字节源（异步流/Worker 回传）或挑战内部校准读数。


### §28 派生面收敛：701 个缺失成员清零 + BufferSource 修复（2026-11-03 深夜）

**表面成员层（`src/runtime/surface_parity.cpp`，生成器 `/tmp/probe/gen_surface.py`）**

- FF159 headless 逐接口测量（成员名 + 访问器类型 + 常量值，701 项/107 接口）
  → 生成 presence 层：仅当名字不是 own key 时定义，子系统真成员永远优先。
  主 realm 与 child-frame resource port 都安装。
- 复测：**missing 701 → 0**（extra 13 为 own-vs-inherited 摆放差异与
  headless profile 禁 push 的测量伪影）。

**AudioBufferSourceNode（功能性 bug 修复，7858653）**

- 修复前：buffer source 从不注册为渲染源、无 DSP → 所有 buffer 驱动的
  离线渲染输出全零。
- 修复后：构造即注册；start(when, offset)/stop 落保留槽；ProcessNode 按
  playbackRate（经 AudioParam 模型取值，注意不是对象 id）、loop 环绕与
  目标采样率重采样播放；图发布 buffer 声道数。
- 对拍：buffer→destination 与 FF159 **逐样本一致（14030.585411）**；
  过 compressor 相对差 1.4e-5（剩余为压缩器 DSP 微差）。
- 教训：FF headless 的 startRendering 对 start/connect 顺序敏感（先
  connect 后 start 会挂死），**headless 不可作 audio 对拍基准**。

**canvas 分解结论**（getImageData 逐像素）：fillRect/strokeLine/gradient
**像素完全一致** —— Skia 解析光栅化移植正确；剩余两处为 ①fillText 字形
光栅化（mac 上 FF 走 CoreText）②PNG 编码器字节（同像素不同压缩）。


### §29 方法论落地：自旋期 API 消费全零 + 程序差异假设（2026-11-04）

**新增基建**：看门狗 0.5s 高频轨迹（`/tmp/probe/hang_trace.txt`，中断回调内
DescribeScriptedCaller + 顺带快照 widget 全局的 `__stats` 消费计数）；注入
探针 v2 包裹 Math.random/getRandomValues/时钟/Worker/fetch/eval。

**自旋期测量（IPPure，1550 次捕获 / 13 分钟）**：

- 轨迹形态：VM 解释器在固定调度点集合间循环（260272/224584 等 58 个站点，
  上跳 627 次/下跳 637 次）——非顺序字节流消费，是调度器空转
- **API 消费全零且恒定**：mathRandom=0、getRandomValues=0、dateNow=12
  （boot 期）、fetch/xhr=0 —— 自旋不是在搜索随机 nonce，是**纯内部状态
  循环**，即 VM 在等某个永不到来的内部事件
- widget 事件循环被自旋阻塞（探针的 5s 定时器永不触发）——计数只能靠
  引擎中断回调快照读出

**排除清单（本轮新增）**：GC/分配吞吐（引擎 78ms ≈ donor SM shell 69ms，
同引擎基线）；SAB/Atomics（引擎与 FF 非隔离页面同样无 SharedArrayBuffer）。

**新假设（程序差异）**：引擎与 FF 收到的 fo 挑战程序**字节数不同**（引擎
878476/878668 vs FF 858624/894456）——服务端按各自探测结果派发不同程序。
若 FF 的程序 165s 可解而引擎的程序无解/超难，则检测点在 **fo 交换之前的
探测阶段**。验证实验：FF headless 与引擎**并发**跑同出口，同刻对比双方
fo 请求/响应尺寸与轮次。次选：静态去扁平化挑战 VM 读退出条件。


### §30 Babel AST 还原 + 长跑数据（2026-11-03）

**AST 反混淆管线**（`debug_tmp/deob/`，Babel + 自研折叠器）：
- 文本级种子替换（`_0x7bac9c` = 454874323 = `(a=>a).length+454874322`）
- 上下文感知常量折叠器（Python 手写扫描器：字符串/模板 `${}`/正则/注释状态
  跟踪 + 调用括号上下文检查，4308 处折叠）
- babel a0 解码（atob + 滚动 16 位 XOR，初始 29367，0xFF 双字节转义；
  **4608 个字符串全部解码**，'call'/'state' 等可读名出现）

**VM 结构确认**：rch 引导脚本 = 字节码解释器（`HT[HD++]` 取指、XOR 掩码
操作码、箭头函数表分发、栈深计数 `this['K']`、`lf`/`li` 步进函数）；
`ZQ` 为哈希累加器（'i'/'h'/'state'）；自旋期 API 消费全零。

**调度确认**：挑战以 `setTimeout(TE, 0/ Xg)` 分块自调度；chunk 内纯计算
不读时钟。bootstrap 含 `_cf_chl_opt`：`wjSe2: 72000`（72s 超时）、
`aUam7: 'non-interactive'`、sitekey、会话令牌。

**长跑数据**：引擎单 chunk 自旋 RSS 稳定 ~100-111MB（无 OOM、无泄漏）；
3600s 超时杀 Context 前未终止；FF 同程序 165s 完成。僵尸引擎进程曾未被
清理（pkill -9 只杀了 FF），多轮对比期间互相抢 CPU —— 已改按端口清杀。

**2 小时无人值守马拉松运行中**（`debug_tmp/marathon120.log`，Context
timeout=7200）——若完成则 600010 = "太慢"；若仍不终止则程序/调度差异实锤。
