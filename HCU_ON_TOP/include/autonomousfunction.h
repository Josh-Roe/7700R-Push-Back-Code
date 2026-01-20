#pragma once
#include "main.h"
#include "settings.h"
#include <cmath>
#include <algorithm>

// ========================================================================
// =======================  RAMSETE CONTROLLER  ===========================
// ========================================================================

// --------------------- Public & internal params -------------------------

inline void moveDistance(
    double inches,
    int timeout,
    double maxSpeed = 127,
    double minSpeed = 5.0   // NEW: minimum commanded speed
) {
    const double kP = MOVE_DISTANCE_P;
    const double kD = LATERAL_KD;

    const double tolerance         = LATERAL_SMALL_ERROR;
    const int    smallErrorTimeout = LATERAL_SMALL_ERROR_TIMEOUT;
    const int    loopDelay         = 10;

    lemlib::Pose start = chassis.getPose();
    const double DEG_TO_RAD = M_PI / 180.0;
    double theta0Rad = start.theta * DEG_TO_RAD;

    // LemLib frame: heading 0° = +Y
    double forwardX = std::sin(theta0Rad);
    double forwardY = std::cos(theta0Rad);

    double error     = inches;
    double prevError = error;
    double output    = 0.0;

    uint32_t startTime             = pros::millis();
    uint32_t withinSmallErrorStart = 0;

    while (pros::millis() - startTime < static_cast<uint32_t>(timeout)) {
        lemlib::Pose cur = chassis.getPose();

        double dx = cur.x - start.x;
        double dy = cur.y - start.y;
        double traveled = dx * forwardX + dy * forwardY;

        error = inches - traveled;

        // small-error timeout
        if (std::fabs(error) < tolerance) {
            if (withinSmallErrorStart == 0)
                withinSmallErrorStart = pros::millis();
            if (pros::millis() - withinSmallErrorStart >=
                static_cast<uint32_t>(smallErrorTimeout))
                break;
        } else {
            withinSmallErrorStart = 0;
        }

        // PD controller
        double derivative = 0.0;
        if (std::fabs(error) > tolerance * 2.0) {
            derivative = error - prevError; // dt baked into kD
        }
        prevError = error;

        output = kP * error + kD * derivative;

        // clamp to max
        if (output > maxSpeed) output = maxSpeed;
        if (output < -maxSpeed) output = -maxSpeed;

        // enforce minimum speed (only if we're not basically done)
        if (std::fabs(output) < minSpeed && std::fabs(error) > tolerance) {
            output = (output >= 0 ? minSpeed : -minSpeed);
        }

        chassis.tank((int)output, (int)output, true);
        pros::delay(loopDelay);
    }

    chassis.tank(0, 0, true);
}


inline pros::Task* imuFixTask = nullptr;

inline void fixIMURotation() {
    float theta = imu.get_rotation();
    chassis.setPose({chassis.getPose().x, chassis.getPose().y, theta + chassis.getPose().theta});

    float initTheta = chassis.getPose().theta;

    while (true) {
        theta = imu.get_rotation() * IMU_SCALE + initTheta;
        chassis.setPose({chassis.getPose().x, chassis.getPose().y, theta});
        pros::delay(10);
    }
}

inline void startIMURotationTask() {
    if (imuFixTask == nullptr) {
        imuFixTask = new pros::Task([](){
            fixIMURotation();
        });
    }
}



