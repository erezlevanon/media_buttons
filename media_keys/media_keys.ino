/**
 * This example turns the ESP32 into a Bluetooth LE keyboard that writes the words, presses Enter, presses a media key and then Ctrl+Alt+Delete
 */
#include <BleKeyboard.h>
#include <OneButtonTiny.h>

#define DEBUG flase
#if DEBUG
#define LOG(x) \
  do { Serial.println(x); } while (0);
#else
#define LOG(x) \
  do { \
  } while (0);
#endif

///////////////SETUP////PARAMS////////////////////
// PINS
constexpr auto BUTTON_PIN = D1;
// How long to wait between checking BLE connection.
constexpr size_t BT_CONNECTION_INTERVAL_MS = 2000;
// How many times to check BLE connection before deep sleep.
constexpr size_t BT_CONNECTION_ATTEMPTS_PER_WAKEUP = 10;
// Delay at the end of each loop, too large and reading button works poorly.
constexpr size_t LOOP_DELAY = 15;
///////////////SETUP////PARAMS////////////////////

BleKeyboard bleKeyboard("playbutton-2");

OneButtonTiny btn(
  BUTTON_PIN,
  true,
  true);

uint8_t connectionWaitCount = 0;

void setup() {
  btn.setDebounceMs(10);
  btn.setClickMs(10);
  btn.attachClick(playpause);
  btn.attachLongPressStart(prevTrack);
#ifdef DEBUG
  Serial.begin(115200);
#endif
  LOG("Starting BLE work!");

  bleKeyboard.begin();
}

void playpause() {
  if (bleKeyboard.isConnected()) {
    LOG("Sending Play/Pause media key...");
    bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
  }
}

void prevTrack() {
  if (bleKeyboard.isConnected()) {
    LOG("Sending Rewind...");
    bleKeyboard.write(KEY_MEDIA_PREVIOUS_TRACK);
  }
}

void loop() {
  btn.tick();

  if (!bleKeyboard.isConnected()) { 
    LOG("Not connetcted");
    connectionWaitCount++;
    delay(BT_CONNECTION_INTERVAL_MS);
    if (connectionWaitCount >= BT_CONNECTION_ATTEMPTS_PER_WAKEUP) {
      esp_deep_sleep_enable_gpio_wakeup(BIT(D1), ESP_GPIO_WAKEUP_GPIO_LOW);
      LOG("Going into deep sleep");
      esp_deep_sleep_start();
    }
  } else {
    delay(LOOP_DELAY);
    connectionWaitCount = 0;
  }
}
