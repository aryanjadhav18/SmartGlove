
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DFRobotDFPlayerMini.h>

// Flex sensor pins
#define FLEX_PIN1 34
#define FLEX_PIN2 35
#define FLEX_PIN3 32

// DFPlayer Mini Serial pins
#define RX_PIN 16
#define TX_PIN 17

// DFPlayer audio file IDs
#define AUDIO_HELP     1
#define AUDIO_FOOD     3
#define AUDIO_WATER    4
#define AUDIO_MEDICINE 5
#define AUDIO_WELCOME  6
#define AUDIO_8        8
#define AUDIO_9        9

LiquidCrystal_I2C lcd(0x27, 16, 2);
DFRobotDFPlayerMini myDFPlayer;

bool welcomePlayed = false;
unsigned long lastSensorCheck = 0;
unsigned long lastAudioPlayTime = 0;
unsigned long lastLCDUpdateTime = 0;
unsigned long welcomeTime = 0;

const unsigned long sensorInterval = 300;
const unsigned long audioDelay = 2500;
const unsigned long lcdDisplayDuration = 1000;
const unsigned long welcomeHoldTime = 6000;

String currentMessage = "";

// Smooth out analog readings
int readFlex(int pin) {
  int total = 0;
  for (int i = 0; i < 5; i++) {
    total += analogRead(pin);
    delay(2);
  }
  return total / 5;
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();

  pinMode(FLEX_PIN1, INPUT);
  pinMode(FLEX_PIN2, INPUT);
  pinMode(FLEX_PIN3, INPUT);

  Serial2.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  if (!myDFPlayer.begin(Serial2)) {
    Serial.println("DFPlayer Mini not found!");
    while (1);
  }
  myDFPlayer.volume(30);
}

void displayMessage(String message) {
  lcd.clear();
  lcd.setCursor(0, 0);
  
  if (message.length() <= 16) {
    lcd.print(message);
  } else {
    lcd.setCursor(0,0);
    lcd.print(message.substring(0, 10));
    lcd.setCursor(0, 1);
    lcd.print(message.substring(10)); // Max 16 chars on 2nd line
  }

  currentMessage = message;
  lastLCDUpdateTime = millis();
}

void loop() {
  unsigned long now = millis();

  if (!welcomePlayed) {
    myDFPlayer.play(AUDIO_WELCOME);
    displayMessage("Welcome");
    welcomePlayed = true;
    lastAudioPlayTime = now;
    welcomeTime = now;
    return;
  }

  if (now - welcomeTime < welcomeHoldTime) {
    return;
  }

  if (now - lastSensorCheck >= sensorInterval) {
    lastSensorCheck = now;

    int flex1 = readFlex(FLEX_PIN1);
    int flex2 = readFlex(FLEX_PIN2);
    int flex3 = readFlex(FLEX_PIN3);

    Serial.printf("F1:%d F2:%d F3:%d\n", flex1, flex2, flex3);

    bool readyForAudio = (now - lastAudioPlayTime > audioDelay);

    if (flex2 < 1550 && flex3 < 1580 && readyForAudio) {
      displayMessage("Need medicines");
      myDFPlayer.play(AUDIO_MEDICINE);
      lastAudioPlayTime = now;
    }
    else if (flex2 < 1550 && flex2> 1500 && readyForAudio) {
      displayMessage("Give me food");
      myDFPlayer.play(AUDIO_FOOD);
      lastAudioPlayTime = now;
    }
    else if (flex2 < 1500 && readyForAudio) {
      displayMessage("Hello how are you?");
      myDFPlayer.play(AUDIO_8);
      lastAudioPlayTime = now;
    }
    else if (flex3 < 1580 && flex3> 1550 && readyForAudio) {
      displayMessage("Give me water");
      myDFPlayer.play(AUDIO_WATER);
      lastAudioPlayTime = now;
    }
      else if (flex3 < 1550 && readyForAudio) {
      displayMessage("What's your name? ");
      myDFPlayer.play(AUDIO_9);
      lastAudioPlayTime = now;
    }
    else if (flex1 > 2000 && readyForAudio) {
      displayMessage("Please Help Me");
      myDFPlayer.play(AUDIO_HELP);
      lastAudioPlayTime = now;
    }
    else if ((now - lastLCDUpdateTime > lcdDisplayDuration) &&
             currentMessage != "Nothing" &&
             flex1 <= 2000 && flex2 >= 1600 && flex3 >= 1600) {
      displayMessage("Nothing");
    }
  }
}