// ======================================================
// SIMPLE PID TURN USING SCALED IMU ROTATION + TIMEOUT
// ======================================================
inline void pidTurnToAngle(double targetDeg) {
    constexpr double kP = 0.8;
    constexpr double kI = 0;
    constexpr double kD = 0;
    constexpr double MAX_OUTPUT = 40.0; // ~20%
    constexpr double TOL_DEG    = 0.3;
    constexpr int    LOOP_MS    = 10;
    constexpr double DT         = LOOP_MS / 1000.0;

    // Hard safety timeout so we don't spin forever
    constexpr int MAX_TIME_MS = 9000; // 11 seconds for 720°
    int elapsedMs = 0;

    double error      = 0;
    double lastError  = 0;
    double integral   = 0;

    // optional: zero pose for this turn (odom, not IMU)
    chassis.setPose(0, 0, 0);

    while (true) {
        double current = imu.get_rotation();  // IMPORTANT: rotation, not heading
        error = targetDeg - current;

        if (fabs(error) < TOL_DEG) {
            pros::lcd::print(0, "PID done: err=%.3f", error);
            break;
        }

        // time-based escape
        if (elapsedMs >= MAX_TIME_MS) {
            pros::lcd::print(0, "PID timeout: curr=%.1f tgt=%.1f", current, targetDeg);
            break;
        }

        integral += error;
        // (optional) anti-windup:
        if (integral > 5000)  integral = 5000;
        if (integral < -5000) integral = -5000;

        double derivative = (error - lastError) / DT;

        double output = kP * error + kI * integral + kD * derivative;
        output = std::clamp(output, -MAX_OUTPUT, MAX_OUTPUT);

        chassis.tank(output, -output, true);

        lastError = error;
        pros::delay(LOOP_MS);
        elapsedMs += LOOP_MS;
    }

    chassis.tank(0, 0, true);
    pros::delay(300);
}

// ======================================================
// SINGLE ODOMETRY CALIBRATION TRIAL
// ======================================================
inline bool runOdomCalibrationTrial(double targetDeg,
                                    double& VO_out,
                                    double& HO_out,
                                    double& TW_out) {
    constexpr double DEG2RAD             = M_PI / 180.0;
    constexpr double ODOM_WHEEL_DIAM_IN  = 2.0;            // NEW_2 wheels
    constexpr double DRIVE_WHEEL_DIAM_IN = WHEEL_DIAMETER; // from settings.h

    // --- zero sensors for this trial ---
    horizontalEnc.reset();
    verticalEnc.reset();
    leftMotors.tare_position();
    rightMotors.tare_position();

    // Zero IMU rotation quickly (no full recal)
    imu.tare_rotation();
    pros::delay(10);

    pros::lcd::print(1, "Turn to: %.1f", targetDeg);

    // --- do the PID turn for this trial ---
    pidTurnToAngle(targetDeg);

    // --- gather data ---
    double dThetaDeg = imu.get_rotation();   // same source as PID
    double dThetaRad = dThetaDeg * DEG2RAD;      // radians

    pros::lcd::print(2, "dTheta=%.1f", dThetaDeg);

    // Guard against very small / bad rotation
    if (!std::isfinite(dThetaRad) || std::fabs(dThetaRad) < 1e-6) {
        pros::lcd::print(3, "bad dTheta: %.3f", dThetaDeg);
        return false; // failed trial
    }

    double vDeg     = verticalEnc.get_position()  / 100.0;  // rotation sensor degrees
    double hDeg     = horizontalEnc.get_position()/ 100.0;  // rotation sensor degrees

    double leftDeg  = leftMotors.get_position();   // avg motor degrees (left)
    double rightDeg = rightMotors.get_position();  // avg motor degrees (right)

    // --- convert rotation sensor degrees -> inches of travel ---
    double odomCirc = M_PI * ODOM_WHEEL_DIAM_IN;
    double vIn      = (vDeg / 360.0) * odomCirc; // vertical wheel travel
    double hIn      = (hDeg / 360.0) * odomCirc; // horizontal wheel travel

    // --- convert drive motor degrees -> inches of travel ---
    double driveCirc = M_PI * DRIVE_WHEEL_DIAM_IN;
    double leftIn    = (leftDeg  / 360.0) * driveCirc;
    double rightIn   = (rightDeg / 360.0) * driveCirc;

    // Extra safety against NaNs
    if (!std::isfinite(vIn) || !std::isfinite(hIn) ||
        !std::isfinite(leftIn) || !std::isfinite(rightIn)) {
        pros::lcd::print(3, "NaN wheel data");
        return false;
    }

    // --- compute geometry for THIS trial ---
    double VO = vIn / dThetaRad;                 // vertical offset
    double HO = hIn / dThetaRad;                 // horizontal offset
    double TW = (std::fabs(leftIn) + std::fabs(rightIn)) / std::fabs(dThetaRad);


    if (!std::isfinite(VO) || !std::isfinite(HO) || !std::isfinite(TW)) {
        pros::lcd::print(3, "NaN VO/HO/TW");
        return false;
    }

    VO_out = VO;
    HO_out = HO;
    TW_out = TW;
    return true;
}


