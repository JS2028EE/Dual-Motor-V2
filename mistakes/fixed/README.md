# Fixed Mistakes

Record the original mistake, correction, and verification.

## 2026-09-22 — Live display and Wi-Fi/TCP hardening

### Corrections
- Replaced the normal large-area UI redraw with a local live crosshair redraw.
- S3 now refreshes joystick visuals every 30 ms (~33 Hz).
- C3 now sends joystick telemetry every 20 ms (50 Hz).
- Replaced the blocking S3 line reader with a byte-by-byte newline parser.
- Enabled TCP no-delay mode on both ends of the controller connection.
- Enabled C3 Wi-Fi auto-reconnect and stopped the TCP client when Wi-Fi is no longer associated.
- Increased the S3 stale-link timeout from 1.5 s to 2 s to avoid declaring a healthy link offline too aggressively.
- Added a TCP write-failure check so a failed send can force a reconnect path.

### Verification status
Firmware changes are committed to GitHub. Physical verification is still required: upload both sketches, move both joysticks continuously, and leave the controller powered for several minutes to verify live response and long-duration link stability.
