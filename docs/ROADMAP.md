# Дорожная карта

## Phase 0 — Native foundation
- C++20, CMake, vcpkg.
- Win32 desktop shell.
- OpenXR runtime и D3D11 graphics binding.
- CI build и unit tests.

## Phase 1 — OpenXR virtual monitor
- [x] Instance and system initialization code.
- [x] D3D11 adapter selection using runtime LUID.
- [x] Session and VIEW reference space.
- [x] D3D11 swapchain and quad composition layer implementation.
- [ ] Verify on physical Meta Quest 3 through wired Link.
- [ ] Add size and position controls.

## Phase 2 — Window Capture
- Windows.Graphics.Capture picker for chosen HWND.
- Direct3D11 frame pool and GPU texture pipeline.
- Handle source resize, close and capture loss.
- Display simulator image on the XR panel.

## Phase 3 — OSD
- Element layout and settings.
- Position, size, color, opacity and visibility.
- JSON profiles and schema versioning.

## Phase 4 — Analog Signal Engine
- Noise and static.
- Thin/rare horizontal bands.
- Color distortion.
- Red/blue chromatic separation.
- Per-effect and master toggles.

## Phase 5 — Scenario Manager
- Save/load OSD and signal settings.
- Import/export training profiles.

## Phase 6 — Passthrough and classroom validation
- Validate Passthrough over Link.
- Check keyboard/controller visibility, comfort, thermal load and lesson-length stability.

## Phase 7 — Release
- Installer/package, user guide, hardware matrix, tests and release notes.
