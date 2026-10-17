"""Test USB input and mixed-source loop using actual sketch functions and mocks."""
from pathlib import Path
import subprocess, tempfile, sys
s=Path(sys.argv[1]).read_text()
dispatch_proto=s[s.index('void dispatchCommand('):].split('{',1)[0]+';'
parser=s.split('// BEGIN COMMAND PARSER (host-tested independently)')[1].split('// END COMMAND PARSER')[0]
stops=s[s.index('void stopSteering()'):s.index('void report(')]
logic=s[s.index('void handle(Command c) {'):s.index('void setup()')]
serial=s.split('// BEGIN SERIAL CONTROL')[1].split('// END SERIAL CONTROL')[0]
loop=s[s.index('void loop() {'):]
stubs=r"""
#include <cassert>
#include <cstdio>
#include <deque>
#include <map>
#include <atomic>
#include <iostream>
constexpr int DRIVE_EN=25,DRIVE_A=26,DRIVE_B=27,STEER_EN=14,STEER_A=32,STEER_B=33;
constexpr int LOW=0,HIGH=1,pdTRUE=1;
constexpr uint32_t REVERSE_DELAY_MS=50,STEER_PULSE_MS=150,STEER_DUTY=128;
int speedPercent=100,driveRequested=0,driveApplied=0,steerRequested=0,steerApplied=0,lastSteerCommand=0;
uint32_t driveReadyAt=0,steerReadyAt=0,steerStartedAt=0,fakeTime=0;
bool steerPulseActive=false;
enum ControlSource {NO_SOURCE,USB_SOURCE,BLE_SOURCE};
ControlSource activeSource=NO_SOURCE;
std::atomic<bool> connected{false},overflowed{false},restartAd{false};
std::atomic<uint32_t> generation{0};
struct Packet {uint32_t generation;char text[97];};
int commands=0;
std::deque<Packet> packets;
bool xQueueReceive(int,Packet*p,int){if(packets.empty())return false;*p=packets.front();packets.pop_front();return true;}
void xQueueReset(int){packets.clear();}
std::map<int,int> pwm,pins;
void ledcWrite(int p,int v){pwm[p]=v;}
uint32_t ledcRead(int p){return pwm[p];}
void digitalWrite(int p,int v){pins[p]=v;}
int digitalRead(int p){return pins[p];}
uint32_t millis(){return fakeTime;}
void delay(int){}
void report(const char*){}
struct BLEDevice {static void startAdvertising(){}};
struct SerialMock {
 std::deque<char> input;
 std::string output;
 int available(){return input.size();}
 int read(){char c=input.front();input.pop_front();return (unsigned char)c;}
 void feed(std::string s){for(char c:s)input.push_back(c);}
 void println(const char*s){output+=s;output+='\n';}
 template<class... A>void printf(const char*f,A... a){char b[512];std::snprintf(b,sizeof(b),f,a...);output+=b;}
} Serial;
"""
tests=r"""
int main(){
 Serial.feed("forward\n");loop();fakeTime=50;loop();
 assert(activeSource==USB_SOURCE && pwm[DRIVE_EN]==255 && !connected);
 fakeTime=500;loop();assert(pwm[DRIVE_EN]==255); // USB works without BLE.
 Serial.feed("status\r\n");loop();assert(driveRequested==-1);
 assert(Serial.output.find("not sensor readings")!=std::string::npos);
 Serial.feed("left\n");loop();fakeTime=550;loop();assert(pwm[STEER_EN]==128);
 fakeTime=700;loop();assert(pwm[STEER_EN]==0 && pwm[DRIVE_EN]==255);
 Serial.feed("speed_30\n");loop();assert(speedPercent==30 && pwm[DRIVE_EN]==77);
 Serial.feed("stop\n");loop();assert(pwm[DRIVE_EN]==0);
 Serial.feed("for");loop();assert(driveRequested==0);
 Serial.feed("ward\n");loop();assert(driveRequested==-1);
 Serial.feed(std::string(100,'x')+"\n");loop();loop();
 assert(driveRequested==0 && activeSource==NO_SOURCE);
 Serial.feed("back\n");loop();assert(driveRequested==1);
 Serial.feed("typo\n");loop();assert(driveRequested==0);
 Serial.feed("telemetry_on\n");loop();assert(telemetryEnabled);
 Serial.feed("telemetry_off\n");loop();assert(!telemetryEnabled);
 Serial.feed("forward\n");loop();generation=1;connected=true;loop();
 assert(driveRequested==0); // BLE connect stops an existing USB run.
 Packet p{};p.generation=1;std::snprintf(p.text,sizeof(p.text),"forward");packets.push_back(p);
 loop();fakeTime=800;loop();assert(activeSource==BLE_SOURCE && pwm[DRIVE_EN]==255);
 connected=false;generation=2;loop();assert(pwm[DRIVE_EN]==0);
 Serial.feed("back\n");loop();fakeTime=850;loop();
 assert(activeSource==USB_SOURCE && pwm[DRIVE_EN]==255);
 std::cout<<"PASS: offline USB, CRLF, fragmentation, status, pulse cutoff, speed, stop, overflow, BLE connection/disconnection and USB recovery.\n";
}
"""
with tempfile.TemporaryDirectory() as d:
 p=Path(d)/'serial.cpp';exe=Path(d)/'serial_test'
 p.write_text('#include <cstdint>\n#include <string>\n#include <cstdlib>\n#include <cmath>\n'+dispatch_proto+parser+stubs+stops+logic+serial+loop+tests)
 subprocess.run(['g++','-std=c++17','-Wall','-Wextra',str(p),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)
