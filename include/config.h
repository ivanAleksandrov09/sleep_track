#pragma once

// --- Display ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_I2C_ADDR 0x3C

// --- Pins ---
#define BUTTON_STOPBEGIN 25
#define BUTTON_RESET 35
#define DHT_PIN 27
#define BUZZER_PIN 12

// --- Button debounce ---
#define BTN_DEBOUNCE_MS 250
#define BTN_CONFIRM_WINDOW_MS 3000 // how long confirm prompt stays visible

// --- DHT ---
#define SENSOR_READ_INTERVAL_MS 2000

// --- Sleep scoring ---
#define SCORE_MAX 100
#define SCORE_DURATION_MAX 70
#define SCORE_COMFORT_MAX 30
#define IDEAL_SLEEP_HOURS 7.5f
#define IDEAL_TEMP 19.0f
#define IDEAL_HUMIDITY 50.0f

// --- History ---
#define HISTORY_COUNT 4
#define PREFS_NAMESPACE "sleeptracker"