// ========================================================================
// ODOMETRY GEOMETRY CALIBRATION
// ========================================================================

inline void resetOnPark(){
      chassis.setPose({float(72-distance_sensor_left.get() / 25.4f - 5.5), float(72-(distance_sensor_back.get() / 25.4f) + 0.5), 180});
        pros::lcd::print(5, "X: %.2f Y: %.2f", chassis.getPose().x, chassis.getPose().y);
}


inline float normalizeDeg180(float deg) {
    while (deg > 180.0f) deg -= 360.0f;
    while (deg <= -180.0f) deg += 360.0f;
    return deg;
}

inline bool near(float a, float target, float tolDeg = 3.0f) {
    return fabsf(a - target) <= tolDeg;
}

inline void distanceResetGoal() {
    // Field walls in your coordinate system
    constexpr float X_WALL_POS =  72.0f; // right wall
    constexpr float X_WALL_NEG = -72.0f; // left wall
    constexpr float Y_WALL_POS =  72.0f; // top wall

    // Your physical offsets (inches) — keep these the same ones you tuned
    // xOffset affects X computed from FRONT sensor
    // yOffset affects Y computed from LEFT/RIGHT sensor
    const float xOffset = 0.5f;
    const float yOffset = 5.5f;

    float theta = normalizeDeg180(chassis.getPose().theta);

    // Distances in inches
    const float dback = distance_sensor_back.get() / 25.4f;
    const float dLeft  = distance_sensor_left.get()  / 25.4f;
    const float dRight = distance_sensor_right.get() / 25.4f;

    auto pose = chassis.getPose();

    // Optional sanity check
    if (!(fabsf(pose.x) <= 80 && fabsf(pose.y) <= 80)) return;

    // ---------------- θ ≈ +90 : ( +29, ±48, +90 ) ----------------
    // front -> right wall (x=+72)
    // left  -> top wall   (y=+72)
    if (near(theta, 90.0f)) {
        const float newX = (X_WALL_POS + dback) + xOffset;
        const float newY = (Y_WALL_POS - dLeft ) - yOffset;
        chassis.setPose({newX, newY, theta});
        return;
    }

    // ---------------- θ ≈ -90 : ( -29, ±48, -90 ) ----------------
    // front -> left wall (x=-72)
    // right -> top wall  (y=+72)
    if (near(theta, -90.0f)) {
        const float newX = (X_WALL_NEG - dback) - xOffset;
        const float newY = (Y_WALL_POS - dRight) - yOffset;
        chassis.setPose({newX, newY, theta});
        return;
    }

    // If you ever get here, you're not in one of your allowed headings.
}




inline void calibrateOdomGeometry() {
    constexpr double TARGET_ROTATION_DEG = 720.0;  // two full spins
    constexpr int    NUM_TRIALS          = 4;      // e.g. +720, -720, +720, -720

    // Full IMU reset ONCE and wait for calibration
    imu.reset();
    while (imu.is_calibrating()) {
        pros::delay(20);
    }
    imu.tare_rotation(); // start rotation from 0 deg

    double voSum = 0.0;
    double hoSum = 0.0;
    double twSum = 0.0;
    int    validTrials = 0;

    for (int i = 0; i < NUM_TRIALS; i++) {
        // Alternate direction: +720, -720, +720, -720...
        double sign   = (i % 2 == 0) ? 1.0 : -1.0;
        double target = sign * TARGET_ROTATION_DEG;

        pros::lcd::print(0, "Trial %d tgt=%.1f", i, target);

        double VO = 0.0;
        double HO = 0.0;
        double TW = 0.0;

        bool ok = runOdomCalibrationTrial(target, VO, HO, TW);
        if (!ok) {
            pros::lcd::print(3, "CALIB FAIL: trial %d", i);
            continue;
        }

        voSum += VO;
        hoSum += HO;
        twSum += TW;
        validTrials++;
    }

    if (validTrials == 0) {
        pros::lcd::print(3, "CALIB FAIL: no trials");
        return;
    }

    double VO_avg = voSum / validTrials;
    double HO_avg = hoSum / validTrials;
    double TW_avg = twSum / validTrials;

    pros::lcd::print(3, "VO: %.3f  HO: %.3f  TW: %.3f", VO_avg, HO_avg, TW_avg);
}



