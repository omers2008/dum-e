# DUM-E Clicker Phrase Selector

This is the first test version of the DUM-E clicker UI. It runs on one ESP32 with:

- 16x2 I2C LCD
- Analog joystick module with push button
- Jumper wires

It does not need the robot or DWM3000 yet. For now, sent commands are printed to the Arduino Serial Monitor.

## Controls

- Joystick up/down: change the current word option
- Joystick right: move to the next word, or send the phrase when complete
- Joystick left: move back to the previous word
- Joystick press: update/print the current command data

## Commands

- `Go` accepts `Me` or another user
- `Get` accepts another user, not `Me`
- `Give` accepts another user, not `Me`
- `Follow` accepts `Me` or another user
- `Wait` is a one-word command
- `Wander` is a one-word command

## Wiring

Use 3.3V for the joystick VCC. ESP32 analog pins are not 5V-safe.

### LCD, I2C 16x2

| LCD pin | ESP32 pin |
| --- | --- |
| GND | GND |
| VCC | 3V3 first |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

If the LCD does not turn on properly from 3.3V, it may need 5V power. If you power the LCD from 5V and the I2C backpack pulls SDA/SCL up to 5V, use a logic level shifter before connecting SDA/SCL to the ESP32.

### Joystick

| Joystick pin | ESP32 pin |
| --- | --- |
| GND | GND |
| +5V / VCC | 3V3 |
| VRx / X | GPIO 34 |
| VRy / Y | GPIO 35 |
| SW / button | GPIO 32 |

If up/down/left/right feel reversed, change the signs in `handleJoystick()`.

## Arduino IDE Setup

1. Install Arduino IDE.
2. Open Boards Manager and install `esp32` by Espressif Systems.
3. Open Library Manager and install `LiquidCrystal I2C`.
4. Open `DUME_Clicker_Phrase_Selector.ino`.
5. Select your board. Usually `ESP32 Dev Module` works.
6. Select the ESP32 COM port.
7. Upload.
8. Open Serial Monitor at `115200 baud`.

## Settings To Change Later

At the top of the sketch:

```cpp
constexpr int MY_USER = 1;
constexpr int USER_COUNT = 4;
```

Change `MY_USER` for each clicker owner. Change `USER_COUNT` when you add more users.

If the LCD stays blank but the backlight works, try changing:

```cpp
LiquidCrystal_I2C lcd(0x27, 16, 2);
```

to:

```cpp
LiquidCrystal_I2C lcd(0x3F, 16, 2);
```
