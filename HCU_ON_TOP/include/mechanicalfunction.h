#pragma once
#include "main.h"
#include <string>
#include "settings.h"

// toggle state flags
inline bool wingd = false;
inline bool scrapd = false;
inline bool hooded = false;
inline bool prerollered = false;

// ==== TOP ROLLER JAM LATCH STATE ====
// Once true, the top roller stays off during INTAKE until another button resets it.
inline bool topRollerLatchedOff = false;

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
            bottomStageFull.move_voltage(12000);
            bottomStageHalf.move_voltage(12000);

            // If jam-latched, keep top roller off even if R1 is pressed again
            if (topRollerLatchedOff) {
                topStage.move_voltage(0);
            } else {
                topStage.move_voltage(12000);
            }

            hoodPiston.set_value(false);
            break;

        case ScoringMode::OUTTAKE:
            bottomStageFull.move_voltage(-12000);
            bottomStageHalf.move_voltage(-12000);
            topStage.move_voltage(-12000);
            hoodPiston.set_value(false);
            break;

        case ScoringMode::TOP:
            bottomStageFull.move_voltage(12000);
            bottomStageHalf.move_voltage(12000);
            topStage.move_voltage(12000);
            hoodPiston.set_value(true);
            break;

        case ScoringMode::MIDDLE:
            bottomStageFull.move_voltage(12000);
            bottomStageHalf.move_voltage(12000);
            topStage.move_voltage(-12000);
            hoodPiston.set_value(false);
            break;
            
        case ScoringMode::SKILLS:
            bottomStageFull.move_voltage(7000);
            bottomStageHalf.move_voltage(7000);
            topStage.move_voltage(-7000);
            hoodPiston.set_value(false);
            break;

        case ScoringMode::NONE:
        default:
            bottomStageFull.move_voltage(0);
            bottomStageHalf.move_voltage(0);
            topStage.move_voltage(0);
            hoodPiston.set_value(false);
            break;
    }
}

// This only changes the requested mode.
// It does NOT clear the jam latch.
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

// R1 is NOT allowed to clear the jam latch anymore
inline void rearmTopRollerIntake() {
    // Intentionally do nothing
}

// No longer needed for R1 rearm logic
inline void notifyIntakeReleased() {
    // Intentionally do nothing
}

// Use this from another button to re-enable the top roller
inline void resetTopRollerLatch() {
    topRollerLatchedOff = false;
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

    // -------- TOP ROLLER TORQUE JAM SETTINGS (INTAKE only) --------
    constexpr double TOP_SPIKE_TORQUE = 0.4;   // lower if it never triggers
    constexpr uint32_t TOP_SPIKE_MS   = 50;    // sustained torque time

    uint32_t topTorqueStart = 0;

    ScoringMode lastRequested = ScoringMode::NONE;
    bool topDelayActive = false;
    uint32_t topDelayStart = 0;
    // ----------------------------------------------------------------------

    while (true) {
        // --- MEASURE LOAD ---
        int convRCurrent = bottomStageFull.get_current_draw(); // mA
        int convLCurrent = bottomStageHalf.get_current_draw(); // mA
        double topTorque = topStage.get_torque();              // Nm

        // ---- Top roller jam detection (INTAKE mode only) ----
        if (requestedMode == ScoringMode::INTAKE && !topRollerLatchedOff) {
            if (topTorque >= TOP_SPIKE_TORQUE) {
                if (topTorqueStart == 0) {
                    topTorqueStart = pros::millis();
                }

                if (pros::millis() - topTorqueStart >= TOP_SPIKE_MS) {
                    topRollerLatchedOff = true;
                    topStage.move_voltage(0); // stop immediately
                }
            } else {
                topTorqueStart = 0;
            }
        } else {
            topTorqueStart = 0;
        }

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
                        // Start TOP sequence: open gate NOW, rollers after delay
                        topDelayActive = true;
                        topDelayStart  = pros::millis();

                        // Gate opens immediately
                        hoodPiston.set_value(true);

                        // Rollers OFF during delay
                        bottomStageFull.move_voltage(0);
                        bottomStageHalf.move_voltage(0);
                        topStage.move_voltage(0);
                    } else {
                        // Any other mode cancels TOP delay
                        topDelayActive = false;
                    }
                }

                if (topDelayActive) {
                    // While waiting: keep gate open, keep rollers off
                    hoodPiston.set_value(true);
                    bottomStageFull.move_voltage(0);
                    bottomStageHalf.move_voltage(0);
                    topStage.move_voltage(0);

                    // After delay: start rollers
                    if (pros::millis() - topDelayStart >= TOP_ROLLER_DELAY_MS) {
                        topDelayActive = false;
                        applyScoringMode(ScoringMode::TOP);
                    }
                } else {
                    // Normal: follow requested mode
                    applyScoringMode(requestedMode);
                }

                // ---- Bottom conveyor jam detection ----
                if (jamNow) {
                    if (jamStart == 0) jamStart = pros::millis();

                    if (pros::millis() - jamStart >= JAM_DETECT_MS) {
                        jamState   = JamState::OUTTAKING;
                        stateStart = pros::millis();

                        bottomStageFull.move_voltage(-12000);
                        bottomStageHalf.move_voltage(-12000);
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

                    bottomStageFull.move_voltage(12000);
                    bottomStageHalf.move_voltage(12000);
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

// ==== YOUR OTHER TOGGLES ====

inline void wing_tog() {
    wingd = !wingd;
    wing.set_value(wingd);
}

inline void scraper_tog() {
    scrapd = !scrapd;
    scraper.set_value(scrapd);
}

inline void preroller_tog() {
    prerollered = !prerollered;
    prerollerLift.set_value(prerollered);
}