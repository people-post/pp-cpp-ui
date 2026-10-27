# CoreMedia camera capture

The vendored SDL driver advertises common integer frame rates inside each continuous
AVFoundation frame-rate range, as well as its endpoints. This lets SDL's discrete format
negotiation select 20/30 FPS without incorrectly preferring 1 FPS from a 1..60 range.

On iOS, the selected `activeFormat` and frame durations are applied together after adding
the session input, within begin/commitConfiguration. InputPriority keeps a quality preset
from replacing the selected resolution. macOS retains its existing High preset.

The camera's SDL properties include `SDL.camera.coremedia.min_frame_ns` and
`SDL.camera.coremedia.max_frame_ns`, read after session startup. They report native device
configuration; SDL's public app-facing spec alone can differ from the native format.
Configuration exceptions fail open instead of silently retaining an unintended frame rate.

Apple API contract: [activeFormat](https://developer.apple.com/documentation/avfoundation/avcapturedevice/activeformat).
Local patch: `third_party/sdl3/src/camera/coremedia/SDL_camera_coremedia.m` (SDL 3.4.16).

## iOS rear-camera zoom

Properties returned by `SDL_GetCameraProperties(camera)` provide this opt-in interface:

| Key | SDL type | Contract |
| --- | --- | --- |
| `SDL.camera.coremedia.zoom_supported` | boolean | Read-only capability; true only on iOS rear cameras whose current native zoom range includes 1x and extends above 1x. |
| `SDL.camera.coremedia.max_zoom` | float | Read-only request ceiling: the lesser of the active format's maximum, currently available native maximum, and 4x. Defaults to 1 when unsupported. |
| `SDL.camera.coremedia.zoom` | float | Writable desired zoom factor; initialized to 1. Values are clamped to `[1, max_zoom]`; NaN/infinity request 1. It is a target, not measured/current native zoom. |
| `SDL.camera.coremedia.zoom_current` | float | Read-only actual `videoZoomFactor`, sampled on every capture-delegate frame even when the desired target is unchanged. Initialized/reset to 1 at open/close. |
| `SDL.camera.coremedia.zoom_ramping` | boolean | Read-only native `isRampingVideoZoom` sampled with `zoom_current`. Initialized/reset to false at open/close. |

macOS, tvOS, visionOS, front cameras and unsupported rear cameras publish
`zoom_supported=false`, `max_zoom=1`, `zoom=1`, `zoom_current=1`,
`zoom_ramping=false` and ignore zoom writes. Callers must
inspect support before offering controls. Values advertised at open are capability
snapshots; the driver also clamps each changed target to the live native range.

The serial capture delegate checks the target before delivering each frame, ignores
changes smaller than 0.01, and uses `rampToVideoZoomFactor:withRate:` at rate 1
(powers of two per second: approximately one second from 1x to 2x). The native
configuration lock is held only while issuing the request, never throughout the ramp.
`zoom_current` and `zoom_ramping` track the native transition rather than echoing its
target. Consumers calculating another target must use the actual factor and wait while
ramping or while the previous target has not yet been reached, to avoid compounding
uncompleted zoom steps.
Frame delivery must already be running; changing the property does not open a camera.
The owner must open/close cameras on its device worker, as for capture startup, rather
than on the UI thread; request writers only change SDL properties.

An eligible rear camera is reset to 1x when opened. Close disables further requests,
cancels any ramp and restores 1x so the next holder, including a call, does not inherit
scanner zoom. Reset/lock failures are logged; a reset failure during open aborts
opening the camera.
A per-camera monitor serializes native updates against close. The delegate reads the
SDL float property and releases its internal property lock before entering that
monitor or taking the AVFoundation configuration lock; neither open nor close writes
properties while holding the native lock or camera monitor. Feedback is published
as one properties-locked pair after both have been released; callers may use
`SDL_LockProperties` around both reads for a coherent snapshot. An atomic closed flag
is checked under that properties lock so a delegate's earlier native sample cannot
overwrite close's final 1/false feedback. These final values are lifecycle defaults;
a native reset failure is still reported through the existing log.

API availability and lock/range requirements were checked against Xcode's iPhoneOS
27.1 SDK `AVCaptureDevice.h` and Apple's documentation:
[rampToVideoZoomFactor:withRate:](https://developer.apple.com/documentation/avfoundation/avcapturedevice/ramp(tovideozoomfactor:withrate:)?language=objc),
[videoZoomFactor](https://developer.apple.com/documentation/avfoundation/avcapturedevice/videozoomfactor?language=objc).
Compilation verifies API use only; real-device smoothness, reset behavior and image
quality still require iOS hardware validation.
