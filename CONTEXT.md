# Phone-Controlled Car

The vocabulary of the `feat/phone-controlled-car` branch: a small car that a person
drives from a phone, with a single frontal sensor that can refuse motion but never
steer it.

## Platform

**Car**:
A four-wheeled platform with one rear drive motor and one front steering servo. It
cannot rotate in place — every movement is a turn with a radius.
_Avoid_: Robot, rover, platform

**Requested throttle**:
What the user is asking for, as sent from the phone: -100 (full reverse) through 0
(stop) to +100 (full forward).
_Avoid_: Speed, throttle level, target speed

**Actual throttle**:
The speed the motor is running at right now. It ramps toward the **requested throttle** at
a bounded rate instead of jumping to it.
_Avoid_: Speed, applied throttle

**Step rate**:
How fast the rear motor's steps are issued. The actual throttle is realised as a step
rate.
_Avoid_: RPM, pulse rate, frequency

**Steering angle**:
The direction the front wheels point, in degrees, relative to **straight**. Negative is
left, positive is right.
_Avoid_: Steering position, servo angle

**Straight**:
The steering angle at which the front wheels point directly ahead.
_Avoid_: Centered, home, zero

## Safety

**Obstacle avoidance**:
Preventing the car from entering the space an obstacle occupies. The car never chooses a
different path on its own — it refuses to move instead.
_Avoid_: Auto-avoid, dodging, self-steering

**Interlock**:
The rule that refuses throttle while an obstacle is within the **stop threshold**.
_Avoid_: Safety stop, bumper, obstacle logic

**Stop threshold**:
The distance ahead of the car at which the interlock engages.
_Avoid_: Obstacle threshold, braking distance

**E-stop**:
The physical button on the car that stops it immediately and can only be released
physically.
_Avoid_: Emergency stop, kill switch

**Latched stop**:
A stop condition that stays engaged until the user explicitly clears it.
_Avoid_: Error state, fault

**Armed**:
The condition in which the car accepts motion commands. A disarmed car ignores them
entirely.
_Avoid_: Enabled, ready, live, idle

**Blind spot**:
A direction the car cannot sense. With a single frontal sensor, everything that is not
ahead is a blind spot.
_Avoid_: Uncovered area

## Power

**Battery state**:
The measured voltage of the battery pack and the charge estimate derived from it.
_Avoid_: Battery level, charge

**Low battery**:
A battery state in which the pack is nearly empty and the car should not be driven far.
_Avoid_: Low voltage warning

**Critical battery**:
A battery state in which the pack is too empty to drive safely and the car stops
accepting throttle.
_Avoid_: Cutoff, brownout

**Charging**:
The car being connected to its charger. The car is not designed to be driven while
charging.
_Avoid_: Plugged in, on the dock

## Link

**Car link**:
The live connection between the phone and the car, proven by a recent **heartbeat**.
_Avoid_: Connection, session, Wi-Fi link

**Heartbeat**:
The periodic signal the phone sends to prove the car link is alive.
_Avoid_: Ping, keepalive

**Control intent**:
What the user is currently asking the car to do — a **requested throttle** and a
steering angle. It stays in effect only while the car link is fresh.
_Avoid_: Setpoint, joystick state

**Phone**:
The handheld device the user drives the car from.
_Avoid_: Controller, remote, app
