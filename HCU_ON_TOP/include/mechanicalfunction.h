#pragma once
#include "main.h"
#include <string>
#include "settings.h"

// toggle state flags
inline bool wingd = false;
inline bool scrapd = false;
inline bool gateBd = true;
inline bool gateTd = false;
inline bool midDescored = false;

// ==== SCORING MODE STATE ====

enum class ScoringMode {
    NONE,
    INTAKE,
    OUTTAKE,
    TOP,
    MIDDLE,
    SKILLS
};

// What the driver wants the system to do right now.
inline ScoringMode requestedMode = ScoringMode::NONE;


// Apply actual motor voltages for a given mode
inline void applyScoringMode(ScoringMode mode) {
    switch (mode) {
        case ScoringMode::INTAKE:
            conveyorR.move_voltage(12000);
            conveyorL.move_voltage(12000);
            gateT.set_value(false);
            gateB.set_value(true);
            break;

        case ScoringMode::OUTTAKE:
            conveyorR.move_voltage(-12000);
            conveyorL.move_voltage(-12000);
            gateT.set_value(false);
            gateB.set_value(true);
            break;

        case ScoringMode::TOP:
            conveyorR.move_voltage(12000);
            conveyorL.move_voltage(12000);
            gateT.set_value(true);
            gateB.set_value(true);
            break;

        case ScoringMode::MIDDLE:
            conveyorR.move_voltage(12000);
            conveyorL.move_voltage(12000);
            gateT.set_value(false);
            gateB.set_value(false);
            break;
            
        case ScoringMode::SKILLS:
            conveyorR.move_voltage(7000);
            conveyorL.move_voltage(7000);
            gateT.set_value(false);
            gateB.set_value(false);
            break;

        case ScoringMode::NONE:
        default:
            conveyorR.move_voltage(0);
            conveyorL.move_voltage(0);
            gateT.set_value(false);
            gateB.set_value(true);
            break;
    }
}

// This is now just a "request" function; the task will do the real work.
inline void setScoringMode(const std::string& mode) {
    if (mode == "INTAKE") {
        requestedMode = ScoringMode::INTAKE;
    } else if (mode == "OUTTAKE") {
        requestedMode = ScoringMode::OUTTAKE;
    } else if (mode == "TOP") {
        requestedMode = ScoringMode::TOP;
    } else if (mode == "MIDDLE") {
        requestedMode = ScoringMode::MIDDLE;
    } else if (mode == "SKILLS") {
        requestedMode = ScoringMode::SKILLS;
    } else { // "NONE" or anything else
        requestedMode = ScoringMode::NONE;
    }
}

// ==== SCORING + JAM HANDLER TASK ====
// Runs in the background and automatically unjams
inline void scoring_task(void* /*param*/) {
    enum class JamState { IDLE, OUTTAKING, INTAKING };
    JamState jamState = JamState::IDLE;

    uint32_t stateStart = pros::millis();
    uint32_t jamStart   = 0;

    // -------- TOP DELAY SEQUENCER (Gate opens first, rollers later) --------
    constexpr uint32_t TOP_ROLLER_DELAY_MS = 100;

    ScoringMode lastRequested = ScoringMode::NONE;
    bool topDelayActive = false;
    uint32_t topDelayStart = 0;
    // ----------------------------------------------------------------------

    while (true) {
        // --- MEASURE LOAD ---
        int convRCurrent = conveyorR.get_current_draw(); // mA
        int convLCurrent = conveyorL.get_current_draw(); // mA

        // Optional: prevent jam-detect during the TOP delay window
        bool allowJamDetect = !topDelayActive;

        bool jamNow = allowJamDetect &&
                      ((convRCurrent > JAM_CURRENT_MA) || (convLCurrent > JAM_CURRENT_MA));

        switch (jamState) {
            case JamState::IDLE: {
                // Detect a new driver request
                if (requestedMode != lastRequested) {
                    lastRequested = requestedMode;

                    if (requestedMode == ScoringMode::TOP) {
                        // Start TOP sequence: open gate NOW, rollers after 100ms
                        topDelayActive = true;
                        topDelayStart  = pros::millis();

                        // Gate opens immediately
                        gateT.set_value(true);
                        gateB.set_value(true);

                        // Rollers OFF during delay
                        conveyorR.move_voltage(0);
                        conveyorL.move_voltage(0);
                    } else {
                        // Any other mode cancels TOP delay
                        topDelayActive = false;
                    }
                }

                if (topDelayActive) {
                    // While waiting: keep gate open, keep rollers off
                    gateT.set_value(true);
                    gateB.set_value(true);
                    conveyorR.move_voltage(0);
                    conveyorL.move_voltage(0);

                    // After delay: start rollers, then we can exit the sequence
                    if (pros::millis() - topDelayStart >= TOP_ROLLER_DELAY_MS) {
                        topDelayActive = false;

                        // Now run actual TOP behavior (rollers on + gates open)
                        // (You can call applyScoringMode(TOP) or set directly)
                        applyScoringMode(ScoringMode::TOP);
                    }
                } else {
                    // Normal: follow requestedMode
                    applyScoringMode(requestedMode);
                }

                // ---- Jam detection (sustained spike) ----
                if (jamNow) {
                    if (jamStart == 0) jamStart = pros::millis();
                    if (pros::millis() - jamStart >= JAM_DETECT_MS) {
                        jamState   = JamState::OUTTAKING;
                        stateStart = pros::millis();

                        conveyorR.move_voltage(-12000);
                        conveyorL.move_voltage(-12000);
                    }
                } else {
                    jamStart = 0;
                }

                break;
            }

            case JamState::OUTTAKING: {
                if (pros::millis() - stateStart >= UNJAM_OUTTAKE_MS) {
                    jamState   = JamState::INTAKING;
                    stateStart = pros::millis();

                    conveyorL.move_voltage(12000);
                    conveyorR.move_voltage(12000);
                }
                break;
            }

            case JamState::INTAKING: {
                if (pros::millis() - stateStart >= UNJAM_INTAKE_MS) {
                    jamState = JamState::IDLE;
                    jamStart = 0;
                }
                break;
            }
        }

        pros::delay(10);
    }
}


// Call this ONCE from initialize() or at the start of opcontrol()
inline void start_scoring_task() {
    static pros::Task scoringTask(scoring_task, nullptr, "Scoring Task");
}

// ==== YOUR OTHER TOGGLES (unchanged) ====

inline void wing_tog() {
    wingd = !wingd;
    wing.set_value(wingd);
}

inline void scraper_tog() {
    scrapd = !scrapd;
    scraper.set_value(scrapd);
}

inline void midgoal_tog() {
    midDescored = !midDescored;
    midDescore.set_value(midDescored);
}

