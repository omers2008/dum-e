/*
  DUM-E clicker phrase selector

  Hardware:
  - ESP32 dev board
  - 16x2 I2C LCD
  - Analog joystick with push button

  Controls:
  - Up / Down: change the highlighted word
  - Right: move to the next word, or send when the phrase is complete
  - Left: go back to the previous word
  - Press joystick: print/update the current phrase data over Serial
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Change this to 0x3F if your LCD does not show text with 0x27.
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ESP32 default I2C pins.
constexpr int LCD_SDA_PIN = 21;
constexpr int LCD_SCL_PIN = 22;

// Joystick pins. Use 3.3V for joystick VCC so analog reads are safe for ESP32.
constexpr int JOY_X_PIN = 34;
constexpr int JOY_Y_PIN = 35;
constexpr int JOY_SW_PIN = 32;

// Set this clicker's owner. Target lists will skip this number for Get/Give.
constexpr int MY_USER = 1;
constexpr int USER_COUNT = 4;

constexpr int ADC_CENTER = 2048;
constexpr int ADC_DEADZONE = 850;
constexpr unsigned long MOVE_REPEAT_MS = 230;
constexpr unsigned long BUTTON_DEBOUNCE_MS = 250;
constexpr unsigned long MESSAGE_MS = 900;

enum CommandType {
  CMD_GO,
  CMD_GET,
  CMD_GIVE,
  CMD_FOLLOW,
  CMD_WAIT,
  CMD_WANDER,
  COMMAND_COUNT
};

const char* COMMANDS[COMMAND_COUNT] = {
  "Go",
  "Get",
  "Give",
  "Follow",
  "Wait",
  "Wander"
};

int commandIndex = 0;
int targetIndex = 0;
int wordIndex = 0;  // 0 = command, 1 = target/send

unsigned long lastMoveAt = 0;
unsigned long lastButtonAt = 0;
unsigned long messageUntil = 0;
String temporaryMessage = "";

bool commandNeedsTarget() {
  return commandIndex == CMD_GO ||
         commandIndex == CMD_GET ||
         commandIndex == CMD_GIVE ||
         commandIndex == CMD_FOLLOW;
}

bool commandAllowsMe() {
  return commandIndex == CMD_GO || commandIndex == CMD_FOLLOW;
}

int availableTargetCount() {
  int count = 0;
  if (commandAllowsMe()) {
    count++;
  }

  for (int user = 1; user <= USER_COUNT; user++) {
    if (user != MY_USER) {
      count++;
    }
  }

  return count;
}

String targetNameAt(int index) {
  if (commandAllowsMe()) {
    if (index == 0) {
      return "Me";
    }
    index--;
  }

  for (int user = 1; user <= USER_COUNT; user++) {
    if (user == MY_USER) {
      continue;
    }
    if (index == 0) {
      return "User " + String(user);
    }
    index--;
  }

  return "User ?";
}

String selectedTargetName() {
  return targetNameAt(targetIndex);
}

void clampState() {
  if (!commandNeedsTarget()) {
    wordIndex = 0;
    targetIndex = 0;
    return;
  }

  int count = availableTargetCount();
  if (targetIndex >= count) {
    targetIndex = count - 1;
  }
  if (targetIndex < 0) {
    targetIndex = 0;
  }
}

String phraseText() {
  if (!commandNeedsTarget()) {
    return String(COMMANDS[commandIndex]);
  }
  return String(COMMANDS[commandIndex]) + " " + selectedTargetName();
}

String commandJson() {
  String target = commandNeedsTarget() ? selectedTargetName() : "";
  String json = "{";
  json += "\"command\":\"" + String(COMMANDS[commandIndex]) + "\"";
  json += ",\"target\":\"" + target + "\"";
  json += ",\"sender\":\"User " + String(MY_USER) + "\"";
  json += "}";
  return json;
}

void showTemporaryMessage(const String& message) {
  temporaryMessage = message;
  messageUntil = millis() + MESSAGE_MS;
}

void printCentered(const String& text) {
  int padding = (16 - text.length()) / 2;
  if (padding < 0) {
    padding = 0;
  }
  for (int i = 0; i < padding; i++) {
    lcd.print(' ');
  }
  lcd.print(text.substring(0, 16 - padding));
}

void drawScreen() {
  lcd.clear();

  if (millis() < messageUntil) {
    lcd.setCursor(0, 0);
    printCentered(temporaryMessage);
    lcd.setCursor(0, 1);
    lcd.print(phraseText().substring(0, 16));
    return;
  }

  lcd.setCursor(0, 0);
  lcd.print(wordIndex == 0 ? ">" : " ");
  lcd.print(COMMANDS[commandIndex]);

  if (commandNeedsTarget()) {
    lcd.setCursor(0, 1);
    lcd.print(wordIndex == 1 ? ">" : " ");
    lcd.print(selectedTargetName());

    int sendCol = 11;
    lcd.setCursor(sendCol, 1);
    lcd.print(wordIndex == 1 ? "Send" : "Next");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("Right=Send");
  }
}

void sendPhrase() {
  Serial.println("SEND " + commandJson());
  showTemporaryMessage("Sent to DUM-E");
  drawScreen();
}

void updatePhraseData() {
  Serial.println("UPDATE " + commandJson());
  showTemporaryMessage("Data updated");
  drawScreen();
}

void moveUpDown(int direction) {
  if (wordIndex == 0) {
    commandIndex += direction;
    if (commandIndex < 0) {
      commandIndex = COMMAND_COUNT - 1;
    }
    if (commandIndex >= COMMAND_COUNT) {
      commandIndex = 0;
    }
    clampState();
    return;
  }

  int count = availableTargetCount();
  targetIndex += direction;
  if (targetIndex < 0) {
    targetIndex = count - 1;
  }
  if (targetIndex >= count) {
    targetIndex = 0;
  }
}

void moveLeftRight(int direction) {
  if (direction < 0) {
    if (wordIndex > 0) {
      wordIndex--;
    }
    return;
  }

  if (!commandNeedsTarget()) {
    sendPhrase();
    return;
  }

  if (wordIndex == 0) {
    wordIndex = 1;
  } else {
    sendPhrase();
  }
}

void handleJoystick() {
  int x = analogRead(JOY_X_PIN);
  int y = analogRead(JOY_Y_PIN);
  bool pressed = digitalRead(JOY_SW_PIN) == LOW;
  unsigned long now = millis();

  if (pressed && now - lastButtonAt > BUTTON_DEBOUNCE_MS) {
    lastButtonAt = now;
    updatePhraseData();
    return;
  }

  if (now - lastMoveAt < MOVE_REPEAT_MS) {
    return;
  }

  // Some joystick modules invert directions. Swap signs here if yours feels backward.
  if (x > ADC_CENTER + ADC_DEADZONE) {
    lastMoveAt = now;
    moveLeftRight(1);
    drawScreen();
  } else if (x < ADC_CENTER - ADC_DEADZONE) {
    lastMoveAt = now;
    moveLeftRight(-1);
    drawScreen();
  } else if (y > ADC_CENTER + ADC_DEADZONE) {
    lastMoveAt = now;
    moveUpDown(-1);
    drawScreen();
  } else if (y < ADC_CENTER - ADC_DEADZONE) {
    lastMoveAt = now;
    moveUpDown(1);
    drawScreen();
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);

  pinMode(JOY_SW_PIN, INPUT_PULLUP);
  analogReadResolution(12);

  Wire.begin(LCD_SDA_PIN, LCD_SCL_PIN);
  lcd.init();
  lcd.backlight();

  lcd.clear();
  lcd.setCursor(0, 0);
  printCentered("DUM-E Clicker");
  lcd.setCursor(0, 1);
  printCentered("Starting...");
  delay(900);

  clampState();
  drawScreen();
  Serial.println("DUM-E clicker ready");
}

void loop() {
  handleJoystick();

  if (temporaryMessage.length() > 0 && millis() >= messageUntil) {
    temporaryMessage = "";
    drawScreen();
  }
}
