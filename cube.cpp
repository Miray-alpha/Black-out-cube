#include <Arduino.h>

#include "config.h"   // System config (modes, pins, etc.)
#include "cube.h"     // Function declarations for cube module
#include "ring.h"     // Used for scramble sequences

// ================================================================
// MODULE STATE VARIABLES
// ================================================================

// Current cube state (facelets)
// 54 characters (3x3 cube) + null terminator
uint8_t _cube_cubelets[55] = {0};

// Flag to indicate cube state changed
bool _cube_updated = false;

// ================================================================
// TEACHING MODE VARIABLES
// ================================================================

// Whether teaching mode is active
bool _teaching_mode = false;

// Maximum number of moves in teaching sequence
#define MAX_SOLUTION_STEPS 50

// Array storing solution moves
uint8_t _solution_steps[MAX_SOLUTION_STEPS];

// Total number of steps in solution
uint8_t _solution_length = 0;

// Current step user is on
uint8_t _current_step = 0;

// ================================================================
// METRICS (Timer / Solve Tracking)
// ================================================================

bool _cube_running_metrics = false;
uint32_t _cube_start = 0;
uint32_t _cube_time = 0;
uint16_t _cube_turns = 0;

// ================================================================
// BASIC FUNCTIONS
// ================================================================

// Return cube state
uint8_t * cube_cubelets() {
    return _cube_cubelets;
}

// Check if cube updated
bool cube_updated() {
    bool ret = _cube_updated;
    _cube_updated = false;
    return ret;
}

// Reset cube (can be expanded later)
void cube_reset() {
    memset(_cube_cubelets, 0, sizeof(_cube_cubelets));
}

// ================================================================
// TEACHING MODE FUNCTIONS
// ================================================================

// Start teaching mode with given steps
void cube_start_teaching(uint8_t *steps, uint8_t length) {
    _teaching_mode = true;
    _solution_length = length;
    _current_step = 0;

    // Copy solution steps
    for (uint8_t i = 0; i < length; i++) {
        _solution_steps[i] = steps[i];
    }
}

// Stop teaching mode
void cube_stop_teaching() {
    _teaching_mode = false;
}

// Check if teaching is active
bool cube_teaching_active() {
    return _teaching_mode;
}

// Get next move user should perform
uint8_t cube_next_teaching_move() {
    if (_current_step < _solution_length) {
        return _solution_steps[_current_step];
    }
    return 0xFF; // No move
}

// ================================================================
// MOVE HANDLING (FROM HALL SENSORS)
// ================================================================

// Called when cube is rotated
void cube_move(uint8_t face, uint8_t count) {

    // Encode move: upper 4 bits = count, lower 4 bits = face
    uint8_t move = ((count & 0x0F) << 4) + (face & 0x0F);

    // ============================================================
    // TEACHING MODE LOGIC
    // ============================================================

    if (_teaching_mode) {

        // If user made correct move
        if (move == _solution_steps[_current_step]) {

            _current_step++;

            // If finished all steps → exit teaching mode
            if (_current_step >= _solution_length) {
                _teaching_mode = false;
            }

        } else {
            // Wrong move → reset progress
            _current_step = 0;
        }
    }

    // ============================================================
    // METRICS (TURN COUNT)
    // ============================================================

    if (_cube_running_metrics) {
        _cube_turns++;
    }
}

// ================================================================
// TIMER / METRICS FUNCTIONS
// ================================================================

// Start timing solve
void cube_metrics_start(uint32_t ms) {
    _cube_start = millis();
    _cube_turns = 0;
    _cube_running_metrics = true;
}

// End timing solve
void cube_metrics_end(uint32_t ms) {
    _cube_time = millis() - _cube_start;
    _cube_running_metrics = false;
}

// Get current time
uint32_t cube_time() {
    if (_cube_running_metrics) {
        return millis() - _cube_start;
    }
    return _cube_time;
}

// Get turn count
uint16_t cube_turns() {
    return _cube_turns;
}

