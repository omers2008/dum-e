# Robot design

Design targets and part choices for the DUM-E waiter robot. Numbers are targets, not measurements yet.

## Targets
| Requirement | Target |
|---|---|
| Size | Footprint about 40×40 cm, tray at about 70–75 cm high (table height), weight up to about 8 kg |
| Load | Up to about 2 kg (a dish and a drink) |
| Speed | Up to about 0.5 m/s |
| Smooth driving | Gentle acceleration and braking, about 0.3 m/s² |
| Obstacles | Stop when an obstacle or a person is closer than about 30 cm, then go around or wait |
| Battery | At least 2 hours of work, fast swap, and autonomous charging on a dock |
| Water and dirt | Splash-proof (roughly IP54 as a design goal, not certified) |
| Tables | At least 2, up to 4 |
| Menu | Up to about 20 items, set by the owner |

## Lidar choice
The lidar is a **Slamtec RPLIDAR C1**. It replaces the YDLIDAR X2 from the first plan.

| Lidar | Price (Nov 2026) | Ranging / range | Why not / why |
|---|---|---|---|
| **RPLIDAR C1 (chosen)** | ₪200–227 on AliExpress with free delivery; $69 at DFRobot | ToF, 12 m (6 m on black), 5000 samples/s, 10 Hz, ±30 mm | IP54 dust and splash rating, which answers the water-and-dirt requirement. Well-known brand with good docs. |
| LDROBOT D500 | ₪208 on AliExpress | ToF, 12 m, 5000 samples/s, 10 Hz | About the same performance and price, but no water/dust rating and thinner docs. |
| LDROBOT D200 | ₪165 on AliExpress | Triangulation, 8 m | Cheapest, but triangulation is less accurate at distance. |
| YDLIDAR X2 | About €80 | Triangulation, 8 m | Same price as the C1 with worse specs. |

Wiring to the ESP32: 5 V power (about 230 mA, 1.15 W) and a 3.3 V UART at 460800 baud, straight to a free ESP32 UART with no level shifter. Buy the kit with the USB adapter board, so it can be tested on a computer first.

## Closed-loop control
Driving to a table is a closed loop:

- **Setpoint:** the tag's angle is 0° (straight ahead) and the distance is about 0.5 m from the table.
- **Controller:** the ESP32 on the robot compares the setpoint with the measurement and computes a correction.
- **Actuators:** the motor driver, then the motors and wheels.
- **Feedback:** the UWB anchor measures the real angle and distance.
- **Inner loop:** the wheel encoders measure each wheel's speed, so the ESP32 can ramp the speed up and down smoothly.
- **Disturbances:** obstacles seen by the lidar, which make the robot stop or slow down.

## Battery and charging
A 3S Li-ion pack (18650 cells, 3S2P, about 11–12.6 V and 6 Ah) runs the 12 V motors directly.
- **Rough draw:** about 2 A total (motors about 0.5–1 A each on average, lidar + ESP32 + anchor about 0.5 A), so about 3 hours on paper.
- **Fast swap:** the pack sits in a 3D-printed tray that slides into the bottom of the robot onto an XT60 connector.
- **Autonomous charging:** the dock has its own UWB tag, so the robot finds it like a table. Spring contacts on the robot touch plates on the dock, and a 3S Li-ion charger in the dock charges the pack.
- **Battery level:** a voltage divider into an ESP32 analog pin. Below about 20%, the robot drives to the dock.

Alternative: a power-tool battery (Makita/DeWalt 18 V) with a printed adapter and a step-down to 12 V. Easiest to swap, but hard to charge through dock contacts.

## Water and dirt
- The electronics are in a closed PETG box with a TPU gasket and cable glands.
- The tray is separate and removable, with a raised edge so spills stay in it.
- The anchor can sit behind plastic (UWB passes through plastic, not metal).
- The RPLIDAR C1 is IP54 rated, so splashes are fine, but it needs a clear 360° view, so it sits low under an overhang with a small skirt against dirt.
- Fenders over the wheels.

## Shocks and stability
- Battery and motors at the very bottom for a low center of gravity.
- Two driven wheels in the middle (differential drive) and spring-loaded casters front and back, so both drive wheels stay on the floor.
- TPU tires with low infill to absorb small bumps.
- Rubber anti-vibration mounts under the electronics box and the tray.
- A non-slip mat and cup cutouts in the tray.
- The biggest factor is software: the encoder loop keeps acceleration gentle.

## Parts
| Area | Part | Why we need it |
|---|---|---|
| **Core** | ESP32 (one on the robot, one per remote) | Controls the robot and runs each remote |
| | MaUWB_AOA Gen2 anchor | Measures the distance and angle to every tag |
| | MaUWB_AOA Gen2 tags | One per table remote and one for the charging dock |
| | Slamtec RPLIDAR C1 | 360° obstacle detection: ToF, 12 m range, IP54; 5 V power, 3.3 V UART at 460800 baud |
| | 2 × 12 V DC gear motors with encoders + motor driver | Drive the robot; the encoders make smooth closed-loop speed control possible |
| | SG90 servo | Opens and closes the tray lid |
| | 2" ST7789 TFT, joystick, buttons | The remote's screen and controls |
| **Battery** | 18650 Li-ion cells, 3S2P (6 cells, ~3000 mAh each) | About 11–12.6 V and ~6 Ah: runs the 12 V motors directly for 2+ hours |
| | 3S BMS board | Protects the cells from overcharge, over-discharge and short circuits, and keeps them balanced |
| | 3D-printed slide-in battery tray | Swap an empty pack for a charged one in seconds |
| | XT60 connector (or Anderson PP45) | Handles the motor current and survives many swaps |
| | Fuse + holder (~10 A) right after the battery | Cuts power on a short circuit |
| | Main power switch | Turns the robot off safely before swapping or working on it |
| | Voltage divider (2 resistors) to an ESP32 analog pin | Lets the robot measure its battery level and know when to charge |
| | Step-down converters to 5 V and 3.3 V | 5 V for the anchor and lidar, 3.3 V for the ESP32 and tag |
| **Charging dock** | 3S Li-ion charger (12.6 V, ~2 A) | Charges the pack properly; a plain power supply would overcharge it |
| | Spring contacts (pogo pins or copper strips) + metal plates on the dock | The robot connects to the charger just by driving onto the dock |
| | UWB tag on the dock | The robot finds the dock the same way it finds a table |
| **Water and dirt** | PETG filament | Electronics box and tray parts; handles water and heat better than PLA |
| | TPU gasket for the electronics box lid | Seals the lid against splashes |
| | Cable glands (PG7) | Wires leave the box without open holes |
| | Removable tray with a raised edge | Keeps spills in the tray and comes out for washing |
| | Wheel fenders (printed) | Keep dirt and liquid away from the motors and body |
| | Lidar skirt / overhang (printed) | Protects the lidar while keeping its 360° view open |
| **Shocks and stability** | Spring-loaded casters (front and back) | Keep both drive wheels on the floor on uneven ground |
| | Pivot arm + spring per motor (alternative) | Same goal: each drive wheel gets its own suspension |
| | TPU tires, low infill | Absorb small bumps and grip better than hard plastic |
| | Rubber anti-vibration mounts (M3) | Isolate the electronics box and the tray from shocks and vibration |
| | Non-slip silicone mat + cup cutouts | Stop plates and glasses from sliding |
| | IMU (MPU6050), optional | Detects bumps and tilt so the robot can slow down; helps smooth turns |
