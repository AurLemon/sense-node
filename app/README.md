# SenseNode Electron Client

Phase 1 desktop client for the SenseNode ESP32-S3 interaction node.

## Run

```powershell
pnpm install
pnpm start
```

If you are behind a local proxy, set these variables in the current PowerShell session before running `pnpm install`:

```powershell
$OutputEncoding = [Console]::OutputEncoding = [System.Text.UTF8Encoding]::new()
$env:HTTP_PROXY = "http://127.0.0.1:7890"
$env:HTTPS_PROXY = "http://127.0.0.1:7890"
$env:npm_config_proxy = "http://127.0.0.1:7890"
$env:npm_config_https_proxy = "http://127.0.0.1:7890"
$env:ELECTRON_GET_USE_PROXY = "1"
$env:ELECTRON_MIRROR = "https://npmmirror.com/mirrors/electron/"
```

If a native dependency was skipped or rebuilt incorrectly, rerun:

```powershell
pnpm rebuild better-sqlite3 @serialport/bindings-cpp --unsafe-perm
```

## Tips

- `better-sqlite3` is a native dependency used by the main process store, so it must be present at install time and in the final Electron package.
- `pnpm install` will trigger `postinstall`, which runs `pnpm rebuild better-sqlite3 --unsafe-perm`.
- If you clean dependencies on a new machine, run `pnpm install` first and then verify `better-sqlite3` is rebuilt before `pnpm make`.
- For packaging, the Electron Forge config keeps native modules unpacked so the final app can load `better-sqlite3` from `app.asar.unpacked`.

````

For non-dev verification:

```powershell
pnpm exec tsc --noEmit
pnpm lint
pnpm exec vite build --config vite.renderer.config.mts
````

`pnpm build` still maps to `electron-forge package`.

## Data Sources

- Serial: choose a port in the Serial tab, then connect at `115200`.
- WiFi: interface placeholder only.
- No demo or generated frames are used. If serial is not connected or has not emitted data, the UI shows `-` and `Device not connected`.

## JSON Lines

The firmware Demo mode emits one JSON object per line.

`sensor_frame` fields:

- `tof_mm`
- `accel.x/y/z`
- `gyro.x/y/z`
- `imu_label`
- `confidence`
- `hand_state`
- `motion_event`
- `fusion_state`
- `final_event`
- `inference_ms`
- `free_heap`
- `uptime_ms`

`warmup_info` fields:

- `sample_rate_hz`
- `window_frames`
- `window_fill`
- `model_labels`
- `free_heap`

The renderer keeps raw event data visible and applies a UI-only stabilizer for `stableEvent`.

## Tray

- Closing the main window hides it to tray.
- Minimizing keeps the window hidden to tray.
- The tray menu can show the main window, pause/resume serial, or quit.
- Only tray `Quit` exits the app.
