/*
 Dual Motor V2 - LATCHED BLE TEST firmware, recorded October 17, 2026.
 Behavioral copy of the revised latched-control sketch shared in chat.
 This is TEST code, not a validated production controller.
 Original ESP32-WROOM-32D, Arduino-ESP32 core 3.x.
 GPIO25->L293D1, GPIO26->2, GPIO27->7;
 GPIO14->9, GPIO32->10, GPIO33->15.
 Pin8 motor +6 V, pin16 logic +5 V; all grounds 4/5/12/13 shared.
 Drive motor3/6; steering11/14.
 TWO separate 10k pull-downs: pin1->GND and pin9->GND.
 Remove fixed 5 V input jumpers before GPIO connection.
 BLE name DualMotorV2; Nordic UART service/RX/TX as defined below.
 Commands: forward, back, left, right, speed_0 ... speed_100, stop.
 Speed is integer; initial setting100. Startup motors remain stopped.
 Commands latch: releasing a button does not stop unless app sends stop/false.
 Drive PWM 1000Hz, 8bit; speed_30 is about30% DUTY, not measured RPM.
 Steering PWM is fixed128/255 (about50%), ALSO at1000Hz.
 stop disables BOTH bridges; intended emergency software command.
 BLE disconnect/unknown command/input overflow also stops motion.
 There is NO movement timeout or steering end-limit protection.
 Continuous steering can stall/heat; stop before pushing an end stop.
 Software stop is NOT a hardware emergency-stop/power-isolation device.
 Known low-duty noise and drive slowdown during steering are unresolved.
 No ultrasonic/GPS/lights/autonomous functions in this test sketch.
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
struct Command { Action action; int speed; };
std::string trimmed(std::string s);
Command parseCommand(std::string s);
void handle(Command c);

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
  for (char &c : s) if (c == '_' || c == ':' || c == '=') c = ' ';
  s = trimmed(s);
  auto split = s.find_first_of(" \t");
  std::string key = split == std::string::npos ? s : s.substr(0, split);
  std::string value = split == std::string::npos ? "" : trimmed(s.substr(split));
  if (key == "speed") {
    if (value.empty() || value.size()>3) return {INVALID,0};
    for (char c : value) if (c<'0' || c>'9') return {INVALID,0};
    int n=std::atoi(value.c_str());
    if (n>100) return {INVALID,0};
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
constexpr uint32_t REVERSE_DELAY_MS=50;
constexpr char SERVICE[]="6e400001-b5a3-f393-e0a9-e50e24dcca9e";
constexpr char RX_UUID[]="6e400002-b5a3-f393-e0a9-e50e24dcca9e";
constexpr char TX_UUID[]="6e400003-b5a3-f393-e0a9-e50e24dcca9e";

struct Packet { uint32_t generation; char text[97]; };
QueueHandle_t commands;
std::atomic<bool> connected{false}, overflowed{false}, restartAd{false};
std::atomic<uint32_t> generation{0};
BLECharacteristic *statusChar=nullptr;
int speedPercent=100;
int driveRequested=0, driveApplied=0;
int steerRequested=0, steerApplied=0;
uint32_t driveReadyAt=0, steerReadyAt=0;

void stopSteering() {
  ledcWrite(STEER_EN,0);
  digitalWrite(STEER_A,LOW); digitalWrite(STEER_B,LOW);
  steerRequested=steerApplied=0;
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
        p.generation=generation.load();
        line.copy(p.text,line.size());
        if (xQueueSend(commands,&p,0)!=pdTRUE) overflowed.store(true);
      }
      if (end==std::string::npos) break;
      start=end+1;
    }
  }
};

void handle(Command c) {
  if (c.action==INVALID || c.action==STOP) {
    stopAll(); report(c.action==STOP ? "stopped" : "unknown command: stopped"); return;
  }
  if (c.action==SPEED) {
    speedPercent=c.speed;
    if (speedPercent==0) stopAll();
    Serial.printf("speed_%d\n",speedPercent);
    return;
  }
  if (c.action==FORWARD || c.action==BACK) {
    driveRequested=speedPercent>0 ? (c.action==FORWARD ? 1 : -1) : 0;
    report(c.action==FORWARD ? "forward" : "back");
    return;
  }
  steerRequested=speedPercent>0 ? (c.action==LEFT ? -1 : 1) : 0;
  report(c.action==LEFT ? "left" : "right");
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
  statusChar->setValue("ready: speed_100; motors stopped");
  BLECharacteristic *rx=service->createCharacteristic(RX_UUID,
      BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
  rx->setCallbacks(new Receiver());
  service->start();
  BLEAdvertising *ad=BLEDevice::getAdvertising();
  ad->addServiceUUID(SERVICE); ad->setScanResponse(true); ad->start();
  Serial.println("BLE DualMotorV2 ready. Speed100; motors stopped. Core3.x.");
}

void loop() {
  uint32_t now=millis();
  static uint32_t seenGeneration=0;
  uint32_t current=generation.load();
  if (current!=seenGeneration || !connected.load()) {
    stopAll(); speedPercent=100; seenGeneration=current;
  }
  if (overflowed.exchange(false)) {
    stopAll(); speedPercent=100; xQueueReset(commands);
    report("input overflow: stopped");
  }
  Packet p;
  while (xQueueReceive(commands,&p,0)==pdTRUE) {
    now=millis();
    if (!connected.load() || p.generation!=generation.load()) continue;
    Serial.print("rx: "); Serial.println(p.text);
    handle(parseCommand(p.text));
  }
  if (!connected.load() || generation.load()!=seenGeneration) {
    stopAll(); speedPercent=100;
  } else {
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
    if (steerApplied!=steerRequested) {
      ledcWrite(STEER_EN,0);
      digitalWrite(STEER_A,LOW); digitalWrite(STEER_B,LOW);
      steerApplied=steerRequested; steerReadyAt=now+REVERSE_DELAY_MS;
    }
    if (steerApplied && int32_t(now-steerReadyAt)>=0) {
      digitalWrite(STEER_A,steerApplied<0 ? HIGH : LOW);
      digitalWrite(STEER_B,steerApplied>0 ? HIGH : LOW);
      ledcWrite(STEER_EN,128);
    }
  }
  if (restartAd.exchange(false)) BLEDevice::startAdvertising();
  delay(5);
}
