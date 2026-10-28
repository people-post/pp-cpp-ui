# CoreMedia camera format preservation

Observed on iPhone 11: SDL reported the requested 480x360 at 30 FPS, but its delivered frames
were 1920x1080 at about 1 FPS. The first frame took about 5 seconds. QR decode took 4–5 ms.

The driver selects `activeFormat` before creating a session, then sets the session to High.
Apple documents that a session preset takes control of the device format. Configure the
selected format and frame duration together after adding the input, using InputPriority.

Native frame-duration diagnostics then showed 1 second for both bounds. The driver advertised
only the ends of continuous AVFrameRateRange values, so a requested 30 FPS picked 1 from a
1..60 range (29 away versus 30). Advertise common in-range rates, including the consumers'
20 and 30 FPS, in addition to endpoints.

Source: https://developer.apple.com/documentation/avfoundation/avcapturedevice/activeformat

Acceptance:

- [x] Compile the native driver for iOS and macOS through pp-browser Release builds.
- [x] Verify delivered dimensions match the selected low-resolution format on iPhone 11.
- [x] Measure preview FPS: native frame duration changed from 1,000,000,000 ns to 33,333,333 ns;
  actual delivery changed from ~1 FPS to ~30 FPS. The first one-second capture report contains
  20 frames; subsequent reports contain 30/31 per ~1 second. Exact first-frame timing is not logged.
- [ ] Verify QR identity confirmation on device (app fake-camera/confirmation tests pass).
- [ ] Check Mac camera capture; call behavior needs device coverage because the driver is shared.

No changes to pixel formats, rotation, permissions, or camera ownership are intended.

Independent static review found no confirmed regression. Actual call-camera and Mac capture
coverage remains pending. Current behavior is documented in [COREMEDIA_CAMERA.md](../COREMEDIA_CAMERA.md).