struct RamseteParamsPublic {
    bool forwards = true;     // true = forward, false = backwards
    double minSpeed = 0;      // minimum motor output
    double maxSpeed = 127;    // maximum motor output
    bool async = false;       // async = return immediately, sync = wait until settled
};

struct RamseteParamsInternal {
    double targetX;
    double targetY;
    bool useFinalHeading;
    double targetThetaDeg;
    bool backwards;
    int timeout;
    double minSpeed;
    double maxSpeed;
};


// ================== RAMSETE CORE (your existing code) ==================
inline void _ramseteCore(double targetX, double targetY,
                         bool useFinalHeading, double targetThetaDeg,
                         bool backwards,
                         int timeout, double minSpeed, double maxSpeed) {
    // Ramsete-style params
    const double b    = BETA;
    const double zeta = ZETA;

    // "Virtual" forward velocity gain
    const double kV      = KV;
    const double maxVCmd = maxSpeed * 0.6; // don't try to use full power from v_d
    const double kMax    = KMAX;

    // Exit conditions
    const double posTolInches   = LATERAL_SMALL_ERROR / 2;
    const double thetaTolDeg    = ANGULAR_SMALL_ERROR / 2;
    const int    smallErrorTime = LATERAL_SMALL_ERROR_TIMEOUT;
    const int    loopDelay      = 10;

    const double DEG2RAD = M_PI / 180.0;
    const double RAD2DEG = 180.0 / M_PI;

    uint32_t startTime             = pros::millis();
    uint32_t withinSmallErrorStart = 0;

    while (pros::millis() - startTime < static_cast<uint32_t>(timeout)) {
        // ---- Current pose ----
        lemlib::Pose cur = chassis.getPose();
        double x     = cur.x;
        double y     = cur.y;
        double theta = cur.theta * DEG2RAD; // radians

        // ---- Global error ----
        double dx = targetX - x;
        double dy = targetY - y;

        // Pick desired heading
        double targetThetaRad;
        if (useFinalHeading) {
            targetThetaRad = targetThetaDeg * DEG2RAD;
        } else {
            // 0° = +Y ⇒ bearing = atan2(dx, dy)
            if (!backwards) {
                targetThetaRad = std::atan2(dx, dy);     // face toward point
            } else {
                targetThetaRad = std::atan2(-dx, -dy);   // face away, drive backward
            }
        }

        // Angle error wrapped to [-pi, pi]
        double dtheta = targetThetaRad - theta;
        while (dtheta >  M_PI) dtheta -= 2.0 * M_PI;
        while (dtheta < -M_PI) dtheta += 2.0 * M_PI;

        // ---- Transform error into robot frame (LemLib frame) ----
        // x = left/right (right +), y = forward/back (forward +), 0° = +Y
        // forward unit = (sinθ, cosθ)
        // left    unit = (-cosθ, sinθ)
        double cosT = std::cos(theta);
        double sinT = std::sin(theta);

        double x_e    = dx * sinT + dy * cosT;      // forward error
        double y_e    = -dx * cosT + dy * sinT;     // left error
        double theta_e = dtheta;

        // ---- Tolerances ----
        double posErr      = std::sqrt(x_e * x_e + y_e * y_e);
        double thetaErrDeg = std::fabs(theta_e * RAD2DEG);

        bool posGood   = posErr < posTolInches;
        bool thetaGood = (!useFinalHeading) || (thetaErrDeg < thetaTolDeg);

        if (posGood && thetaGood) {
            if (withinSmallErrorStart == 0)
                withinSmallErrorStart = pros::millis();
            if (pros::millis() - withinSmallErrorStart >= (uint32_t)smallErrorTime)
                break; // done
        } else {
            withinSmallErrorStart = 0;
        }

        // ================== HEADING-ONLY MODE FOR LARGE ANGLE ERROR ==================
        {
            // Threshold where we stop trying to drive and just fix heading
            const double HEADING_ONLY_DEG = 90.0;   // tune if needed
            // Proportional gain: deg of error -> motor command
            const double K_TURN_DEG       = 0.8;    // tune 0.5–1.0

            double signedThetaErrDeg = theta_e * RAD2DEG;
            double absThetaErrDeg    = std::fabs(signedThetaErrDeg);

            if (absThetaErrDeg > HEADING_ONLY_DEG) {
                // Simple P controller on heading
                double turnCmd = absThetaErrDeg * K_TURN_DEG;

                // Clamp to available range
                if (turnCmd > maxSpeed) turnCmd = maxSpeed;
                if (minSpeed > 0 && turnCmd < minSpeed) turnCmd = minSpeed;

                // Direction: sign decides spin direction (shortest path because of angle wrap)
                turnCmd *= (signedThetaErrDeg > 0 ? 1.0 : -1.0);

                double leftCmd  = -turnCmd;
                double rightCmd =  turnCmd;

                chassis.tank(static_cast<int>(leftCmd), static_cast<int>(rightCmd), true);
                pros::delay(loopDelay);
                // Skip Ramsete math this loop; keep turning until heading is reasonable
                continue;
            }
        }
        // ============================================================================

        // ---- "Virtual" desired velocities ----
        // Forward speed magnitude grows with distance
        double v_d = kV * posErr;
        if (!backwards) {
            if (v_d > maxVCmd) v_d = maxVCmd;
        } else {
            v_d = -v_d;
            if (v_d < -maxVCmd) v_d = -maxVCmd;
        }

        double w_d = 0.0;

        // ---- Ramsete gain (capped) ----
        double k_raw = 2.0 * zeta * std::sqrt(b * v_d * v_d);
        double k     = std::min(k_raw, kMax);

        double sinThetaE  = std::sin(theta_e);
        double sincThetaE = (std::fabs(theta_e) < 1e-6) ? 1.0 : (sinThetaE / theta_e);

        // ---- Ramsete control law ----
        double v = v_d * std::cos(theta_e) + k * x_e;
        double w = w_d + k * theta_e + b * v_d * sincThetaE * y_e;

        // Forward-only mode: don't let v go negative
        // if (!backwards && v < 0) {
        //     v = 0;
        // }

        // ---- Convert to left/right commands ----
        double leftCmd  = v - w * (TRACKWIDTH / 2.0);
        double rightCmd = v + w * (TRACKWIDTH / 2.0);

        // Normalize to ±maxSpeed
        double maxMag = std::max(std::fabs(leftCmd), std::fabs(rightCmd));
        if (maxMag > maxSpeed && maxMag > 1e-6) {
            double scale = maxSpeed / maxMag;
            leftCmd  *= scale;
            rightCmd *= scale;
        }

        // Small deadband so it doesn't stall just outside tolerance
        if (posErr > posTolInches && minSpeed > 0) {
            if (std::fabs(leftCmd) < minSpeed) {
                leftCmd = (leftCmd >= 0 ? 1 : -1) * minSpeed;
            }
            if (std::fabs(rightCmd) < minSpeed) {
                rightCmd = (rightCmd >= 0 ? 1 : -1) * minSpeed;
            }
        }

        chassis.tank(static_cast<int>(leftCmd), static_cast<int>(rightCmd), true);
        pros::delay(loopDelay);
    }

    chassis.tank(0, 0, true);
}

