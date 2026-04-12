#pragma once
#include "main.h"
#include <string>
#include "settings.h"

// toggle state flags
inline bool wingd = false;
inline bool scrapd = false;
inline bool hooded = false;
inline bool prerollered = false;

// Top roller stops for the current intake session if torque is detected
inline bool intakeTopStoppedByTorque = false;

// ==== SCORING MODE STATE ====

enum class ScoringMode
{
    NONE,
    INTAKE,
    OUTTAKE,
    TOP,
    MIDDLE,
    SKILLS
};

inline uint32_t middleStartTime = 0;
inline bool middleSequenceActive = false;
inline ScoringMode requestedMode = ScoringMode::NONE;

// Apply actual motor voltages for a given mode
inline void applyScoringMode(ScoringMode mode)
{
    switch (mode)
    {
    case ScoringMode::INTAKE:
    topStage.set_brake_mode(MOTOR_BRAKE_HOLD);
        bottomStageFull.move_voltage(12000);
        bottomStageHalf.move_voltage(12000);

        if (intakeTopStoppedByTorque)
        {
            topStage.move_voltage(0);
        }
        else
        {
            topStage.move_voltage(12000);
        }

        hoodPiston.set_value(false);
        break;

    case ScoringMode::OUTTAKE:
    topStage.set_brake_mode(MOTOR_BRAKE_COAST);
        bottomStageFull.move_voltage(-12000);
        bottomStageHalf.move_voltage(-12000);
        topStage.move_voltage(-12000);
        hoodPiston.set_value(false);
        break;

    case ScoringMode::TOP:
    topStage.set_brake_mode(MOTOR_BRAKE_COAST);
        bottomStageFull.move_voltage(12000);
        bottomStageHalf.move_voltage(12000);
        topStage.move_voltage(12000);
        hoodPiston.set_value(true);
        break;

    case ScoringMode::MIDDLE:
    topStage.set_brake_mode(MOTOR_BRAKE_COAST);
        bottomStageFull.move_voltage(12000);
        bottomStageHalf.move_voltage(12000);
        topStage.move_voltage(-12000);
        hoodPiston.set_value(true);
        break;

    case ScoringMode::SKILLS:
    topStage.set_brake_mode(MOTOR_BRAKE_COAST);
        bottomStageFull.move_voltage(6000);
        bottomStageHalf.move_voltage(6000);
        topStage.move_voltage(-6000);
        hoodPiston.set_value(true);
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
inline void setScoringMode(const std::string &mode)
{
    ScoringMode previousMode = requestedMode;

    if (mode == "INTAKE")
    {
        requestedMode = ScoringMode::INTAKE;

        // Every time we ENTER intake, start fresh torque detection
        if (previousMode != ScoringMode::INTAKE)
        {
            intakeTopStoppedByTorque = false;
        }
    }
    else if (mode == "OUTTAKE")
    {
        requestedMode = ScoringMode::OUTTAKE;
    }
    else if (mode == "TOP")
    {
        requestedMode = ScoringMode::TOP;
    }
    else if (mode == "MIDDLE")
    {
        requestedMode = ScoringMode::MIDDLE;
    }
    else if (mode == "SKILLS")
    {
        requestedMode = ScoringMode::SKILLS;
    }
    else
    {
        requestedMode = ScoringMode::NONE;
    }

    if (requestedMode == ScoringMode::MIDDLE && previousMode != ScoringMode::MIDDLE)
    {
        middleSequenceActive = true;
        middleStartTime = pros::millis();
    }

    if (requestedMode != ScoringMode::MIDDLE)
    {
        middleSequenceActive = false;
    }
}

inline void rearmTopRollerIntake()
{
    // Intentionally do nothing
}

inline void notifyIntakeReleased()
{
    // Intentionally do nothing
}

inline void resetTopRollerLatch()
{
    // Intentionally do nothing
}

// ==== SCORING + JAM HANDLER TASK ====
inline void scoring_task(void * /*param*/)
{
    enum class JamState
    {
        IDLE,
        OUTTAKING,
        INTAKING
    };

    JamState jamState = JamState::IDLE;

    uint32_t stateStart = pros::millis();
    uint32_t jamStart = 0;

    constexpr uint32_t TOP_ROLLER_DELAY_MS = 0;
    constexpr double TOP_SPIKE_TORQUE = 0.5;
    constexpr uint32_t TOP_SPIKE_MS = 50;

    uint32_t topTorqueStart = 0;

    ScoringMode lastRequested = ScoringMode::NONE;
    bool topDelayActive = false;
    uint32_t topDelayStart = 0;

    while (true)
    {
        int convRCurrent = bottomStageFull.get_current_draw();
        int convLCurrent = bottomStageHalf.get_current_draw();
        double topTorque = topStage.get_torque();

        // Always try torque detect during INTAKE
        if (requestedMode == ScoringMode::INTAKE)
        {
            if (!intakeTopStoppedByTorque)
            {
                if (topTorque >= TOP_SPIKE_TORQUE)
                {
                    if (topTorqueStart == 0)
                    {
                        topTorqueStart = pros::millis();
                    }

                    if (pros::millis() - topTorqueStart >= TOP_SPIKE_MS)
                    {
                        intakeTopStoppedByTorque = true;
                        topStage.move_voltage(0);
                    }
                }
                else
                {
                    topTorqueStart = 0;
                }
            }
        }
        else
        {
            topTorqueStart = 0;
            intakeTopStoppedByTorque = false;
        }

        bool allowJamDetect = !topDelayActive;

        bool jamDetectionEnabled =
            (requestedMode == ScoringMode::INTAKE);

        bool jamNow = jamDetectionEnabled &&
                      allowJamDetect &&
                      ((convRCurrent > JAM_CURRENT_MA) || (convLCurrent > JAM_CURRENT_MA));

        // Always command top roller from requested mode
        topStage.set_brake_mode(MOTOR_BRAKE_COAST);

        if (requestedMode == ScoringMode::TOP)
        {
            topStage.move_voltage(12000);
        }
        else if (requestedMode == ScoringMode::MIDDLE)
        {
            topStage.move_voltage(-12000);
        }
        else if (requestedMode == ScoringMode::SKILLS)
        {
            topStage.move_voltage(-7000);
        }
        else if (requestedMode == ScoringMode::OUTTAKE)
        {
            topStage.move_voltage(-12000);
        }
        else if (requestedMode == ScoringMode::INTAKE)
        {
            if (intakeTopStoppedByTorque)
            {
                topStage.move_voltage(0);
            }
            else
            {
                topStage.move_voltage(12000);
            }
        }
        else
        {
            topStage.move_voltage(0);
        }

        switch (jamState)
        {
        case JamState::IDLE:
        {
            if (requestedMode != lastRequested)
            {
                lastRequested = requestedMode;

                if (requestedMode == ScoringMode::TOP)
                {
                    topDelayActive = true;
                    topDelayStart = pros::millis();

                    hoodPiston.set_value(true);
                    bottomStageFull.move_voltage(0);
                    bottomStageHalf.move_voltage(0);
                }
                else
                {
                    topDelayActive = false;
                }
            }

            if (topDelayActive)
            {
                hoodPiston.set_value(true);
                bottomStageFull.move_voltage(0);
                bottomStageHalf.move_voltage(0);

                if (pros::millis() - topDelayStart >= TOP_ROLLER_DELAY_MS)
                {
                    topDelayActive = false;
                }
            }
            else
            {
                switch (requestedMode)
                {
                case ScoringMode::INTAKE:
                    bottomStageFull.move_voltage(12000);
                    bottomStageHalf.move_voltage(12000);
                    hoodPiston.set_value(false);
                    break;

                case ScoringMode::OUTTAKE:
                    bottomStageFull.move_voltage(-12000);
                    bottomStageHalf.move_voltage(-12000);
                    hoodPiston.set_value(false);
                    break;

                case ScoringMode::TOP:
                    bottomStageFull.move_voltage(12000);
                    bottomStageHalf.move_voltage(12000);
                    hoodPiston.set_value(true);
                    break;

                case ScoringMode::MIDDLE:
                    if (middleSequenceActive && (pros::millis() - middleStartTime < 100))
                    {
                        bottomStageFull.move_voltage(-4000);
                        bottomStageHalf.move_voltage(-4000);
                    }
                    else
                    {
                        middleSequenceActive = false;
                        bottomStageFull.move_voltage(12000);
                        bottomStageHalf.move_voltage(12000);
                    }
                    hoodPiston.set_value(true);
                    break;

                case ScoringMode::SKILLS:
                    bottomStageFull.move_voltage(6000);
                    bottomStageHalf.move_voltage(6000);
                    hoodPiston.set_value(true);
                    break;

                case ScoringMode::NONE:
                default:
                    bottomStageFull.move_voltage(0);
                    bottomStageHalf.move_voltage(0);
                    hoodPiston.set_value(false);
                    break;
                }
            }

            if (jamNow)
            {
                if (jamStart == 0)
                {
                    jamStart = pros::millis();
                }

                if (pros::millis() - jamStart >= JAM_DETECT_MS)
                {
                    jamState = JamState::OUTTAKING;
                    stateStart = pros::millis();

                    bottomStageFull.move_voltage(-12000);
                    bottomStageHalf.move_voltage(-12000);
                }
            }
            else
            {
                jamStart = 0;
            }

            break;
        }

        case JamState::OUTTAKING:
        {
            bottomStageFull.move_voltage(-12000);
            bottomStageHalf.move_voltage(-12000);

            if (pros::millis() - stateStart >= UNJAM_OUTTAKE_MS)
            {
                jamState = JamState::INTAKING;
                stateStart = pros::millis();
            }
            break;
        }

        case JamState::INTAKING:
        {
            bottomStageFull.move_voltage(12000);
            bottomStageHalf.move_voltage(12000);

            if (pros::millis() - stateStart >= UNJAM_INTAKE_MS)
            {
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
inline void start_scoring_task()
{
    static pros::Task scoringTask(scoring_task, nullptr, "Scoring Task");
}

// ==== YOUR OTHER TOGGLES ====

inline void wing_tog()
{
    wingd = !wingd;
    wing.set_value(wingd);
}

inline void scraper_tog()
{
    scrapd = !scrapd;
    scraper.set_value(scrapd);
}

inline void preroller_tog()
{
    prerollered = !prerollered;
    prerollerLift.set_value(prerollered);
}