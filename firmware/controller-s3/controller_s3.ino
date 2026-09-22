/*
 * Dual Motor V2 — ES3C28P Display Firmware
 * Target: ESP32-S3 ES3C28P, ILI9341V 240x320
 *
 * Landscape cyberpunk engineering console.
 * Touch is intentionally unused.
 *
 * UART: S3 GPIO43 RX <- C3 GPIO20 TX
 *       S3 GPIO44 TX -> C3 GPIO21 RX
 *
 * TFT_eSPI must be configured for the ES3C28P pins:
 * CS=10, DC=46, SCK=12, MOSI=11, MISO=13.
 */

#include <Arduino.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();
HardwareSerial ControllerSerial(1);

constexpr uint8_t UART_RX_PIN = 43;
constexpr uint8_t UART_TX_PIN = 44;
constexpr uint8_t TFT_BL_PIN = 45;

constexpr int SCREEN_W = 320;
constexpr int SCREEN_H = 240;

constexpr uint16_t BG = TFT_BLACK;
constexpr uint16_t PANEL = 0x10A2;
constexpr uint16_t GRID = 0x2945;
constexpr uint16_t CYAN = 0x07FF;
constexpr uint16_t MAGENTA = 0xF81F;
constexpr uint16_t GREEN = 0x07E0;
constexpr uint16_t YELLOW = 0xFFE0;
constexpr uint16_t WHITE = TFT_WHITE;

int navX = 0, navY = 0, driveX = 0, driveY = 0;
String lastEvent = "SYSTEM READY";
uint32_t lastPacketMs = 0;
uint32_t lastDrawMs = 0;

void panel(int x, int y, int w, int h, uint16_t accent) {
  tft.fillRoundRect(x, y, w, h, 6, PANEL);
  tft.drawRoundRect(x, y, w, h, 6, accent);
}

void crosshair(int cx, int cy, int x, int y, uint16_t color) {
  tft.drawCircle(cx, cy, 26, GRID);
  tft.drawFastHLine(cx - 30, cy, 60, GRID);
  tft.drawFastVLine(cx, cy - 30, 60, GRID);
  int px = cx + map(x, -1000, 1000, -22, 22);
  int py = cy + map(y, -1000, 1000, 22, -22);
  tft.fillCircle(px, py, 5, color);
}

void drawUI() {
  tft.fillScreen(BG);

  tft.drawFastHLine(0, 24, SCREEN_W, CYAN);
  tft.setTextColor(CYAN, BG);
  tft.setTextSize(2);
  tft.setCursor(8, 5);
  tft.print("DUAL MOTOR // V2");

  tft.setTextSize(1);
  tft.setTextColor(GREEN, BG);
  tft.setCursor(245, 9);
  tft.print("CTRL ONLINE");

  panel(6, 32, 150, 132, CYAN);
  tft.setTextColor(CYAN, PANEL);
  tft.setCursor(14, 40);
  tft.print("NAVIGATION");
  crosshair(81, 100, navX, navY, MAGENTA);
  tft.setTextColor(WHITE, PANEL);
  tft.setCursor(15, 140);
  tft.print("X:"); tft.print(navX);
  tft.setCursor(82, 140);
  tft.print("Y:"); tft.print(navY);

  panel(164, 32, 150, 132, MAGENTA);
  tft.setTextColor(MAGENTA, PANEL);
  tft.setCursor(172, 40);
  tft.print("DRIVE INPUT");
  crosshair(239, 100, driveX, driveY, CYAN);
  tft.setTextColor(WHITE, PANEL);
  tft.setCursor(173, 140);
  tft.print("X:"); tft.print(driveX);
  tft.setCursor(240, 140);
  tft.print("Y:"); tft.print(driveY);

  panel(6, 172, 308, 60, GRID);
  tft.setTextColor(YELLOW, PANEL);
  tft.setCursor(14, 180);
  tft.print("STATUS");

  tft.setTextColor(WHITE, PANEL);
  tft.setCursor(14, 195);
  tft.print("UART: ");
  tft.print((millis() - lastPacketMs < 500) ? "CONNECTED" : "WAITING");

  tft.setCursor(115, 195);
  tft.print("VEHICLE: NOT CONNECTED");

  tft.setCursor(14, 213);
  tft.print("EVENT: ");
  tft.print(lastEvent.substring(0, 38));

  tft.setTextColor(CYAN, PANEL);
  tft.setCursor(270, 213);
  tft.print("V2");
}

void handlePacket(String packet) {
  packet.trim();

  if (packet.startsWith("INPUT,")) {
    int p1 = packet.indexOf(',', 6);
    int p2 = packet.indexOf(',', p1 + 1);
    int p3 = packet.indexOf(',', p2 + 1);

    if (p1 > 0 && p2 > p1 && p3 > p2) {
      navX = packet.substring(6, p1).toInt();
      navY = packet.substring(p1 + 1, p2).toInt();
      driveX = packet.substring(p2 + 1, p3).toInt();
      driveY = packet.substring(p3 + 1).toInt();
      lastPacketMs = millis();
    }
  } else if (packet.startsWith("EVENT,")) {
    lastEvent = packet.substring(6);
    lastPacketMs = millis();
  } else if (packet == "READY,C3" || packet == "PONG,C3") {
    lastEvent = packet;
    lastPacketMs = millis();
  }
}

void setup() {
  ControllerSerial.begin(115200, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);

  // ES3C28P LCD backlight: GPIO45, HIGH = ON.
  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(BG);

  tft.setTextColor(CYAN, BG);
  tft.setTextSize(3);
  tft.setCursor(55, 80);
  tft.print("DUAL MOTOR V2");
  tft.setTextSize(1);
  tft.setCursor(102, 120);
  tft.print("CONTROLLER INITIALIZING");
  delay(900);

  ControllerSerial.println("PING");
  drawUI();
}

void loop() {
  while (ControllerSerial.available()) {
    String packet = ControllerSerial.readStringUntil('\n');
    handlePacket(packet);
  }

  if (millis() - lastDrawMs >= 100) {
    lastDrawMs = millis();
    drawUI();
  }
}
