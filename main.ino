#include <Arduino.h>

#include "config.h"
#include "system.h"
#include "cube.h"
#include "utils.h"

// ================================================================
// SETUP
// Runs once when device starts
// ================================================================

void setup() {

    // Start serial (for debugging)
    Serial.begin(115200);
    delay(100);

    // Initialize system utilities (battery, buzzer, etc.)
    utils_setup();

    //  CRITICAL: initialize blackout (MOSFET)
    system_blackout_setup();

    // ============================================================
    // TEST BLACKOUT (REMOVE LATER)
    // ============================================================

    delay(2000);

    Serial.println("Blackout ON");
    system_set_blackout(true);   // LEDs OFF

    delay(2000);

    Serial.println("Blackout OFF");
    system_set_blackout(false);  // LEDs ON
}

// ================================================================
// LOOP
// Runs forever
// ================================================================

void loop() {

    // Feed watchdog if you use it
    system_wdt_feed();

    // You will later add:
    // - touch handling
    // - cube logic
    // - teaching mode
}