// ================== ASYNC WRAPPER + WAIT LOGIC ==================

// Forward declaration of task function so ensureRamseteTask can see it
inline void RamseteTaskFn(void*);

// ================== SINGLE PERSISTENT RAMSETE TASK ==================

inline pros::Task*           RamseteTask   = nullptr;
inline bool                  RamseteActive = false;   // currently executing a command
inline RamseteParamsInternal RamseteCmd{};            // latest command
inline bool                  RamseteHasCmd = false;   // command waiting/being run

// Ensure the persistent Ramsete task exists
inline void ensureRamseteTask() {
    if (RamseteTask == nullptr) {
        RamseteTask = new pros::Task(RamseteTaskFn, nullptr, "Ramsete");
        // Debug: if creation fails (out of task slots), warn on LCD
        if (RamseteTask == nullptr) {
            pros::lcd::print(7, "Ramsete task create FAIL");
        }
    }
}

// "Cancel" just means: no command is currently queued/running
inline void cancelRamsete() {
    RamseteHasCmd  = false;
    RamseteActive  = false;
    chassis.tank(0, 0, true);
}

// Ramsete task function: runs forever, executes commands when present
inline void RamseteTaskFn(void* /*unused*/) {
    while (true) {
        if (RamseteHasCmd) {
            RamseteActive = true;

            _ramseteCore(
                RamseteCmd.targetX,
                RamseteCmd.targetY,
                RamseteCmd.useFinalHeading,
                RamseteCmd.targetThetaDeg,
                RamseteCmd.backwards,
                RamseteCmd.timeout,
                RamseteCmd.minSpeed,
                RamseteCmd.maxSpeed
            );

            RamseteHasCmd  = false; // done with this command
            RamseteActive  = false;
        }

        pros::delay(5);
    }
}


