/*
 Dual Motor V2 - BLE + HC-SR04 obstacle-stop TEST firmware.
 Revision: latched drive, bounded steering pulses, front distance sensing.
 Original ESP32-WROOM-32D, Arduino-ESP32 core 3.x.
 Motor GPIO25->L293D1, GPIO26->2, GPIO27->7;
 GPIO14->L293D9, GPIO32->10, GPIO33->15.
 HC-SR04 TRIG->GPIO18; ECHO->GPIO19 through a 1k/2k voltage divider.
 This is TEST code, not a validated production controller.
 Pin8 motor +6 V, pin16 logic +5 V; all grounds 4/5/12/13 shared.
 Drive motor3/6; steering11/14.
 TWO separate 10k pull-downs: pin1->GND and pin9->GND.
 Remove fixed 5 V input jumpers before GPIO connection.
 BLE name DualMotorV2; Nordic UART service/RX/TX as defined below.
 Commands over BLE or USB Serial: forward, back, left, right, speed_0 ... speed_100, stop.
 Serial: 115200 baud, Newline; status/help/telemetry_on/telemetry_off.
 Logs show commanded outputs only, not measured voltage/current/angle.
 Speed is integer; initial setting100. Startup motors remain stopped.
 Drive commands latch. Left/right trigger one bounded pulse; no powered hold.
 Drive PWM 1000Hz, 8bit; speed_30 is about30% DUTY, not measured RPM.
 Steering PWM is fixed128/255 (about50%), ALSO at1000Hz.
 stop disables BOTH bridges; intended emergency software command.
 BLE disconnect/unknown command/input overflow also stops motion.
 There is NO drive timeout or position/end-limit feedback. Steering pulse is timed.
 A timed pulse can still reach a stop early: supervise and use stop as needed.
 Software stop is NOT a hardware emergency-stop/power-isolation device.
 Same-side repeated packets do not extend/retrigger a pulse until opposite or stop.
 No guarantee the wheels hold angle after steering power is removed.
 Continuous noise reduction reported at 150 ms; 200 ms travel/current results pending.
 HC-SR04 readings are reported over USB Serial; forward drive is blocked
 when the sensor reports <=20 cm, has no valid echo, or its reading is stale.
 Reverse and steering remain available. Clearing an obstacle does NOT restart
 the car; send a new forward command. This is not a certified safety system.
 No GPS, lights, or autonomous navigation functions are included.
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
constexpr uint32_t STEER_PULSE_MS=200; // Test 3: combined bench-source and longer steering pulse.
constexpr uint32_t STEER_DUTY=128; // Preserve previous ~50% steering PWM.

// HC-SR04 front sensor. GPIO19 MUST receive a level-shifted <=3.3 V Echo.
constexpr uint8_t ULTRASONIC_TRIG_PIN=18;
constexpr uint8_t ULTRASONIC_ECHO_PIN=19;
constexpr float OBSTACLE_STOP_CM=20.0f;
constexpr uint32_t ULTRASONIC_INTERVAL_MS=70;
constexpr uint32_t ULTRASONIC_TIMEOUT_US=30000;
constexpr uint32_t ULTRASONIC_STALE_MS=250;
volatile bool ultrasonicEchoArmed=false;
volatile bool ultrasonicEchoReady=false;
volatile uint32_t ultrasonicEchoRiseUs=0;
volatile uint32_t ultrasonicEchoPulseUs=0;
bool ultrasonicWaiting=false;
bool ultrasonicReadingValid=false;
bool ultrasonicObstacleDetected=false;
float ultrasonicDistanceCm=-1.0f;
uint32_t ultrasonicLastTriggerMs=0;
uint32_t ultrasonicTriggerStartUs=0;
uint32_t ultrasonicLastValidMs=0;

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
uint32_t steerStartedAt=0;
bool steerPulseActive=false;
int lastSteerCommand=0;
enum ControlSource { NO_SOURCE, USB_SOURCE, BLE_SOURCE };
ControlSource activeSource=NO_SOURCE;

void stopSteering() {
  ledcWrite(STEER_EN,0);
  digitalWrite(STEER_A,LOW); digitalWrite(STEER_B,LOW);
  steerRequested=steerApplied=0;
  steerPulseActive=false;
}
void stopDrive() {
  ledcWrite(DRIVE_EN,0);
  digitalWrite(DRIVE_A,LOW); digitalWrite(DRIVE_B,LOW);
  driveRequested=driveApplied=0;
}
void stopAll() { stopDrive(); stopSteering(); lastSteerCommand=0; }
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

bool ultrasonicAllowsForward() {
  return ultrasonicReadingValid &&
         (millis()-ultrasonicLastValidMs)<=ULTRASONIC_STALE_MS &&
         !ultrasonicObstacleDetected;
}

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
    if (c.action==FORWARD && !ultrasonicAllowsForward()) {
      stopDrive();
      report(ultrasonicObstacleDetected
        ? "forward blocked: obstacle within 20 cm"
        : "forward blocked: ultrasonic reading unavailable or stale");
      return;
    }
    driveRequested=speedPercent>0 ? (c.action==FORWARD ? -1 : 1) : 0;
    report(c.action==FORWARD ? "forward" : "back");
    return;
  }
  if (speedPercent==0) return;
  int direction=c.action==LEFT ? 1 : -1;
  if (direction==lastSteerCommand) return;
  stopSteering();
  lastSteerCommand=direction;
  steerRequested=direction;
  report(c.action==LEFT ? "left pulse" : "right pulse");
}

void updateMotors(uint32_t now) {
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
    if (steerPulseActive && uint32_t(now-steerStartedAt)>=STEER_PULSE_MS) {
      stopSteering(); // Disable EN and clear steering direction outputs.
    }
    if (steerApplied!=steerRequested) {
      ledcWrite(STEER_EN,0);
      digitalWrite(STEER_A,LOW); digitalWrite(STEER_B,LOW);
      steerApplied=steerRequested; steerReadyAt=now+REVERSE_DELAY_MS;
    }
    if (steerApplied && int32_t(now-steerReadyAt)>=0) {
      digitalWrite(STEER_A,steerApplied<0 ? HIGH : LOW);
      digitalWrite(STEER_B,steerApplied>0 ? HIGH : LOW);
      if (!steerPulseActive) {
        steerStartedAt=now;
        steerPulseActive=true;
      }
      ledcWrite(STEER_EN,STEER_DUTY);
    }
}

void ultrasonicEchoISR();
void updateUltrasonic(uint32_t nowMs);

void setup() {
  pinMode(ULTRASONIC_TRIG_PIN,OUTPUT);
  digitalWrite(ULTRASONIC_TRIG_PIN,LOW);
  pinMode(ULTRASONIC_ECHO_PIN,INPUT);
  attachInterrupt(digitalPinToInterrupt(ULTRASONIC_ECHO_PIN),
                  ultrasonicEchoISR,CHANGE);
  ultrasonicLastTriggerMs=millis()-ULTRASONIC_INTERVAL_MS;

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
  Serial.println("BLE + USB Serial + HC-SR04 ready. 115200 baud, Newline. Motors stopped.");
  Serial.println("HC-SR04: TRIG GPIO18, ECHO GPIO19 via 1k/2k divider; stop threshold 20 cm.");
  Serial.println("Forward requires a fresh valid distance reading; reverse remains available.");
}

void IRAM_ATTR ultrasonicEchoISR() {
  if (!ultrasonicEchoArmed) return;
  if (digitalRead(ULTRASONIC_ECHO_PIN)==HIGH) {
    ultrasonicEchoRiseUs=micros();
  } else if (ultrasonicEchoRiseUs!=0) {
    ultrasonicEchoPulseUs=micros()-ultrasonicEchoRiseUs;
    ultrasonicEchoReady=true;
    ultrasonicEchoArmed=false;
    ultrasonicEchoRiseUs=0;
  }
}

void updateUltrasonic(uint32_t nowMs) {
  bool ready=false;
  uint32_t pulseWidthUs=0;
  noInterrupts();
  if (ultrasonicEchoReady) {
    ready=true;
    pulseWidthUs=ultrasonicEchoPulseUs;
    ultrasonicEchoReady=false;
  }
  interrupts();

  if (ready) {
    ultrasonicWaiting=false;
    if (pulseWidthUs>0 && pulseWidthUs<=24000) {
      ultrasonicDistanceCm=(pulseWidthUs*0.0343f)/2.0f;
      ultrasonicReadingValid=true;
      ultrasonicLastValidMs=nowMs;
      ultrasonicObstacleDetected=(ultrasonicDistanceCm<=OBSTACLE_STOP_CM);
      if (ultrasonicObstacleDetected && driveRequested==-1) {
        stopDrive();
        report("OBSTACLE: forward drive stopped");
      }
    } else {
      ultrasonicReadingValid=false;
      ultrasonicObstacleDetected=false;
      if (driveRequested==-1) {
        stopDrive();
        report("ultrasonic invalid reading: forward stopped");
      }
    }
  }

  if (ultrasonicWaiting &&
      uint32_t(micros()-ultrasonicTriggerStartUs)>=ULTRASONIC_TIMEOUT_US) {
    noInterrupts();
    ultrasonicEchoArmed=false;
    ultrasonicEchoRiseUs=0;
    interrupts();
    ultrasonicWaiting=false;
    ultrasonicReadingValid=false;
    ultrasonicObstacleDetected=false;
    if (driveRequested==-1) {
      stopDrive();
      report("ultrasonic timeout: forward stopped");
    }
  }

  if (!ultrasonicWaiting &&
      uint32_t(nowMs-ultrasonicLastTriggerMs)>=ULTRASONIC_INTERVAL_MS) {
    noInterrupts();
    ultrasonicEchoReady=false;
    ultrasonicEchoRiseUs=0;
    ultrasonicEchoArmed=true;
    interrupts();
    ultrasonicWaiting=true;
    ultrasonicTriggerStartUs=micros();
    ultrasonicLastTriggerMs=nowMs;
    digitalWrite(ULTRASONIC_TRIG_PIN,LOW);
    delayMicroseconds(2);
    digitalWrite(ULTRASONIC_TRIG_PIN,HIGH);
    delayMicroseconds(10);
    digitalWrite(ULTRASONIC_TRIG_PIN,LOW);
  }

  if (ultrasonicReadingValid &&
      uint32_t(nowMs-ultrasonicLastValidMs)>ULTRASONIC_STALE_MS) {
    ultrasonicReadingValid=false;
    if (driveRequested==-1) {
      stopDrive();
      report("ultrasonic reading stale: forward stopped");
    }
  }
}



// BEGIN SERIAL CONTROL
bool telemetryEnabled=false;
char serialLine[97]{};
size_t serialLength=0;
bool serialDiscard=false;

const char *sourceName() {
  return activeSource==USB_SOURCE ? "usb" :
         activeSource==BLE_SOURCE ? "ble" : "none";
}

void printStatus(uint32_t now) {
  uint32_t elapsed=steerPulseActive ? uint32_t(now-steerStartedAt) : 0;
  uint32_t remaining=steerPulseActive && elapsed<STEER_PULSE_MS ?
                     STEER_PULSE_MS-elapsed : 0;
  Serial.printf("t=%lu src=%s speed=%d drive=%d pwm=%lu steer=%d pwm=%lu remain=%lu ms\n",
    (unsigned long)now,sourceName(),speedPercent,driveApplied,
    (unsigned long)ledcRead(DRIVE_EN),steerApplied,
    (unsigned long)ledcRead(STEER_EN),(unsigned long)remaining);
  Serial.printf("pins D[%d,%d] S[%d,%d]; configured pulse=%lu ms\n",
    digitalRead(DRIVE_A),digitalRead(DRIVE_B),
    digitalRead(STEER_A),digitalRead(STEER_B),(unsigned long)STEER_PULSE_MS);
  if (ultrasonicReadingValid) {
    Serial.printf("ultrasonic=%.1f cm obstacle=%s forward=%s\n",
      ultrasonicDistanceCm, ultrasonicObstacleDetected ? "YES" : "NO",
      ultrasonicAllowsForward() ? "ALLOWED" : "BLOCKED");
  } else {
    Serial.println("ultrasonic=NO VALID READING; forward=BLOCKED");
  }
}

void dispatchCommand(const std::string &line, uint8_t source) {
  Command c=parseCommand(line);
  Serial.printf("t=%lu rx[%s]: %s\n",(unsigned long)millis(),
                source==USB_SOURCE ? "usb" : "ble",line.c_str());
  handle(c);
  activeSource=(c.action==STOP || c.action==INVALID ||
               (c.action==SPEED && c.speed==0)) ? NO_SOURCE : static_cast<ControlSource>(source);
}

void processSerialLine(const std::string &line) {
  if (line=="status") { printStatus(millis()); return; }
  if (line=="help") {
    Serial.println("forward back left right speed_0..speed_100 stop status telemetry_on telemetry_off");
    Serial.println("Forward is blocked unless HC-SR04 has a fresh reading above 20 cm.");
    return;
  }
  if (line=="telemetry_on" || line=="telemetry_off") {
    telemetryEnabled=line=="telemetry_on";
    Serial.println(telemetryEnabled ? "telemetry on: 250 ms" : "telemetry off");
    return;
  }
  dispatchCommand(line,USB_SOURCE);
}

void pollSerial() {
  // Bound work per loop so serial traffic cannot monopolize motor updates.
  for (unsigned n=0;n<64 && Serial.available()>0;n++) {
    int byte=Serial.read();
    if (byte<0) break;
    char c=char(byte);
    if (c=='\r' || c=='\n') {
      if (!serialDiscard && serialLength) {
        serialLine[serialLength]='\0';
        std::string line=trimmed(serialLine);
        if (!line.empty()) processSerialLine(line);
      }
      serialLength=0; serialDiscard=false;
    } else if (!serialDiscard) {
      if (c=='\0' || serialLength>=sizeof(serialLine)-1) {
        stopAll(); activeSource=NO_SOURCE;
        serialLength=0; serialDiscard=true;
        Serial.println("serial invalid/overflow: stopped; discard until newline");
      } else serialLine[serialLength++]=c;
    }
  }
}

void logOutputs(uint32_t now) {
  static int previousDrive=999,previousSteer=999;
  static uint32_t previousDrivePWM=999,previousSteerPWM=999,lastPrint=0;
  uint32_t dp=ledcRead(DRIVE_EN),sp=ledcRead(STEER_EN);
  bool changed=previousDrive!=driveApplied || previousSteer!=steerApplied ||
               previousDrivePWM!=dp || previousSteerPWM!=sp;
  if (changed || (telemetryEnabled && uint32_t(now-lastPrint)>=250)) {
    printStatus(now); lastPrint=now;
    previousDrive=driveApplied; previousSteer=steerApplied;
    previousDrivePWM=dp; previousSteerPWM=sp;
  }
}
// END SERIAL CONTROL

void loop() {
  uint32_t now=millis();
  static uint32_t seenGeneration=0;
  uint32_t current=generation.load();
  if (current!=seenGeneration) {
    stopAll(); activeSource=NO_SOURCE; speedPercent=100; seenGeneration=current;
    report(connected.load() ? "BLE connected: stopped" : "BLE disconnected: stopped");
  }
  if (overflowed.exchange(false)) {
    stopAll(); activeSource=NO_SOURCE; speedPercent=100; xQueueReset(commands);
    report("input overflow: stopped");
  }
  Packet p;
  while (xQueueReceive(commands,&p,0)==pdTRUE) {
    now=millis();
    if (!connected.load() || p.generation!=generation.load()) continue;
    dispatchCommand(p.text,BLE_SOURCE);
  }
  pollSerial();
  now=millis();
  updateUltrasonic(now);
  now=millis();
  if (generation.load()!=seenGeneration ||
      (activeSource==BLE_SOURCE && !connected.load())) {
    stopAll(); activeSource=NO_SOURCE; speedPercent=100;
  } else {
    updateMotors(now);
  }
  logOutputs(millis());
  if (restartAd.exchange(false)) BLEDevice::startAdvertising();
  delay(5);
}
