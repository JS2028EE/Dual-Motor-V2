"""Host-test actual parser/motor functions using mocked GPIO/PWM, not ESP32 BLE."""
from pathlib import Path
import subprocess
import tempfile
import sys

source = Path(sys.argv[1]).read_text()
parser = source.split('// BEGIN COMMAND PARSER (host-tested independently)')[1].split('// END COMMAND PARSER')[0]
stops = source[source.index('void stopSteering()'):source.index('void report(')]
logic = source[source.index('void handle(Command c) {'):source.index('void setup()')]
stubs = r'''
#include <cassert>
#include <iostream>
#include <map>
constexpr int DRIVE_EN=25,DRIVE_A=26,DRIVE_B=27,STEER_EN=14,STEER_A=32,STEER_B=33;
constexpr int LOW=0,HIGH=1;
constexpr uint32_t REVERSE_DELAY_MS=50,STEER_PULSE_MS=150,STEER_DUTY=128;
int speedPercent=100,driveRequested=0,driveApplied=0,steerRequested=0,steerApplied=0,lastSteerCommand=0;
uint32_t driveReadyAt=0,steerReadyAt=0,steerStartedAt=0;
bool steerPulseActive=false;
std::map<int,int> pwm,pins;
void ledcWrite(int p,int v){pwm[p]=v;}
void digitalWrite(int p,int v){pins[p]=v;}
void report(const char*){}
struct SerialMock {template<class... A>void printf(const char*,A...) {}} Serial;
'''
tests = r'''
int main(){
 assert(parseCommand("speed_30").speed==30);
 for(auto s:{"speed_101","speed_-1","speed_30.0","speed_nan"})assert(parseCommand(s).action==INVALID);
 handle(parseCommand("forward"));updateMotors(0);assert(pwm[DRIVE_EN]==0);
 updateMotors(50);assert(pwm[DRIVE_EN]==255 && pins[DRIVE_A]==LOW && pins[DRIVE_B]==HIGH);
 handle(parseCommand("left"));updateMotors(50);assert(pwm[STEER_EN]==0);
 updateMotors(100);assert(steerPulseActive && pwm[STEER_EN]==128);
 handle(parseCommand("left"));updateMotors(200);assert(steerStartedAt==100);
 updateMotors(249);assert(pwm[STEER_EN]==128);
 updateMotors(250);assert(!steerPulseActive && pwm[STEER_EN]==0 && pins[STEER_A]==0 && pins[STEER_B]==0);
 assert(driveRequested==-1 && pwm[DRIVE_EN]==255);
 handle(parseCommand("left"));updateMotors(500);assert(steerRequested==0 && pwm[STEER_EN]==0);
 handle(parseCommand("right"));updateMotors(500);updateMotors(550);assert(steerPulseActive && pins[STEER_B]==HIGH);
 handle(parseCommand("stop"));updateMotors(600);assert(pwm[STEER_EN]==0 && pwm[DRIVE_EN]==0);
 handle(parseCommand("left"));updateMotors(700);assert(steerRequested==-1);
 handle(parseCommand("stop"));updateMotors(800);assert(!steerPulseActive && pwm[STEER_EN]==0);
 handle(parseCommand("back"));updateMotors(800);updateMotors(850);assert(pins[DRIVE_A]==HIGH && pins[DRIVE_B]==LOW);
 handle(parseCommand("speed_0"));assert(pwm[DRIVE_EN]==0 && pwm[STEER_EN]==0);
 handle(parseCommand("left"));assert(steerRequested==0);
 handle(parseCommand("speed_100"));assert(driveRequested==0);
 // Check unsigned elapsed time across millis() rollover.
 stopAll();steerRequested=steerApplied=-1;lastSteerCommand=-1;
 steerPulseActive=true;steerStartedAt=UINT32_MAX-50;pwm[STEER_EN]=128;
 updateMotors(120);assert(!steerPulseActive && pwm[STEER_EN]==0);
 std::cout<<"PASS: reversed drive, latched drive, bounded steering, repeated-command guard, stop, speed zero, timer rollover.\n";
}
'''
with tempfile.TemporaryDirectory() as tmp:
    cpp = Path(tmp) / 'pulse.cpp'
    exe = Path(tmp) / 'pulse_test'
    cpp.write_text('#include <cstdint>\n#include <string>\n#include <cstdlib>\n#include <cmath>\n'+parser+stubs+stops+logic+tests)
    subprocess.run(['g++','-std=c++17','-Wall','-Wextra',str(cpp),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
