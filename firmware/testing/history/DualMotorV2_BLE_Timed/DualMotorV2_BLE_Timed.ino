/*
 Dual Motor V2 - BLE bench-control firmware
 Target: original ESP32-WROOM-32D, Arduino-ESP32 core 3.x.
 Uses BLE/FreeRTOS supplied with that core; no extra libraries.

 ESP32 GPIO -> L293D physical pin:
 25 -> 1 (drive enable), 26 -> 2, 27 -> 7
 14 -> 9 (steer enable), 32 -> 10, 33 -> 15
 L293D pin 8: motor boost +6 V. Pin 16: regulated +5 V.
 All four grounds (4,5,12,13) join common load ground.
 Motor leads: drive 3/6; steering 11/14.
 TWO pull-downs: 10k pin 1 to GND AND 10k pin 9 to GND.
 Remove every fixed 5 V control jumper before GPIO connection.

 BLE name: DualMotorV2
 Nordic UART service 6e400001-b5a3-f393-e0a9-e50e24dcca9e
 App writes UTF-8 text to RX 6e400002-b5a3-f393-e0a9-e50e24dcca9e
 Optional status NOTIFY: 6e400003-b5a3-f393-e0a9-e50e24dcca9e
 One command per BLE write or newline-separated complete commands.
 Commands: forward, back, left, right, stop, speed:50.0
 Also accepts speed=50.0, speed 50.0, {"speed":50.0},
 {"command":"forward"}, and {"forward":true}/{"forward":false}.
 Out-of-range/nonfinite speed values are rejected; speed 0 stops both.
 Initial speed is 0; set speed then send a direction.

 Forward/back must repeat every 200 ms while held; timeout 700 ms.
 Configure button release to send stop, if supported by your app.
 Left/right are 150 ms steering pulses at 50% duty, max one per second.
 Steering does NOT know its angle or automatically center.
 Disconnect resets motion and speed to zero. Unknown commands stop motion.
 No authentication: close-range supervised bench tests only.
 No GPS, ultrasonic avoidance, lights, or autonomous mode in this sketch.
 Earlier unintended motor power path remains unresolved: software does
 not guarantee hardware isolation. First test with wheels lifted.
*/

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <atomic>
#include <string>
#include <cstdlib>
#include <cmath>
#include <cctype>
#include <initializer_list>

// BEGIN COMMAND PARSER (host-tested independently)
enum Action { INVALID, FORWARD, BACK, LEFT, RIGHT, STOP, SPEED };
struct Command { Action action; float speed; };
std::string trimmed(std::string s);
Command parseCommand(std::string s);
void handle(Command c, uint32_t receivedAt, uint32_t now);

std::string trimmed(std::string s) {
  const auto a = s.find_first_not_of(" \t\r\n");
  if (a == std::string::npos) return "";
  return s.substr(a, s.find_last_not_of(" \t\r\n") - a + 1);
}

Command parseCommand(std::string s) {
  s = trimmed(s);
  if (s.empty() || s.size() > 96) return {INVALID, 0};
  if (s.front() == '{') {
    if (s.back() != '}') return {INVALID, 0};
    s = trimmed(s.substr(1, s.size()-2));
    // Only a single key/value object, not arbitrary JSON.
    if (s.find(',') != std::string::npos) return {INVALID, 0};
    std::string clean;
    for (char c : s) if (c != '"') clean += c;
    s = clean;
  }
  for (char &c : s) if (c == ':' || c == '=') c = ' ';
  s = trimmed(s);
  auto split = s.find_first_of(" \t");
  std::string key = split == std::string::npos ? s : s.substr(0, split);
  std::string value = split == std::string::npos ? "" : trimmed(s.substr(split));
  if (key == "speed") {
    char *end = nullptr;
    float n = std::strtof(value.c_str(), &end);
    if (value.empty() || end == value.c_str() || *end != '\0' ||
        !std::isfinite(n) || n < 0 || n > 100) return {INVALID, 0};
    return {SPEED, n};
  }
  if (key == "command") { key = value; value = ""; }
  Action a = INVALID;
  if (key == "forward") a = FORWARD;
  else if (key == "back") a = BACK;
  else if (key == "left") a = LEFT;
  else if (key == "right") a = RIGHT;
  else if (key == "stop") a = STOP;
  if (a == INVALID) return {INVALID, 0};
  if (value == "false" || value == "0") return {STOP, 0};
  if (!value.empty() && value != "true" && value != "1") return {INVALID, 0};
  return {a, 0};
}
// END COMMAND PARSER

