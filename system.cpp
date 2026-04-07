#include <Arduino.h>

#include "system.h"
#include "bluetooth.h"
#include "display.h"
#include "touch.h"
#include "config.h"

// ================================================================
// MODULE STATE VARIABLES
// ================================================================

// Tracks if system is sleeping
bool _system_sleeping = false;

// Tracks blackout state
// true  = LEDs OFF
// false = LEDs ON
bool _system_blackout = false;

#if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
__attribute__ ((section(".bss.uninitialised"),zero_init)) unsigned int _system_wdt_triggered;
#elif defined(__GNUC__)
__attribute__ ((section(".uninitialised"))) unsigned int _system_wdt_triggered;
#elif defined(__ICCARM__)
unsigned int _system_wdt_triggered @ ".uninitialised";
#endif

// ================================================================
// PRIVATE FUNCTIONS
// ================================================================

// Check if SoftDevice is enabled
bool isSoftDevice() {
    uint8_t check;
    sd_softdevice_is_enabled(&check);
    return check == 1;
}

// Watchdog interrupt
void WDT_IRQHandler(void) {
    _system_wdt_triggered = 1;
    NRF_WDT->EVENTS_TIMEOUT = 0;
}

// ================================================================
// BLACKOUT CONTROL (CORE FEATURE)
// ================================================================

// Initialize MOSFET pin
void system_blackout_setup() {
    pinMode(MOSFET_PIN, OUTPUT);

    // Default: LEDs ON
    digitalWrite(MOSFET_PIN, HIGH);

    _system_blackout = false;
}

// Enable or disable blackout
void system_set_blackout(bool enable) {

    _system_blackout = enable;

    if (enable) {
        // Turn OFF LED power
        digitalWrite(MOSFET_PIN, LOW);
    } else {
        // Turn ON LED power
        digitalWrite(MOSFET_PIN, HIGH);
    }
}

// Toggle blackout mode
void system_toggle_blackout() {
    system_set_blackout(!_system_blackout);
}

// Return blackout state
bool system_blackout_enabled() {
    return _system_blackout;
}

// ================================================================
// WATCHDOG FUNCTIONS
// ================================================================

bool system_wdt_triggered() {
    bool output = (_system_wdt_triggered == 1);
    _system_wdt_triggered = 0;
    return output;
}

void system_wdt_enable(unsigned long seconds) {
    _system_wdt_triggered = 0;
    NRF_WDT->CONFIG      = (WDT_CONFIG_HALT_Pause << WDT_CONFIG_HALT_Pos) |
                           (WDT_CONFIG_SLEEP_Run << WDT_CONFIG_SLEEP_Pos);
    NRF_WDT->CRV         = 32768 * seconds + 1;
    NRF_WDT->RREN       |= WDT_RREN_RR0_Msk;
    NVIC_SetPriority(WDT_IRQn, 7);
    NVIC_ClearPendingIRQ(WDT_IRQn);
    NVIC_EnableIRQ(WDT_IRQn);
    NRF_WDT->INTENSET    = WDT_INTENSET_TIMEOUT_Msk;
    NRF_WDT->TASKS_START = 1;
}

void system_wdt_disable() {
    NRF_WDT->CONFIG = (WDT_CONFIG_HALT_Pause << WDT_CONFIG_HALT_Pos) |
                      (WDT_CONFIG_SLEEP_Pause << WDT_CONFIG_SLEEP_Pos);
    NRF_WDT->CRV    = 4294967295;
}

void system_wdt_feed() {
    if ((bool)(NRF_WDT->RUNSTATUS) == true) {
        NRF_WDT->RR[0] = WDT_RR_RR_Reload;
    }
}

// ================================================================
// BASIC SYSTEM CONTROL
// ================================================================

void system_reset() {
    NVIC_SystemReset();
}

// Power OFF system
void system_off() {

    delay(100);

    bluetooth_scan(false);
    bluetooth_disconnect();

    display_clear();
    display_off();

    touch_power_mode(1);

    sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);
    Serial.end();

    // 🔥 IMPORTANT: turn LEDs OFF before shutdown
    system_set_blackout(true);

    NRF_POWER->SYSTEMOFF = 1;
}

// Check sleep state
bool system_sleeping() {
    return _system_sleeping;
}

// Sleep mode
void system_sleep() {

    _system_sleeping = true;

    delay(100);

    bluetooth_scan(false);
    bluetooth_disconnect();

    display_clear();
    display_off();
    touch_power_mode(1);

    Serial.end();

    // 🔥 IMPORTANT: blackout during sleep
    system_set_blackout(true);

    NRF_POWER->TASKS_LOWPWR = 1;

    while (true) {
        __WFE();
        __WFI();

        delay(2000);

        if (digitalRead(TOUCH_INT_PIN) == LOW) {
            break;
        }
    }

    display_on();
    touch_power_mode(0);
    touch_setup(TOUCH_INT_PIN);

    Serial.begin(115200);
    delay(100);

    // Restore LEDs ON after wake
    system_set_blackout(false);

    _system_sleeping = false;
}

// Delay helper
void system_delay(uint32_t ms) {
    uint32_t start = millis();
    while (millis() - start < ms) delay(1);
}
