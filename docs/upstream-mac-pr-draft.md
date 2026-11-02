# upstream-mac PR draft — working notes (not for commit)

Branch `upstream-mac`, all commits local. Push/PR timing: user decision.

## Proposed PR title

`mac: align the DOMException surface, CSS.supports grammars, the macOS profile, a property-access tracker, the truepeople example and runtime tests`

## Commit summary (4 thematic commits on top of 6d45317)

1. **c863d99** `port: align the DOMException surface, CSS.supports grammars and the macOS profile`
   - DOMException gains the six nsException members Firefox carries beside
     name/message/code: `filename`, `lineNumber`, `columnNumber`, `stack`,
     `result` (NS_ERROR_GENERATE_FAILURE(NS_ERROR_MODULE_DOM, code)), `data`
     (null). Throw site captured via `JS::DescribeScriptedCaller` +
     `JS::CaptureCurrentStack` into reserved slots 4–7.
   - `CSS.supports` answers the modern property grammars Firefox 159 ships
     (33 properties: accent-color, aspect-ratio, backdrop-filter,
     color-scheme, contain, container-type, field-sizing, filter, hyphens,
     image-rendering, mask-image, overflow-wrap, paint-order, place-items,
     rotate, scale, scrollbar-gutter, scrollbar-width, shape-outside,
     text-wrap, text-underline-offset, touch-action, translate, user-select,
     will-change, …). `text-wrap:pretty`, `line-clamp`, `interpolate-size`
     stay unsupported, as in Firefox 159. Inherited set + UA initial-value
     table gain the matching entries.
   - `profiles/firefox-environment-macos.json`: observed macOS baseline
     (rv:159.0 MacIntel, buildID 20251014000000, Apple M1 WebGL).
   - `tools/build.py --profile <name under profiles/>` so a platform build
     bakes its own baseline; default unchanged (`firefox-environment.json`).
2. **1fff7db** `port: add the RUYIJS_CF_ACCESS_LOG property-access tracker`
   - Off-by-default diagnostic: when the env var names a file, every logged
     host-object read is appended there (Model::Read field kinds, document
     hidden/visibilityState, CSS.supports checks, matchMedia queries).
   - `tools/signal_parity.py` collects the same surface from real Firefox
     159 via geckodriver for side-by-side comparison.
3. **4e82c0a** `port: add the truepeople managed-challenge example`
   - Cloudflare managed challenge end to end on the Necko stack; stage
     verdicts from net_log and page state only. Registers as
     `python -m ruyijs.examples truepeople`. Configures the mediaFeatures
     group to the preferences Firefox 159 answers on this host.
4. **3977c7a** `port: add the macOS runtime integration tests`
   - 17 in-process checks over the Python API (see tests file docstring).
   - `.gitignore` admits `/tests/` again (dropped in the sanitize pass;
     this set is plain API behaviour) and whitelists
     `tools/signal_parity.py`.

## Commit summary (7 thematic commits on top of 6d45317)

5. **681e17f** `port: shape macOS text with Core Text natively`
   - Real Firefox 159 on macOS shapes with Core Text; the HarfBuzz path
     misses system Helvetica's Apple AAT kern table and derives ink from
     DWRITE metrics. The proven CoreText line moves over behind the
     existing container (font_coretext.cpp, metrics_coretext.cpp,
     glyph_coretext.cpp, glyph_mask_coretext.cpp); the shaper's own files
     keep their Windows bodies under `#ifdef _WIN32`. font_preferences.cpp
     gains BrowserLanguage(), the reader paired with SetBrowserLanguage.
   - Parity after the move (`tools/shaping_parity.py` vs real Firefox 159
     via geckodriver): per-character advances exact for every Latin and
     explicit-CJK font, CJK/emoji fallback advances exact, Helvetica AAT
     kerning applies again, canvas fillText ink pixel-identical (CJK
     sample 835/835, same box).
6. **1a61ec0** `build: link libnecko_core against a configurable NSS directory`
   - `--necko-nss-lib` / `RUYIJS_NECKO_NSS_LIB`: a local browser objdir can
     ship the split NSS form (no SSL exports in libnss3, no libssl3 in
     dist/bin) — the release distribution merges SSL into libnss3.
