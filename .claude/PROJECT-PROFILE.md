# Project profile — SN50v3

Read by the ai-engineering harness (context pack, `dev-task`) and by agents before any work here.
Last refreshed: 2026-10-24 on main@c4f1bc5ddbbd43cddea16f6a4784a67d18acd98b by Claude (ai-engineering onboarding; no CI in the repo, build command taken from `README.md` and `Projects/Applications/DRAGINO-LRWAN-AT/Makefile`).
`[V]` verified · `[I]` inferred, confirm before relying on it.

## 1. What it is
Firmware for the Dragino SN50v3-LB/-NB LoRaWAN sensor node, forked from the vendor repo `dragino/SN50v3`. The node
runs on the LA66 module: an ASR6601 SoC (Cortex-M4F, ASR "tremo" SDK) with an SX126x radio — **not STM32**
(`build/make/common.mk` `HOST_ARCH := Cortex-M4F`, `Drivers/peripheral`, `tremo_loader.py`). [V]
This fork adds SHT-40 temperature/humidity support and a self-contained GCC/make build for AU915 (3 commits by
cclautert on top of vendor `2595a2a`). [V]

| Part | Path | Notes |
|---|---|---|
| Application | `Projects/Applications/DRAGINO-LRWAN-AT/` (`src/`, `inc/`, `cfg/gcc.ld`, `Makefile`, `project.uvprojx`) | main loop, AT commands (`command.c`), uplink payload (`lora_app.c`), keys/commissioning (`inc/Commissioning.h`) [V] |
| Sensor drivers | `Drivers/sensor/` | SHT2x/SHT3x/SHT4x (`sht40.c`), BH1750, `bsp.c` auto-detection, `I2C_sensor.c` [V] |
| SoC drivers / CMSIS | `Drivers/peripheral`, `Drivers/system`, `Drivers/CMSIS`, `Drivers/crypto` (prebuilt `libcrypto.a`) | vendor SDK [V] |
| LoRaWAN stack | `Middlewares/LoRa/` (LoRaMac-node, regions, sx126x radio) | vendor stack [V] |
| Build system | `build/make/*.mk`, `build/scripts/` (`tremo_loader.py` = UART flasher, Keil generators) | [V] |
| Bench sketch | `utils/SHT40_test/SHT40_test.ino` | Arduino/ESP32, not built by make [V] |
| Tests | — | **none** (no unit, no host-side tests, no CI) [V] |

