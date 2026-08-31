# Development Roadmap

Phases are defined for the **car platform** (rear throttle motor + front steering servo).
There is no differential drive and no turn-in-place; movement is throttle + steering angle.

---

## Phase 0 — Foundation

### Objective

Define the project architecture, requirements, hardware assumptions, and development workflow.

### Deliverables

* Repository structure
* Project documentation
* Architecture (incl. kinematic model)
* Hardware plan (incl. pin map)
* Communication strategy (MQTT/JSON)
* Phase definitions

### Definition of Done

The project can be explained from hardware to final navigation without requiring implementation details.

---

## Phase 1 — Throttle & Steering Control

### Objective

Make the car physically move and steer.

### Deliverables

* L298N wiring (single motor channel)
* PWM throttle: forward, reverse, stop/brake
* Throttle calibration (dead zone, mapping to speed)
* Steering servo control (center calibration, min/max angle, δ_max)
* Sensor-head servo control (basic pan)
* Physical E-stop button reading
* Safety stop on boot

### Definition of Done

The car can execute deterministic throttle (forward/reverse/stop) and steering-angle commands.

---

## Phase 2 — Remote Control

### Objective

Allow a user to control the car remotely.

### Deliverables

* MQTT connect (STA / router mode first)
* Command handling: THROTTLE, STEER, STOP, ESTOP, RESET_ESTOP, SET_SPEED_LIMIT
* Basic telemetry (throttle, steering angle, battery, mode)
* Connection-timeout stop (500 ms window)
* Auto-reconnect while stopped

### Definition of Done

The user can reliably drive the car remotely, and the car stops when the link is lost.

---

## Phase 3 — Obstacle Avoidance

### Objective

Give the car basic autonomous behavior.

### Deliverables

* Ultrasonic measurement (2 sensors, sequential firing)
* Distance filtering
* Obstacle threshold
* Emergency stop (obstacle + E-stop)
* Basic avoidance behavior (steer around obstacles)

### Definition of Done

The car can move autonomously without intentionally colliding with detected obstacles.

---

## Phase 4 — Rotating Sonar

### Objective

Transform the ultrasonic sensors into a directional scanning system.

### Deliverables

* Sensor-head servo integration
* Angular positioning
* Dual-sensor firing sequence (no crosstalk)
* 0–180° scan
* Angle/distance dataset
* Scan repeatability

> Sensor-head layout (angled vs parallel) and exact scan geometry are TBD — see `docs/decisions.md`.

### Definition of Done

The car can perform a repeatable 180° scan and produce structured measurement data.

---

## Phase 5 — Visualization

### Objective

Visualize sensor measurements outside the car (Python on the PC).

### Deliverables

* Telemetry protocol (JSON/MQTT)
* Scan transmission (`car/scan`)
* Desktop visualization
* Polar representation
* Cartesian point representation

### Definition of Done

A user can observe the car's current sensor scan in real time.

---

## Phase 6 — Mapping

### Objective

Create a persistent representation of the environment.

### Deliverables

* Coordinate system
* Point cloud
* Grid representation
* Occupancy classification
* Map update mechanism
* Map persistence

### Definition of Done

The system can produce a useful 2D representation of a test environment.

---

## Phase 7 — Localization

### Objective

Estimate the car's position and orientation.

### Candidate approaches

* Rear-wheel encoder (preferred candidate — status TBD)
* Steering-angle measurement + bicycle-model odometry
* IMU
* Sensor fusion
* Scan matching

The final approach will be selected after experimentation.

### Deliverables

* Position representation
* Orientation representation
* Movement estimation (bicycle model)
* Error measurement
* Localization evaluation

### Definition of Done

The system can estimate the car's pose within an experimentally defined acceptable error.

---

## Phase 8 — Autonomous Exploration

### Objective

Allow the car to explore an environment while updating its map.

### Deliverables

* Exploration strategy
* Unknown-space detection
* Movement decisions (throttle + steer)
* Map updates
* Exploration termination condition

### Definition of Done

The car can autonomously explore a controlled environment and produce a map.

---

## Phase 9 — Path Planning

### Objective

Calculate a route between two coordinates, respecting the car's kinematics.

### Initial algorithm

A*.

### Deliverables

* Grid-based path representation
* Start coordinate
* Goal coordinate
* Obstacle representation
* A* implementation
* Path validation
* Kinematic feasibility check (turning radius / δ_max)
* Path visualization

### Definition of Done

Given a known map, the system can calculate a collision-free path between two reachable coordinates that the car can physically follow.

---

## Phase 10 — Autonomous Navigation

### Objective

Convert the planned path into physical car movement.

### Deliverables

* Path following (e.g. pure pursuit / Stanley)
* Steering-angle setpoint calculation
* Throttle control
* Turning behavior (steered turns, no turn-in-place)
* Position feedback
* Obstacle monitoring
* Goal detection

### Definition of Done

The car can receive a reachable destination and autonomously navigate to it.

---

# Final Integration

```text
Sensors
   ↓
Perception
   ↓
Localization
   ↓
Mapping
   ↓
Path Planning
   ↓
Navigation
   ↓
Throttle + Steering
   ↓
Motor + Servos
```

The final system should connect all previously validated components.