// ================== RAMSETE PUBLIC API (SYNC + ASYNC) ==================

// go to a point (no final heading)
inline void RamseteToPoint(double targetX, double targetY,
                           int timeout = 2000,
                           RamseteParamsPublic params = {}) {
    ensureRamseteTask();

    // If task couldn't be created (out of slots), just run synchronously
    if (RamseteTask == nullptr) {
        _ramseteCore(
            targetX, targetY,
            false,          // useFinalHeading
            0.0,
            !params.forwards,
            timeout,
            params.minSpeed,
            params.maxSpeed > 0 ? params.maxSpeed : 127.0
        );
        return;
    }

    // wait for any previous command to finish
    while (RamseteActive) pros::delay(5);

    // fill command struct
    RamseteCmd.targetX         = targetX;
    RamseteCmd.targetY         = targetY;
    RamseteCmd.useFinalHeading = false;
    RamseteCmd.targetThetaDeg  = 0.0;
    RamseteCmd.backwards       = !params.forwards;      // backwards if forwards = false
    RamseteCmd.timeout         = timeout;
    RamseteCmd.minSpeed        = params.minSpeed;
    RamseteCmd.maxSpeed        = (params.maxSpeed > 0) ? params.maxSpeed : 127.0;

    RamseteHasCmd = true;

    // sync mode: block until command finishes
    if (!params.async) {
        while (RamseteHasCmd) pros::delay(5);
    }
}

// go to a pose (point + heading)
inline void RamseteToPose(double targetX, double targetY, double targetThetaDeg,
                          int timeout = 2000,
                          RamseteParamsPublic params = {}) {
    ensureRamseteTask();

    if (RamseteTask == nullptr) {
        _ramseteCore(
            targetX, targetY,
            true,
            targetThetaDeg,
            !params.forwards,
            timeout,
            params.minSpeed,
            params.maxSpeed > 0 ? params.maxSpeed : 127.0
        );
        return;
    }

    while (RamseteActive) pros::delay(5);

    RamseteCmd.targetX         = targetX;
    RamseteCmd.targetY         = targetY;
    RamseteCmd.useFinalHeading = true;
    RamseteCmd.targetThetaDeg  = targetThetaDeg;
    RamseteCmd.backwards       = !params.forwards;
    RamseteCmd.timeout         = timeout;
    RamseteCmd.minSpeed        = params.minSpeed;
    RamseteCmd.maxSpeed        = (params.maxSpeed > 0) ? params.maxSpeed : 127.0;

    RamseteHasCmd = true;

    if (!params.async) {
        while (RamseteHasCmd) pros::delay(5);
    }
}
// =============================
// SE2 PARAM STRUCTS
// =============================
struct SE2ParamsPublic {
    bool   forwards = true;
    double minSpeed = 0;
    double maxSpeed = 127;
    bool   async    = false;
};