## 2. Branches and remotes
- Remotes: `origin` = `https://github.com/dragino/SN50v3` (vendor, read-only for us); `fork` = `https://github.com/cclautert/SN50v3` (the user's fork). [V]
- **Integration branch (PRs target): `main`** on remote **`fork`** [V] — `fork/main` = `c4f1bc5` = local `main`; the harness contract uses `"repo": { "baseBranch": "main", "remote": "fork" }`.
- ⚠️ Local `main` tracks `origin/main` (vendor) and shows "ahead 3"; the SHT-40/AU915 commits are on `fork/main` and `fork/add-sht40-support` (same commit), not on the vendor. [V]
- No merge commits / PR history on the fork yet; the only merge is a vendor one (`26b35b4`). Branch naming: `add-<slug>` (one example). [I]
- Vendor branch `origin/SN50v3-LA66S` (LA66S variant) moves independently; not merged here. [V]
- No CI (`.github/` absent), no CLAUDE.md, no Docker. [V]

## 3. Commands (Windows `cmd.exe`, from the repo root — this is how the harness runs them)
| Check | Command | Notes |
|---|---|---|
| build | `cd Projects\Applications\DRAGINO-LRWAN-AT && make -j8 TREMO_SDK_PATH=../../.. TOOLCHAIN_PATH=C:/Users/crist/AppData/Local/Programs/arm-gnu-toolchain-14.2/bin/ && if not exist Make_out\DRAGINO-LRWAN-AT.bin exit /b 1` | GNU Make 4.4.1 (winget) + Arm GNU toolchain 14.2.1. ~35 s. Output `Make_out/DRAGINO-LRWAN-AT.{elf,bin,hex,map}` (all git-ignored). Region is fixed in the Makefile (`-DREGION_AU915`). The `.mk` files call `uname`/`mkdir -p`/`rm -rf`: they need Git's `usr\bin` on PATH (true on this host). Only warning: ld "LOAD segment with RWX permissions" (harmless with this linker script). Size at baseline: text 91748, data 592, bss 15492. [V] |
| unit | — | Not applicable: the repo has no tests yet (user decision 2026-10-24: profile now, tests later). [V] |
| keil | `project.uvprojx` (Keil MDK) | Vendor's primary build; Keil not installed here. Add new `.c` files to it too, or the Keil build breaks while make (wildcards) passes. [I] |

Never run: `make flash` (or the `tremo_loader.py ... flash` line that `make` prints at the end), anything with
`build/scripts/tremo_loader.py`, `tools/hex2bin`, `tools/FLM`, any serial port / COM port access, or OTA update tools.
Plain `make` (target `all`) only builds and echoes the flash command — it does not flash. [V]

## 4. Test baseline
`main@c4f1bc5, 2026-10-24`, measured by the harness in a `C:\wt` worktree (RUN-20261025T010849-f1fa40, contract
`TASK-SN50-baseline-20261024`, remote `fork`): build **PASS** (exit 0, `.bin` produced) · unit **not applicable — no
test suite exists** · gate Completed, fully verified [V]. Failing set: none.
⚠️ The only automated proof of a change here is "it compiles and links for AU915". Behaviour (sensor reads, payload
bytes, LoRaWAN join) is unverified until host-side tests exist; a reviewer must check payload/AT/region changes by reading.

## 5. Skills
No project skills: the repo has no `.claude/skills/`. Nothing is bound; agents use the global skills only.

## 6. Risk
The ai-engineering harness sets each change's tier from the files it touches (`harness/lib/risk.js`: these rows
on top of its defaults — CI/deploy workflows, Dockerfiles, compose files, `appsettings*`, `.env*` are high; docs are
low; anything else is medium). The tier raises the review depth (low quick · medium standard · high deep) and sets
the human approvals `harness merge check` asks for, bound to the exact commit. Solo repo: high asks for two people
and gets one — the harness reports the cap.
Human approvers: Cristiano

| Tier | Path | Why |
|---|---|---|
| high | `Projects/Applications/DRAGINO-LRWAN-AT/src/lora_app.c` | uplink payload layout: the server-side decoder depends on every byte |
| high | `Projects/Applications/DRAGINO-LRWAN-AT/src/command.c` | AT command set: provisioning and keys in the field |
| high | `Projects/Applications/DRAGINO-LRWAN-AT/inc/**` | `Commissioning.h` (keys/EUIs), `lora_config.h`, `version.h` |
| high | `Projects/Applications/DRAGINO-LRWAN-AT/Makefile` | LoRaWAN region (`-DREGION_AU915`) and build flags |
| high | `Projects/Applications/DRAGINO-LRWAN-AT/cfg/**` | linker script: app at `0x0800D000`; overlapping `0x08000000` bricks the Dragino bootloader |
| high | `Middlewares/LoRa/**` | LoRaWAN MAC, regions, crypto, radio: join, duty cycle and regulatory behaviour |
| high | `Drivers/sensor/**` | sensor detection and readings that feed the payload |
| high | `Drivers/crypto/**` | prebuilt crypto library |
| high | `build/**` | build rules and the UART flasher |
| high | `tools/**` | flashing / image tools |
| low | `utils/**` | bench sketches, not in the firmware image |
| low | `README.md` | docs |

## Contradictions found while onboarding
- The onboarding brief described this as "STM32" firmware: it is ASR6601 (LA66, Cortex-M4F) — see section 1. [V]
- README "Building from the command line" says `TREMO_SDK_PATH=<repo root>`; a relative `../../..` from the project dir works too (used above so the command is worktree-independent). Not a contradiction, a simplification. [V]
- Local `main` upstream is `origin/main` (vendor) although the work lives on `fork/main` — see section 2. [V]
- The harness context pack (`harness/lib/context.js`) compares the profile's commit with `origin/<base>`, i.e. the vendor here, not `fork/main`: its "ahead" warning is about the wrong remote for this repo. [V]