constexpr uint8_t DRIVE_EN=25, DRIVE_A=26, DRIVE_B=27;
constexpr uint8_t STEER_EN=14, STEER_A=32, STEER_B=33;
constexpr uint32_t DRIVE_TIMEOUT_MS=700, STEER_PULSE_MS=150;
constexpr uint32_t STEER_GAP_MS=1000, REVERSE_DELAY_MS=50;
constexpr char SERVICE[]="6e400001-b5a3-f393-e0a9-e50e24dcca9e";
constexpr char RX_UUID[]="6e400002-b5a3-f393-e0a9-e50e24dcca9e";
constexpr char TX_UUID[]="6e400003-b5a3-f393-e0a9-e50e24dcca9e";

struct Packet { uint32_t generation, receivedAt; char text[97]; };
QueueHandle_t commands;
std::atomic<bool> connected{false}, overflowed{false}, restartAd{false};
std::atomic<uint32_t> generation{0};
BLECharacteristic *statusChar=nullptr;
float speedPercent=0;
int driveRequested=0, driveApplied=0;
uint32_t lastDriveAt=0, driveReadyAt=0, steerStartedAt=0, lastSteerAt=0;
bool steering=false, everSteered=false;

void stopSteering() {
  ledcWrite(STEER_EN,0);
  digitalWrite(STEER_A,LOW); digitalWrite(STEER_B,LOW);
  steering=false;
}
void stopDrive() {
  ledcWrite(DRIVE_EN,0);
  digitalWrite(DRIVE_A,LOW); digitalWrite(DRIVE_B,LOW);
  driveRequested=driveApplied=0;
}
void stopAll() { stopDrive(); stopSteering(); }
void report(const char *s) {
  Serial.println(s);
  if (connected.load() && statusChar) {
    statusChar->setValue(s); statusChar->notify();
  }
}

class ServerEvents : public BLEServerCallbacks {
  void onConnect(BLEServer *) override {
    generation.fetch_add(1); connected.store(true);
  }
  void onDisconnect(BLEServer *) override {
    connected.store(false); generation.fetch_add(1); restartAd.store(true);
  }
};
class Receiver : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *c) override {
    auto bytes=c->getValue();
    std::string raw(bytes.c_str(), bytes.length());
    if (raw.empty()) return;
    if (raw.size()>192 || raw.find('\0')!=std::string::npos) {
      overflowed.store(true); return;
    }
    size_t start=0;
    while (start<raw.size()) {
      auto end=raw.find('\n',start);
      std::string line=trimmed(raw.substr(start,end==std::string::npos ? end : end-start));
      if (!line.empty()) {
        if (line.size()>96) { overflowed.store(true); return; }
        Packet p{};
        p.generation=generation.load(); p.receivedAt=millis();
        line.copy(p.text,line.size());
        if (xQueueSend(commands,&p,0)!=pdTRUE) overflowed.store(true);
      }
      if (end==std::string::npos) break;
      start=end+1;
    }
  }
};