struct SE2ParamsInternal {
    double targetX;
    double targetY;
    bool   useFinalHeading;
    double targetThetaDeg;
    bool   backwards;
    int    timeout;
    double minSpeed;
    double maxSpeed;
};

// =============================
// SE2 CORE USING KX, KY, KTH
// =============================
inline void _se2Core(double targetX, double targetY,
                     bool useFinalHeading, double targetThetaDeg,
                     bool backwards,
                     int timeout, double minSpeed, double maxSpeed)
{
    const double posTolInches   = LATERAL_SMALL_ERROR/2;
    const double thetaTolDeg    = ANGULAR_SMALL_ERROR/2;
    const int    smallErrorTime = LATERAL_SMALL_ERROR_TIMEOUT;
    const int    loopDelay      = 10;

    const double DEG2RAD = M_PI / 180.0;
    const double RAD2DEG = 180.0 / M_PI;

    uint32_t startTime             = pros::millis();
    uint32_t withinSmallErrorStart = 0;

    while (pros::millis() - startTime < (uint32_t)timeout) {
        lemlib::Pose cur = chassis.getPose();
        double x     = cur.x;
        double y     = cur.y;
        double theta = cur.theta * DEG2RAD;

        double dx = targetX - x;
        double dy = targetY - y;

        double targetThetaRad;
        if (useFinalHeading) {
            targetThetaRad = targetThetaDeg * DEG2RAD;
        } else {
            if (!backwards)
                targetThetaRad = std::atan2(dx, dy);
            else
                targetThetaRad = std::atan2(-dx, -dy);
        }

        double dtheta = targetThetaRad - theta;
        while (dtheta >  M_PI) dtheta -= 2.0 * M_PI;
        while (dtheta < -M_PI) dtheta += 2.0 * M_PI;

        double cosT = std::cos(theta);
        double sinT = std::sin(theta);

        double forwardErr = dx * sinT + dy * cosT;
        double leftErr    = -dx * cosT + dy * sinT;

        double rho   = std::sqrt(forwardErr * forwardErr + leftErr * leftErr);
        double alpha = std::atan2(leftErr, forwardErr);
        double delta = dtheta;

        double posErr      = rho;
        double thetaErrDeg = std::fabs(delta * RAD2DEG);

        bool posGood   = posErr < posTolInches;
        bool thetaGood = (!useFinalHeading) || (thetaErrDeg < thetaTolDeg);

        if (posGood && thetaGood) {
            if (withinSmallErrorStart == 0)
                withinSmallErrorStart = pros::millis();
            if (pros::millis() - withinSmallErrorStart >= (uint32_t)smallErrorTime)
                break;
        } else {
            withinSmallErrorStart = 0;
        }

        // ---- SE2 control (KX, KY, KTH) ----
        double v = KX * rho * std::cos(alpha);

        double sinA = std::sin(alpha);
        double cosA = std::cos(alpha);
        double alphaSafe = (std::fabs(alpha) < 1e-6) ? 1.0 : alpha;

        double w = KY * alpha
                 + KX * (sinA * cosA / alphaSafe) * (alpha + KTH * delta);

        if (backwards) v = -v;

        double leftCmd  = v - w * (TRACKWIDTH / 2.0);
        double rightCmd = v + w * (TRACKWIDTH / 2.0);

        double maxMag = std::max(std::fabs(leftCmd), std::fabs(rightCmd));
        if (maxMag > maxSpeed && maxMag > 1e-6) {
            double scale = maxSpeed / maxMag;
            leftCmd  *= scale;
            rightCmd *= scale;
        }

        if (rho > posTolInches && minSpeed > 0) {
            if (std::fabs(leftCmd) < minSpeed)
                leftCmd = (leftCmd >= 0 ? 1 : -1) * minSpeed;
            if (std::fabs(rightCmd) < minSpeed)
                rightCmd = (rightCmd >= 0 ? 1 : -1) * minSpeed;
        }

        chassis.tank(static_cast<int>(leftCmd), static_cast<int>(rightCmd), true);
        pros::delay(loopDelay);
    }

    chassis.tank(0, 0, true);
}

