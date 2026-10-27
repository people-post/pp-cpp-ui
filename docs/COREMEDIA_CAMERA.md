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