7. **607ddf8** `tools: add the shaping parity probe`

## Verification (2026-11-01, macOS arm64, SDK 26.5, Mozilla clang, Firefox 159.0 donor)

- Donor build: `obj-necko` (browser, docs/macos.md mozconfig-necko) →
  `dist/bin/xpcshell` reports `JavaScript-C159.0`; `obj-js-standalone`
  (SpiderMonkey, jemalloc kept — `--disable-jemalloc` suppresses
  `libmozglue.dylib`, which `tools/donor.py` requires).
- Shell (CoreText shaping): `tools/build.py --donor-obj
  ../firefox-159/obj-js-standalone --firefox-gfx-obj ../firefox-159/obj-necko
  --firefox-font-prefs ../firefox-159/obj-necko/dist/bin/greprefs.js
  --necko-nss-lib <Firefox159.app>/Contents/MacOS` → `dist/js` 54 MB.
- `ci/smoke_test.py`: 10/10 ok (default Windows baseline profile).
- `tests/test_macos_runtime.py`: 17/17 PASS on the CoreText shell.
- `ci/network_smoke.py`: PASS.
- Runtime assets for direct HTTPS must be staged next to the shell
  (`dist/necko_tls/necko/`): `firefox-roots.pem`,
  `firefox-root-constraints.json`, `firefox-suffixes.dat`
  (source: `vendor/necko-standalone/necko/`), otherwise the first direct
  HTTPS request fails with `Invalid TLS CA certificate bundle path`.

## Known issue: the rebuilt libnecko_core stalls inside the helper session

Three layers, all established by experiment on the same tree:

1. **Link closure.** A local browser objdir's `libnss3.dylib` has no SSL
   exports (0 `SSL_*`) and dist/bin has no `libssl3`; the release
   distribution merges 59 `SSL_*` exports into libnss3. Linking against
   the objdir fails with undefined `_SSL_VersionRangeSet` — hence
   `--necko-nss-lib` (1a61ec0) pointing the link at the donor app's
   `Contents/MacOS`, which links cleanly.
2. **The library itself is sound.** Driving `necko.core_client.CoreClient`
   directly (same helper venv, same dylib, no shell) completes a full
   HTTP/2 exchange with example.com: ClientHello, SETTINGS, headers and
   body all round-trip (verified with read/write probes compiled into the
   dylib). `closed_route._ensure_necko_win()` also passes.
3. **The stall is a lock interaction inside the session process.** When
   the shell spawns the helper as `oneshot --session --timing-v3`, the
   process deadlocks: the main thread and a thread named `necko-http2`
   each park on `lock_PyThread_acquire_lock` (macOS `sample` shows both
   threads parked, ~1700 samples each, no progress). The session then
   never sees request frames and the shell reports the read timeout.
   With the dylib absent from `dist/necko_tls`, the helper falls back to
   xpcshell and every check passes — so the interaction is specific to
   the core-library path inside a session process. Direct (non-session)
   driving works, which points at the h2 pump thread's lock handshake
   under `python3.14` + ctypes callbacks as the suspect.

Local machines keep the vendored dylib (the engine's second lookup path
`dist/necko/` serves it). The repro for upstream is `ci/network_smoke.py`
plus the newly built dylib and one `sample` of the hung helper.

## Residual parity items (non-blocking)

- TextMetrics ink box sits ~1 CSS px left of Firefox's (systematic on
  every sample; ink *width* matches). Font-baseline boxes come from the
  HarfBuzz fallback formula (no BASE-table query), so `fontBoundingBox`
  em differs (Times 17 vs 19).
- `canvas ctx.letterSpacing` is not implemented (constant 0 px); whole
  string advances quantize 1/60 px off Firefox on some samples.

## Notes for review

- All C++ changes are pure additions on domains upstream did not touch
  since a626670 (verified blob-identical against the merge base).
- Porting kept upstream formatting/line endings byte-for-byte; one file
  (`media.cpp`) has mixed EOLs upstream and was edited byte-precisely.
- The tracker and `signal_parity.py` exist to support the follow-up
  shaping-parity work (HarfBuzz-mac vs CoreText vs real Firefox).