// =============================
// SINGLE PERSISTENT SE2 TASK
// =============================
inline pros::Task*       SE2Task   = nullptr;
inline bool              SE2Active = false;
inline SE2ParamsInternal SE2Cmd{};
inline bool              SE2HasCmd = false;

inline void SE2TaskFn(void*);

inline void ensureSE2Task() {
    if (SE2Task == nullptr) {
        SE2Task = new pros::Task(SE2TaskFn, nullptr, "SE2");
    }
}

inline void cancelSE2() {
    SE2HasCmd  = false;
    SE2Active  = false;
    chassis.tank(0, 0, true);
}

inline void SE2TaskFn(void*) {
    while (true) {
        if (SE2HasCmd) {
            SE2Active = true;

            _se2Core(
                SE2Cmd.targetX,
                SE2Cmd.targetY,
                SE2Cmd.useFinalHeading,
                SE2Cmd.targetThetaDeg,
                SE2Cmd.backwards,
                SE2Cmd.timeout,
                SE2Cmd.minSpeed,
                SE2Cmd.maxSpeed
            );

            SE2HasCmd  = false;
            SE2Active  = false;
        }
        pros::delay(5);
    }
}

// =============================
// PUBLIC API: SE2ToPoint / SE2ToPose
// =============================
inline void SE2ToPoint(double targetX, double targetY,
                       int timeout = 2000,
                       SE2ParamsPublic params = {})
{
    ensureSE2Task();

    if (SE2Task == nullptr) {
        _se2Core(
            targetX, targetY,
            false, 0.0,
            !params.forwards,
            timeout,
            params.minSpeed,
            params.maxSpeed > 0 ? params.maxSpeed : 127.0
        );
        return;
    }

    while (SE2Active) pros::delay(5);

    SE2Cmd.targetX         = targetX;
    SE2Cmd.targetY         = targetY;
    SE2Cmd.useFinalHeading = false;
    SE2Cmd.targetThetaDeg  = 0.0;
    SE2Cmd.backwards       = !params.forwards;
    SE2Cmd.timeout         = timeout;
    SE2Cmd.minSpeed        = params.minSpeed;
    SE2Cmd.maxSpeed        = params.maxSpeed > 0 ? params.maxSpeed : 127.0;

    SE2HasCmd = true;

    if (!params.async)
        while (SE2HasCmd) pros::delay(5);
}

inline void SE2ToPose(double targetX, double targetY, double targetThetaDeg,
                      int timeout = 2000,
                      SE2ParamsPublic params = {})
{
    ensureSE2Task();

    if (SE2Task == nullptr) {
        _se2Core(
            targetX, targetY,
            true, targetThetaDeg,
            !params.forwards,
            timeout,
            params.minSpeed,
            params.maxSpeed > 0 ? params.maxSpeed : 127.0
        );
        return;
    }

    while (SE2Active) pros::delay(5);

    SE2Cmd.targetX         = targetX;
    SE2Cmd.targetY         = targetY;
    SE2Cmd.useFinalHeading = true;
    SE2Cmd.targetThetaDeg  = targetThetaDeg;
    SE2Cmd.backwards       = !params.forwards;
    SE2Cmd.timeout         = timeout;
    SE2Cmd.minSpeed        = params.minSpeed;
    SE2Cmd.maxSpeed        = params.maxSpeed > 0 ? params.maxSpeed : 127.0;

    SE2HasCmd = true;

    if (!params.async)
        while (SE2HasCmd) pros::delay(5);
}
