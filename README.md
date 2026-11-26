# DUM-E: a waiter robot
this is the project page for my 12th grade finals project in mechatronics.

DUM-E is a waiter robot for a restaurant or café. It finds the customer with a UWB "radar", brings the order to the table, and avoids obstacles and people on the way.

## how it works
- **the robot** has a MaUWB_AOA Gen2 anchor (the "radar") that measures the distance and angle to every tag, a lidar to spot obstacles, and a tray with a lid that only opens at the right table.
- **every table** has a remote: an ESP32 with a color screen, a joystick, buttons and a MaUWB_AOA Gen2 tag. The remote is based on my [ESP32 Pocket Arcade](https://github.com/omers2008/esp32-pocket-arcade), so customers can also play games while they wait.
- **the owner / kitchen** has the same remote. It is used to set the menu, receive orders and confirm when an order is ready.
- the remotes and the robot talk to each other over ESP-NOW.

**POC (working now):** the radar is connected to the remote and the tag is connected to a power source. The remote shows the tag's distance and angle on a radar screen. Next step: building the robot itself.

## commands
On the remote you build a short sentence: pick a command, then a table (1 to 4, or "me"). Joystick up/down changes the option, right goes to the next word (on the last word it becomes "send to DUM-E"), left goes back.

1. **go:** drives to the selected table, or to me.
2. **give:** opens the tray, waits for me to put an item in, drives to the selected table and opens the tray for them. For example a waiter sending a dish to a table, or a customer sending something to another table.
3. **get / order:** order from the menu, or ask another table for an item (see below).
4. **follow:** follows the selected remote until another command, for example a waiter while serving.
5. **wait:** stops where it is until another command.
6. **wander:** a single-word command that roams with no target, using Google's [fly brain map](https://blog.google/innovation-and-ai/technology/research/male-fruit-fly-brain-map/). I'm considering adding a camera and maybe a light sensor so it can react to more than just the lidar.

### get / order and the menu
"get" became "get / order". Besides asking another table for an item, a customer can order items from a menu set by the owner:

1. **the owner sets the menu:** on their remote they add, change or remove items (dishes, drinks, desserts). The menu is saved on the robot and sent to every remote, so a change reaches every table right away.
2. **the customer orders:** picks "get / order", scrolls the menu with the joystick, picks an item and sends it.
3. **the order reaches the kitchen:** the owner's remote shows the item and the table number.
4. **the order is ready:** staff put it in the robot's tray and confirm on the remote.
5. **the robot serves:** drives to the table by its tag, opens the tray, and the customer confirms they took it.

The old "get" still works: pick a table and ask it for an item, for example the owner sending the robot to collect dishes. Whoever is asked can also reject the request.

## robot design
Size, battery and charging, water and dirt resistance, stability and the full parts list are in [docs/robot-design.md](docs/robot-design.md).
