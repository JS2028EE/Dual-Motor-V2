# Mistakes Encountered

Record mistakes with the original condition, cause, and impact. Do not silently erase history.

## 2026-09-22 — Display wipe and TCP reliability issues

### Symptoms
- A visible line/wipe appeared to erase the LCD while the UI refreshed.
- Joystick movement appeared delayed instead of continuously following the physical stick.
- The Wi-Fi/TCP controller link could drop after running for a short time.

### Root causes found
1. The S3 dynamic UI path redrew large display regions every 100 ms. Even without an explicit full-screen fill, the repeated panel redraw created a visible refresh/wipe effect.
2. Joystick telemetry was only sent every 50 ms, and the S3 display was only refreshed every 100 ms.
3. S3 used a blocking line reader (`readStringUntil('\\n')`), which is unnecessary for a high-rate control stream.
4. TCP was not explicitly configured for low-latency packets with `setNoDelay(true)`.
5. The C3 did not explicitly enable Wi-Fi auto-reconnect or immediately stop the TCP client when the Wi-Fi association was lost.

### Impact
The controller could feel visually laggy and the network link could appear to go offline even though the firmware had reconnect logic.
