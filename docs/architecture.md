# System Architecture

## 1. Overview

The project is a **car-shaped mobile robot** composed of two primary computing layers:

1. Robot controller (ESP32)
2. External computer (PC)

The robot controller is responsible for real-time hardware interaction:

* Throttle (rear DC motor)
* Steering (front servo)
* Sensor-head rotation (servo)
* Ultrasonic acquisition
* Safety / emergency stop

The external computer is responsible for computationally heavier functionality such as visualization, mapping, experimentation, and eventually path planning.

> Note: this platform is **not** a differential-drive rover. It uses throttle + steering (bicycle/Ackermann model). There is no turn-in-place.

---

## 2. High-Level Architecture

```text
                    ┌──────────────────────────┐
                    │       User / PC          │
                    │                          │
                    │ Visualization            │
                    │ Map                      │
                    │ Path Planning            │
                    │ Commands                 │
                    └────────────┬─────────────┘
                                 │
                              Wi-Fi / MQTT
                                 │
                    ┌────────────▼─────────────┐
                    │          ESP32            │
                    │                           │
                    │ Throttle Control          │
                    │ Steering Control          │
                    │ Sensor-Head Control       │
                    │ Ultrasonic Acquisition    │
                    │ Safety / E-stop           │
                    │ Real-Time Logic           │
                    └──────┬───────────┬────────┘
                           │           │
                 ┌─────────┘           └─────────┐
                 ▼                               ▼
          ┌─────────────┐                 ┌─────────────┐
          │  Actuators  │                 │   Sensors   │
          │ rear motor  │                 │ 2× HC-SR04  │
          │ 2× servos   │                 │ E-stop btn  │
          └─────────────┘                 └─────────────┘
```

---

## 3. Robot Responsibilities

The ESP32 should initially handle:

* Throttle control (PWM, forward/reverse/stop)
* Steering servo control
* Sensor-head servo control
* Ultrasonic measurements (sequential firing)
* Safety stops (obstacle + connection timeout + E-stop)
* Command reception
* Telemetry transmission

Later it may also handle:

* Odometry (encoder + steering angle)
* Local obstacle avoidance
* Basic localization
* Low-level navigation control (throttle + steer setpoints)

---

## 4. External Computer Responsibilities

The external system may handle:

* Sensor visualization
* Data logging
* Map generation
* Map visualization
* Path planning
* Experiment analysis
* Configuration
* User interface

---

## 5. Separation of Concerns

The robot should not depend on the external computer for basic safety.

For example:

```text
Obstacle detected
      ↓
ESP32
      ↓
Immediate stop (throttle = 0)
```

The PC should not be required for this behavior.

However:

```text
Select destination
      ↓
PC
      ↓
Path planning
      ↓
Navigation command (throttle + steer)
      ↓
ESP32
```

can depend on the external system.

---

## 6. Robot State

The conceptual robot state is:

```text
RobotState {
    position        # world frame (x, y)
    orientation     # heading theta
    speed           # forward speed
    throttle        # commanded throttle (-100 .. +100)
    steeringAngle   # commanded steering angle, degrees
    sensorData      # distance measurements
    battery
    currentMode     # MANUAL / AUTO / SAFETY_STOP
}
```

The exact implementation will evolve.

---

## 7. Kinematic Model

The car uses the **bicycle model** (rear drive + front steering).

```text
      front wheels
         ▲ δ (steering angle)
        / \
       /   \
      ●─────●  ← steered, not driven
      │     │
      │     │  wheelbase L
      │     │
      ●─────●  ← driven rear axle, speed v
```

Relationships (for small angles, with wheelbase `L`):

```text
Instantaneous turning radius  R = L / tan(δ)
Heading rate                  θ̇ = (v / L) · tan(δ)
```

Consequences:

* Turns have a minimum radius set by the max steering angle: `R_min = L / tan(δ_max)`.
* The car cannot turn in place.
* Steering at high throttle is more dynamic than at low throttle.

The wheelbase `L` and max steering angle `δ_max` must be measured from the chassis and
recorded (see `docs/decisions.md`) — agents should not hard-code guessed values.

---

## 8. Coordinate Systems

The project will eventually use multiple coordinate systems.

### Robot coordinate system

Relative to the robot. Origin at the midpoint of the rear axle; X forward, Y to the left, θ is heading.

```text
        Y
        ↑
        │
        │
        ●──────→ X
       Robot
```

### World coordinate system

Fixed to the environment.

```text
        Y
        ↑
        │
        │
        └────────→ X
       Origin
```

A transformation between these systems will be required for mapping.

---

## 9. Final Data Flow

```text
Sensors
   ↓
Sensor Measurements
   ↓
Robot Coordinate System
   ↓
Localization
   ↓
World Coordinate System
   ↓
Map
   ↓
Path Planner
   ↓
Target Path
   ↓
Navigation Controller
   ↓
Throttle + Steering Commands
   ↓
Motor + Servos
```

---

## 10. Architecture Evolution

The architecture is expected to evolve.

Early phases should prioritize simplicity.

Complex robotics algorithms should only be introduced when a previous phase demonstrates the need for them.
