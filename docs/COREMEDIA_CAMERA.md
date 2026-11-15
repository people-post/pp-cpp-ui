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
A frame-duration configuration exception is logged and the camera still opens at the device
rate (macOS UVC cameras can reject a duration a hair outside their range); the properties above
show the rate that was actually applied.

Apple API contract: [activeFormat](https://developer.apple.com/documentation/avfoundation/avcapturedevice/activeformat).
Local patch: `third_party/sdl3/src/camera/coremedia/SDL_camera_coremedia.m` (SDL 3.4.16).

## Closing while a frame is in flight

Upstream SDL (3.4.16, and main as of this patch, where `ClosePhysicalCamera` still carries the
"close_cond" FIXME) lets `SDL_CameraThreadIterate` drop the device lock while it converts a frame.
On backends that deliver frames on their own thread (`ProvidesOwnCallbackThread`: CoreMedia, Android,
Emscripten) close has no thread to join, so it does not wait for that frame: a late frame then writes
into surfaces close has destroyed, or puts an output node into the lists of the next opening so the
filled list becomes a cycle and `SDL_AcquireCameraFrame` spins forever holding the device lock.
`-[AVCaptureSession stopRunning]` does not wait for a delegate callback that is already running
(measured on macOS), so nothing in CoreMedia's own close waits for callbacks either.

Local patch (re-apply when bumping SDL; the list is in `third_party/UPSTREAM.json`):

- Which backends take the new path: only those with `ProvidesOwnCallbackThread` (CoreMedia, Android,
  Emscripten). Backends with SDL's own camera thread (MediaFoundation, V4L2, PipeWire, dummy) keep
  upstream's locking in `SDL_CameraThreadIterate` unchanged (blocking lock, unlocked during
  conversion, re-locked to queue the frame): close joins their thread before it takes the lock, so
  they never had the race.
- Own-callback backends: `SDL_CameraThreadIterate` holds the device lock for the whole frame,
  conversion included, and marks a failed device lost while still holding it, so the loss applies
  only to that opening. Callbacks are kept out of a close by the device lock that
  `ClosePhysicalCamera` takes: it sets `shutdown` first, then takes the lock (which waits for a frame
  in progress), then calls the backend's `CloseDevice` and destroys the surfaces under the lock.
  Callbacks lock through `SDL_LockCameraUnlessShutdown`, which never queues on the lock: a callback
  either got the lock before close (close waits for that one frame) or sees `shutdown` and leaves. It
  also refuses a device that is not open (`hidden == NULL`).
- Lock order on the callback path: device lock, then the device-hash write lock (taken inside
  `SDL_CameraDisconnected` / `SDL_CameraPermissionOutcome` to queue their events). The rule that keeps
  this deadlock-free: never lock an *open* device while holding the device-hash lock. Destroying a
  device under that lock is fine because a destroyed device is already closed and
  `ClosePhysicalCamera` returns before locking it.
- On own-callback backends, destroying a device keeps the `SDL_Camera` struct and its lock allocated
  (about 0.5 KB, reclaimed at process exit) so a callback that still holds the pointer finds
  `shutdown` set. This is an intentional leak: leak checkers will report it from
  `DestroyCameraHashItem`; suppress it there (for example `leak:DestroyCameraHashItem` for
  LeakSanitizer, or `detect_leaks=0` in camera stress runs). Normal open/close never destroys a
  device; this happens when a lost device is closed, an open fails midway, the subsystem quits, or
  (Android) another app takes the camera.
- CoreMedia: the frame callback reads `device` and a strong reference to `hidden` in one
  `@synchronized(delegate)` section; `COREMEDIA_CloseDevice` clears `delegate.device` in that section
  before it releases `hidden`. Under the device lock the callback compares `device->hidden` with its
  own `hidden`; the pointer comparison is safe because the callback's strong reference keeps its
  `hidden` alive, so a new opening cannot reuse that address. A frame of a closed session therefore
  never reaches a reopened one. Permission handling and the zoom update (including
  `lockForConfiguration` on iOS) now run under the device lock. A missing sample is skipped instead of
  being passed to CoreMedia.
- Android (compiled by CI only, never run): `onImageAvailable` checks its reader under the lock (a
  pointer check; a new reader could reuse a deleted one's address, which would at worst mark the new
  opening lost); `onDisconnected` / `onError` use the same locking. `CameraPermissionCallback` is
  unchanged. Emscripten also takes the new path and has not been compiled.

### How to stress this

There is no automated test: the dummy driver delivers no frames. The real-camera stress program
(`camstress.c`) and the hook patches (`hooks-patched.patch` for this code, `hooks-unpatched.patch` for
untouched SDL) are kept in the app team's notes. The hooks are guarded by `PP_CAMERA_RACE_HOOKS`,
which the normal build never defines; they inject delays at each stage of the frame callback, a
backend frame failure, and counters for late or misdirected frames. Run under ASan with
`detect_leaks=0` (see the intentional leak above) and under TSan.

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
