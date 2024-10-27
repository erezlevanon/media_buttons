/**
 * This example turns the ESP32 into a Bluetooth LE keyboard that writes the words, presses Enter, presses a media key and then Ctrl+Alt+Delete
 */
#include <BleKeyboard.h>
#include <OneButtonTiny.h>

#define DEBUG false
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

// Battery
// IMPORTANT! if using NimBLE battery level will only work with this NimBLE hack.
// https://github.com/T-vK/ESP32-BLE-Keyboard/issues/210#issuecomment-1995165739
#define CHECK_BATTERY true
#if CHECK_BATTERY
constexpr auto BATTERY_READ_PIN = A0;
// Got from https://www.tinytronics.nl/en/power/batteries/li-po/pkcell-li-po-battery-3.7v-150mah-jst-ph-lp402025
constexpr float MIN_BATTERY_VOLTAGE = 2.9f;
constexpr float MAX_BATTERY_VOLTAGE = 4.075f;
// How often to test for battery level.
constexpr size_t MS_BATTERY_CHECK = 1000 * 60 * 3;  // 3 minutes.
constexpr size_t BATTERY_NUM_MEASUREMENTS = 16;
#endif
///////////////SETUP////PARAMS////////////////////

BleKeyboard bleKeyboard("playbutton");

OneButtonTiny btn(
  BUTTON_PIN,
  true,
  true);

uint8_t connectionWaitCount = 0;

#if CHECK_BATTERY
size_t last_battery_check = -MS_BATTERY_CHECK;
void updateBattery() {
  if (millis() - last_battery_check < MS_BATTERY_CHECK) return;
  LOG("Update Battery");
  last_battery_check = millis();
  uint32_t vBatt = 0;
  for (int i = 0; i < 16; i++) {
    vBatt += analogReadMilliVolts(BATTERY_READ_PIN);
  }
  float vBattF = 2.0 * vBatt / 16.0 / 1000.0;
  LOG("Battery voltage:")
  LOG(vBattF);
  uint8_t level = (uint8_t)fmap((float)vBattF, MIN_BATTERY_VOLTAGE, MAX_BATTERY_VOLTAGE, 0.0f, 100.0f);
  LOG("Battery level:")
  LOG(level);
  bleKeyboard.setBatteryLevel(level);
}

#define UPDATE_BATTERY() \
  do { \
    updateBattery(); \
  } while (0);
#else
#define UPDATE_BATTERY() \
  do { \
  } while (0);
#endif

template<typename T> T fmap(T value, T in_min, T in_max, T out_min, T out_max) {
  return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setup() {
#if CHECK_BATTERY
  pinMode(BATTERY_READ_PIN, INPUT);
#endif
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
    UPDATE_BATTERY();
    delay(LOOP_DELAY);
    connectionWaitCount = 0;
  }
}
