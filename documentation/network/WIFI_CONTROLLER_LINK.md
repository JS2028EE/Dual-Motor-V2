# Dual Motor V2 — Wi-Fi Controller Link

## Purpose

This document defines the wireless connection between the ESP32-C3 handheld controller input board and the ESP32-S3 ES3C28P display board.

## Topology

```
ESP32-C3 SuperMini
Wi-Fi Station
      |
      | 2.4 GHz Wi-Fi
      |
ESP32-S3 ES3C28P
Wi-Fi Access Point
      |
      +-- TCP Server :4210
```

No external router or Internet connection is required.

## Network configuration

| Parameter | Value |
|---|---|
| SSID | `DUAL-MOTOR-V2` |
| Password | `DMV2-CTRL` |
| S3 AP IP | `192.168.4.1` |
| TCP port | `4210` |
| AP channel | 6 |
| Maximum intended controller clients | 1 |

The S3 starts the AP and TCP server during boot. The C3 joins the AP and then opens a TCP client connection to 192.168.4.1:4210.

## Message protocol

Messages are newline-terminated ASCII.

### C3 to S3

```
READY,C3
DEVICE,DUAL_MOTOR_V2_C3
INPUT,0,0,0,0
EVENT,NAV_SELECT
EVENT,NAV_BACK
EVENT,DRIVE_BRAKE
EVENT,DRIVE_ESTOP
PONG,C3
```

### S3 to C3

```
PING
IDENTIFY
```

## Input packet

`INPUT,navX,navY,driveX,driveY`

Each axis is normalized to approximately -1000 to +1000 with a configurable center deadzone.

Example:

```
INPUT,-412,87,0,931
```

## Connection monitoring

The C3 retries Wi-Fi and TCP connections when disconnected.

The S3 sends periodic PING messages and treats the controller as stale if no packet is received for approximately 1.5 seconds.

## Electrical relationship

Wi-Fi removes the need for the C3 and S3 to exchange controller data over their UART pins. The boards still need their own appropriate power connections.

The old UART wires may remain physically soldered, but GPIO20/GPIO21 on the C3 and GPIO43/GPIO44 on the S3 are not used by the current Wi-Fi firmware.

## Security note

This is a local project network, not an Internet service. The SSID/password are currently compiled into both controller sketches for the prototype. Change them in both sketches together if the network credentials are changed.
