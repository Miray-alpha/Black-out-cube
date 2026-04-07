#ifndef _SYSTEM_H
#define _SYSTEM_H

// ================================================================
// SYSTEM CONTROL
// ================================================================

void system_reset();
void system_off();
void system_sleep();
bool system_sleeping();

// ================================================================
// WATCHDOG TIMER
// ================================================================

void system_wdt_enable(unsigned long seconds);
void system_wdt_disable();
void system_wdt_feed();
bool system_wdt_triggered();

// ================================================================
// DELAY
// ================================================================

void system_delay(uint32_t ms);

// ================================================================
// BLACKOUT MODE (LED POWER CONTROL)
// ================================================================

// Initialize MOSFET pin
void system_blackout_setup();

// Enable / disable blackout
void system_set_blackout(bool enable);

// Toggle blackout
void system_toggle_blackout();

// Check blackout state
bool system_blackout_enabled();

#endif // _SYSTEM_H