void handle(Command c, uint32_t receivedAt, uint32_t now) {
  if (c.action==INVALID || c.action==STOP) {
    stopAll(); report(c.action==STOP ? "stopped" : "unknown command: stopped"); return;
  }
  if (c.action==SPEED) {
    speedPercent=c.speed;
    if (speedPercent==0) stopAll();
    Serial.printf("speed %.1f%%\n",speedPercent);
    return; // A slider update never renews the drive-command timeout.
  }
  if (c.action==FORWARD || c.action==BACK) {
    driveRequested=speedPercent>0 ? (c.action==FORWARD ? 1 : -1) : 0;
    lastDriveAt=receivedAt;
    return;
  }
  if (speedPercent==0) return;
  if (steering || (everSteered && now-lastSteerAt<STEER_GAP_MS)) return;
  digitalWrite(STEER_A,c.action==LEFT ? HIGH : LOW);
  digitalWrite(STEER_B,c.action==RIGHT ? HIGH : LOW);
  ledcWrite(STEER_EN,128); // Fixed 50% steering pulse; speed slider controls drive.
  steering=true; everSteered=true; steerStartedAt=lastSteerAt=now;
}

void setup() {
  for (uint8_t pin : {DRIVE_EN,DRIVE_A,DRIVE_B,STEER_EN,STEER_A,STEER_B}) {
    digitalWrite(pin,LOW); pinMode(pin,OUTPUT);
  }
  Serial.begin(115200);
  bool okDrive=ledcAttach(DRIVE_EN,1000,8);
  bool okSteer=ledcAttach(STEER_EN,1000,8);
  if (!okDrive || !okSteer) {
    digitalWrite(DRIVE_EN,LOW); digitalWrite(STEER_EN,LOW);
    Serial.println("PWM setup failed; motor power must stay OFF.");
    while (true) delay(100);
  }
  stopAll();
  commands=xQueueCreate(12,sizeof(Packet));
  if (!commands) { Serial.println("Queue allocation failed"); while(true) delay(100); }
  BLEDevice::init("DualMotorV2");
  BLEServer *server=BLEDevice::createServer();
  server->setCallbacks(new ServerEvents());
  BLEService *service=server->createService(SERVICE);
  statusChar=service->createCharacteristic(TX_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  statusChar->addDescriptor(new BLE2902());
  statusChar->setValue("ready: speed 0");
  BLECharacteristic *rx=service->createCharacteristic(RX_UUID,
      BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
  rx->setCallbacks(new Receiver());
  service->start();
  BLEAdvertising *ad=BLEDevice::getAdvertising();
  ad->addServiceUUID(SERVICE); ad->setScanResponse(true); ad->start();
  Serial.println("BLE DualMotorV2 ready. Set speed, then direction. Core 3.x.");
}

void loop() {
  uint32_t now=millis();
  static uint32_t seenGeneration=0;
  uint32_t current=generation.load();
  if (current!=seenGeneration || !connected.load()) {
    stopAll(); speedPercent=0; seenGeneration=current;
  }
  if (overflowed.exchange(false)) {
    stopAll(); speedPercent=0; xQueueReset(commands);
    report("input overflow: stopped; set speed again");
  }
  Packet p;
  while (xQueueReceive(commands,&p,0)==pdTRUE) {
    now=millis();
    if (!connected.load() || p.generation!=generation.load() || now-p.receivedAt>500) continue;
    Serial.print("rx: "); Serial.println(p.text);
    handle(parseCommand(p.text),p.receivedAt,now);
  }
  if (!connected.load() || generation.load()!=seenGeneration) {
    stopAll(); speedPercent=0;
  } else {
    if (driveRequested && now-lastDriveAt>=DRIVE_TIMEOUT_MS) stopDrive();
    if (steering && now-steerStartedAt>=STEER_PULSE_MS) stopSteering();
    if (driveApplied!=driveRequested) {
      ledcWrite(DRIVE_EN,0);
      digitalWrite(DRIVE_A,LOW); digitalWrite(DRIVE_B,LOW);
      driveApplied=driveRequested; driveReadyAt=now+REVERSE_DELAY_MS;
    }
    if (driveApplied && int32_t(now-driveReadyAt)>=0) {
      digitalWrite(DRIVE_A,driveApplied>0 ? HIGH : LOW);
      digitalWrite(DRIVE_B,driveApplied<0 ? HIGH : LOW);
      ledcWrite(DRIVE_EN,uint32_t(lroundf(speedPercent*255.0f/100.0f)));
    }
  }
  if (restartAd.exchange(false)) BLEDevice::startAdvertising();
  delay(5);
}
