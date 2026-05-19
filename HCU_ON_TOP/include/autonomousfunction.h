#pragma once
#include "main.h"
#include "settings.h"
#include <cmath>
#include <algorithm>

// ========================================================================
// =======================  RAMSETE CONTROLLER  ===========================
// ========================================================================

// --------------------- Public & internal params -------------------------

inline double normalizeAngleDeg(double angle) {
    while (angle > 180.0) angle -= 360.0;
    while (angle < -180.0) angle += 360.0;
    return angle;
}

inline void moveDistance(
    double inches,
    int timeout,
    double maxSpeed = 127,
    double minSpeed = 5.0
) {
    const double kP = MOVE_DISTANCE_P;
    const double kD = LATERAL_KD;

    // Heading correction gain
    const double kHeadingP = 1.8;   // tune this
    // optional derivative if wanted later:
    // const double kHeadingD = 0.0;

    const double tolerance         = LATERAL_SMALL_ERROR;
    const int    smallErrorTimeout = LATERAL_SMALL_ERROR_TIMEOUT;
    const int    loopDelay         = 10;

    lemlib::Pose start = chassis.getPose();
    double targetHeading = start.theta;

    const double DEG_TO_RAD = M_PI / 180.0;
    double theta0Rad = targetHeading * DEG_TO_RAD;

    // LemLib frame: heading 0° = +Y
    double forwardX = std::sin(theta0Rad);
    double forwardY = std::cos(theta0Rad);

    double error     = inches;
    double prevError = error;

    uint32_t startTime             = pros::millis();
    uint32_t withinSmallErrorStart = 0;

    while (pros::millis() - startTime < static_cast<uint32_t>(timeout)) {
        lemlib::Pose cur = chassis.getPose();

        // Distance traveled along original heading
        double dx = cur.x - start.x;
        double dy = cur.y - start.y;
        double traveled = dx * forwardX + dy * forwardY;

        error = inches - traveled;

        // small-error timeout
        if (std::fabs(error) < tolerance) {
            if (withinSmallErrorStart == 0) {
                withinSmallErrorStart = pros::millis();
            }
            if (pros::millis() - withinSmallErrorStart >=
                static_cast<uint32_t>(smallErrorTimeout)) {
                break;
            }
        } else {
            withinSmallErrorStart = 0;
        }

        // Forward PD
        double derivative = 0.0;
        if (std::fabs(error) > tolerance * 2.0) {
            derivative = error - prevError;
        }
        prevError = error;

        double forward = kP * error + kD * derivative;

        // Clamp forward
        if (forward > maxSpeed) forward = maxSpeed;
        if (forward < -maxSpeed) forward = -maxSpeed;

        // Enforce minimum speed only if not basically done
        if (std::fabs(forward) < minSpeed && std::fabs(error) > tolerance) {
            forward = (forward >= 0 ? minSpeed : -minSpeed);
        }

        // Heading hold
        double headingError = normalizeAngleDeg(targetHeading - cur.theta);
        double turn = kHeadingP * headingError;

        // Combine
        double left  = forward + turn;
        double right = forward - turn;

        // Final clamp
        if (left > maxSpeed) left = maxSpeed;
        if (left < -maxSpeed) left = -maxSpeed;
        if (right > maxSpeed) right = maxSpeed;
        if (right < -maxSpeed) right = -maxSpeed;

        chassis.tank((int)left, (int)right, true);
        pros::delay(loopDelay);
    }

    chassis.tank(0, 0, true);
}


inline pros::Task* imuFixTask = nullptr;

inline void fixPose() {
    float theta = imu.get_rotation();
    chassis.setPose({chassis.getPose().x, chassis.getPose().y, theta + chassis.getPose().theta});

    float initTheta = chassis.getPose().theta;

    while (true) {
        theta = imu.get_rotation() * IMU_SCALE + initTheta;
        chassis.setPose({chassis.getPose().x, chassis.getPose().y, theta});
        pros::delay(10);
    }
}

inline void startFixPoseTask() {
    if (imuFixTask == nullptr) {
        imuFixTask = new pros::Task([](){
            fixPose();
        });
    }
}

inline void driveUntilPark() {
    int start = pros::millis();
while (imu.get_roll() < 3.0 && pros::millis() - start < 3000) {
    chassis.tank(80, 80);
    pros::delay(10);
}
chassis.tank(0, 0);
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
      chassis.setPose({float(72-(distance_sensor_back.get() / 25.4f) - 5.5), float(-72+(distance_sensor_left.get() / 25.4f) + 7.5), chassis.getPose().theta});
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


inline double _wrapRad(double a) {
    while (a > M_PI) a -= 2.0 * M_PI;
    while (a < -M_PI) a += 2.0 * M_PI;
    return a;
}

inline double _wrapDeg(double a) {
    while (a > 180.0) a -= 360.0;
    while (a <= -180.0) a += 360.0;
    return a;
}

inline double _clampDouble(double v, double lo, double hi) {
    return std::max(lo, std::min(v, hi));
}

inline double _sgn(double v) {
    return (v >= 0.0) ? 1.0 : -1.0;
}

inline int _motorCmd(double v) {
    return static_cast<int>(std::round(_clampDouble(v, -127.0, 127.0)));
}

inline double _sinc(double x) {
    return (std::fabs(x) < 1e-6) ? 1.0 : std::sin(x) / x;
}

inline double _angleMeanDeg(double aDeg, double bDeg, double alpha) {
    return _wrapDeg(aDeg + alpha * _wrapDeg(bDeg - aDeg));
}

// LemLib convention in your code:
// 0 deg = +Y, +90 deg = +X.
// Positive turn should increase heading, so:
// left = forward + turn, right = forward - turn.
inline void _tankFromVTurn(double v, double turn, double maxSpeed, double minSpeed, double errorForMinSpeed) {
    maxSpeed = _clampDouble(maxSpeed, 1.0, 127.0);

    double leftCmd  = v + turn;
    double rightCmd = v - turn;

    double maxMag = std::max(std::fabs(leftCmd), std::fabs(rightCmd));
    if (maxMag > maxSpeed && maxMag > 1e-6) {
        double scale = maxSpeed / maxMag;
        leftCmd *= scale;
        rightCmd *= scale;
    }

    if (minSpeed > 0.0 && std::fabs(errorForMinSpeed) > 0.05) {
        if (std::fabs(leftCmd) > 1e-6 && std::fabs(leftCmd) < minSpeed) {
            leftCmd = _sgn(leftCmd) * minSpeed;
        }
        if (std::fabs(rightCmd) > 1e-6 && std::fabs(rightCmd) < minSpeed) {
            rightCmd = _sgn(rightCmd) * minSpeed;
        }
    }

    maxMag = std::max(std::fabs(leftCmd), std::fabs(rightCmd));
    if (maxMag > maxSpeed && maxMag > 1e-6) {
        double scale = maxSpeed / maxMag;
        leftCmd *= scale;
        rightCmd *= scale;
    }

    chassis.tank(_motorCmd(leftCmd), _motorCmd(rightCmd), true);
}

inline void _poseErrorRobotFrame(
    double targetX,
    double targetY,
    bool useFinalHeading,
    double targetThetaDeg,
    bool backwards,
    double finalHeadingSwitchDist,
    double& forwardErr,
    double& sideErr,
    double& headingErrRad,
    double& posErr,
    double& desiredThetaRad
) {
    constexpr double DEG2RAD = M_PI / 180.0;

    lemlib::Pose cur = chassis.getPose();
    double theta = cur.theta * DEG2RAD;

    double dx = targetX - cur.x;
    double dy = targetY - cur.y;

    double pointTheta = backwards ? std::atan2(-dx, -dy) : std::atan2(dx, dy);
    posErr = std::sqrt(dx * dx + dy * dy);

    if (useFinalHeading && posErr <= finalHeadingSwitchDist) {
        desiredThetaRad = targetThetaDeg * DEG2RAD;
    } else {
        desiredThetaRad = pointTheta;
    }

    headingErrRad = _wrapRad(desiredThetaRad - theta);

    double sinT = std::sin(theta);
    double cosT = std::cos(theta);

    forwardErr = dx * sinT + dy * cosT;
    sideErr = -dx * cosT + dy * sinT;
}

// ========================================================================
// RAMSETE
// ========================================================================

struct RamseteParamsPublic {
    bool forwards = true;
    double minSpeed = 0;
    double maxSpeed = 127;
    bool async = false;
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

inline pros::Task* RamseteTask = nullptr;
inline volatile bool RamseteActive = false;
inline volatile bool RamseteHasCmd = false;
inline volatile bool RamseteCancel = false;
inline RamseteParamsInternal RamseteCmd{};

inline void _ramseteCore(
    double targetX,
    double targetY,
    bool useFinalHeading,
    double targetThetaDeg,
    bool backwards,
    int timeout,
    double minSpeed,
    double maxSpeed
) {
    constexpr double RAD2DEG = 180.0 / M_PI;

    const double b = BETA;
    const double zeta = ZETA;
    const double kV = KV;
    const double kMax = KMAX;

    const double posTolInches = LATERAL_SMALL_ERROR / 2.0;
    const double thetaTolDeg = ANGULAR_SMALL_ERROR / 2.0;
    const double finalSwitch = 7.0;

    const int settleTimeMs = LATERAL_SMALL_ERROR_TIMEOUT;
    const int loopDelayMs = 10;

    uint32_t startTime = pros::millis();
    uint32_t settleStart = 0;
    RamseteCancel = false;

    while (!RamseteCancel && pros::millis() - startTime < static_cast<uint32_t>(timeout)) {
        double xErr = 0.0;
        double yErr = 0.0;
        double thetaErr = 0.0;
        double posErr = 0.0;
        double desiredTheta = 0.0;

        _poseErrorRobotFrame(
            targetX,
            targetY,
            useFinalHeading,
            targetThetaDeg,
            backwards,
            finalSwitch,
            xErr,
            yErr,
            thetaErr,
            posErr,
            desiredTheta
        );

        double finalHeadingErrDeg = _wrapDeg(targetThetaDeg - chassis.getPose().theta);

        bool closeToPoint = posErr < posTolInches;
        bool headingGood = (!useFinalHeading) || std::fabs(finalHeadingErrDeg) < thetaTolDeg;

        if (closeToPoint && headingGood) {
            if (settleStart == 0) settleStart = pros::millis();
            if (pros::millis() - settleStart >= static_cast<uint32_t>(settleTimeMs)) break;
        } else {
            settleStart = 0;
        }

        double thetaErrDeg = thetaErr * RAD2DEG;

        if (std::fabs(thetaErrDeg) > 85.0 && posErr > 3.0) {
            double turn = _clampDouble(0.8 * thetaErrDeg, -maxSpeed, maxSpeed);
            _tankFromVTurn(0.0, turn, maxSpeed, minSpeed, thetaErrDeg);
            pros::delay(loopDelayMs);
            continue;
        }

        double vRef = kV * posErr;
        double maxVRef = maxSpeed * 0.65;
        vRef = _clampDouble(vRef, 0.0, maxVRef);
        if (backwards) vRef = -vRef;

        double wRef = 0.0;
        double k = std::min(2.0 * zeta * std::sqrt(wRef * wRef + b * vRef * vRef), kMax);

        double v = vRef * std::cos(thetaErr) + k * xErr;
        double w = wRef + k * thetaErr + b * vRef * _sinc(thetaErr) * yErr;

        double turn = w * (TRACKWIDTH / 2.0);

        _tankFromVTurn(v, turn, maxSpeed, minSpeed, posErr);
        pros::delay(loopDelayMs);
    }

    chassis.tank(0, 0, true);
}

inline void RamseteTaskFn(void*);

inline void ensureRamseteTask() {
    if (RamseteTask == nullptr) {
        RamseteTask = new pros::Task(RamseteTaskFn, nullptr, "Ramsete");
        if (RamseteTask == nullptr) pros::lcd::print(7, "Ramsete task FAIL");
    }
}

inline void cancelRamsete() {
    RamseteCancel = true;
    RamseteHasCmd = false;
    RamseteActive = false;
    chassis.tank(0, 0, true);
}

inline void RamseteTaskFn(void*) {
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

            RamseteHasCmd = false;
            RamseteActive = false;
        }

        pros::delay(5);
    }
}

inline void RamseteToPoint(
    double targetX,
    double targetY,
    int timeout = 2000,
    RamseteParamsPublic params = {}
) {
    ensureRamseteTask();
    while (RamseteActive || RamseteHasCmd) pros::delay(5);

    RamseteCmd = {
        targetX,
        targetY,
        false,
        0.0,
        !params.forwards,
        timeout,
        params.minSpeed,
        params.maxSpeed > 0 ? params.maxSpeed : 127.0
    };

    RamseteCancel = false;
    RamseteHasCmd = true;

    if (!params.async) {
        while (RamseteHasCmd || RamseteActive) pros::delay(5);
    }
}

inline void RamseteToPose(
    double targetX,
    double targetY,
    double targetThetaDeg,
    int timeout = 2000,
    RamseteParamsPublic params = {}
) {
    ensureRamseteTask();
    while (RamseteActive || RamseteHasCmd) pros::delay(5);

    RamseteCmd = {
        targetX,
        targetY,
        true,
        targetThetaDeg,
        !params.forwards,
        timeout,
        params.minSpeed,
        params.maxSpeed > 0 ? params.maxSpeed : 127.0
    };

    RamseteCancel = false;
    RamseteHasCmd = true;

    if (!params.async) {
        while (RamseteHasCmd || RamseteActive) pros::delay(5);
    }
}

// ========================================================================
// SE2
// ========================================================================

struct SE2ParamsPublic {
    bool forwards = true;
    double minSpeed = 0;
    double maxSpeed = 127;
    bool async = false;
};

struct SE2ParamsInternal {
    double targetX;
    double targetY;
    bool useFinalHeading;
    double targetThetaDeg;
    bool backwards;
    int timeout;
    double minSpeed;
    double maxSpeed;
};

inline pros::Task* SE2Task = nullptr;
inline volatile bool SE2Active = false;
inline volatile bool SE2HasCmd = false;
inline volatile bool SE2Cancel = false;
inline SE2ParamsInternal SE2Cmd{};

inline void _se2Core(
    double targetX,
    double targetY,
    bool useFinalHeading,
    double targetThetaDeg,
    bool backwards,
    int timeout,
    double minSpeed,
    double maxSpeed
) {
    constexpr double RAD2DEG = 180.0 / M_PI;

    const double posTolInches = LATERAL_SMALL_ERROR / 2.0;
    const double thetaTolDeg = ANGULAR_SMALL_ERROR / 2.0;
    const double finalSwitch = 7.0;

    const int settleTimeMs = LATERAL_SMALL_ERROR_TIMEOUT;
    const int loopDelayMs = 10;

    uint32_t startTime = pros::millis();
    uint32_t settleStart = 0;
    SE2Cancel = false;

    while (!SE2Cancel && pros::millis() - startTime < static_cast<uint32_t>(timeout)) {
        double forwardErr = 0.0;
        double sideErr = 0.0;
        double headingErr = 0.0;
        double posErr = 0.0;
        double desiredTheta = 0.0;

        _poseErrorRobotFrame(
            targetX,
            targetY,
            useFinalHeading,
            targetThetaDeg,
            backwards,
            finalSwitch,
            forwardErr,
            sideErr,
            headingErr,
            posErr,
            desiredTheta
        );

        double finalHeadingErrDeg = _wrapDeg(targetThetaDeg - chassis.getPose().theta);

        bool posGood = posErr < posTolInches;
        bool thetaGood = (!useFinalHeading) || std::fabs(finalHeadingErrDeg) < thetaTolDeg;

        if (posGood && thetaGood) {
            if (settleStart == 0) settleStart = pros::millis();
            if (pros::millis() - settleStart >= static_cast<uint32_t>(settleTimeMs)) break;
        } else {
            settleStart = 0;
        }

        double headingErrDeg = headingErr * RAD2DEG;

        double v = KX * forwardErr;
        double turn = KY * sideErr + KTH * headingErrDeg;

        if (useFinalHeading && posErr < 3.0) {
            v = 0.0;
            turn = KTH * finalHeadingErrDeg;
        }

        if (!backwards && v < 0.0 && posErr > 2.0) v = 0.0;
        if (backwards && v > 0.0 && posErr > 2.0) v = 0.0;

        _tankFromVTurn(v, turn, maxSpeed, minSpeed, posErr + std::fabs(headingErrDeg));
        pros::delay(loopDelayMs);
    }

    chassis.tank(0, 0, true);
}

inline void SE2TaskFn(void*);

inline void ensureSE2Task() {
    if (SE2Task == nullptr) {
        SE2Task = new pros::Task(SE2TaskFn, nullptr, "SE2");
        if (SE2Task == nullptr) pros::lcd::print(7, "SE2 task FAIL");
    }
}

inline void cancelSE2() {
    SE2Cancel = true;
    SE2HasCmd = false;
    SE2Active = false;
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

            SE2HasCmd = false;
            SE2Active = false;
        }

        pros::delay(5);
    }
}

inline void SE2ToPoint(
    double targetX,
    double targetY,
    int timeout = 2000,
    SE2ParamsPublic params = {}
) {
    ensureSE2Task();
    while (SE2Active || SE2HasCmd) pros::delay(5);

    SE2Cmd = {
        targetX,
        targetY,
        false,
        0.0,
        !params.forwards,
        timeout,
        params.minSpeed,
        params.maxSpeed > 0 ? params.maxSpeed : 127.0
    };

    SE2Cancel = false;
    SE2HasCmd = true;

    if (!params.async) {
        while (SE2HasCmd || SE2Active) pros::delay(5);
    }
}

inline void SE2ToPose(
    double targetX,
    double targetY,
    double targetThetaDeg,
    int timeout = 2000,
    SE2ParamsPublic params = {}
) {
    ensureSE2Task();
    while (SE2Active || SE2HasCmd) pros::delay(5);

    SE2Cmd = {
        targetX,
        targetY,
        true,
        targetThetaDeg,
        !params.forwards,
        timeout,
        params.minSpeed,
        params.maxSpeed > 0 ? params.maxSpeed : 127.0
    };

    SE2Cancel = false;
    SE2HasCmd = true;

    if (!params.async) {
        while (SE2HasCmd || SE2Active) pros::delay(5);
    }
}

// ========================================================================
// LTV
// ========================================================================

struct LTVParamsPublic {
    bool forwards = true;
    double minSpeed = 0;
    double maxSpeed = 127;
    bool async = false;
};

struct LTVParamsInternal {
    double targetX;
    double targetY;
    bool useFinalHeading;
    double targetThetaDeg;
    bool backwards;
    int timeout;
    double minSpeed;
    double maxSpeed;
};

inline pros::Task* LTVTask = nullptr;
inline volatile bool LTVActive = false;
inline volatile bool LTVHasCmd = false;
inline volatile bool LTVCancel = false;
inline LTVParamsInternal LTVCmd{};

inline void _ltvCore(
    double targetX,
    double targetY,
    bool useFinalHeading,
    double targetThetaDeg,
    bool backwards,
    int timeout,
    double minSpeed,
    double maxSpeed
) {
    constexpr double RAD2DEG = 180.0 / M_PI;

    const double posTolInches = LATERAL_SMALL_ERROR / 2.0;
    const double thetaTolDeg = ANGULAR_SMALL_ERROR / 2.0;
    const double finalSwitch = 7.0;

    const int settleTimeMs = LATERAL_SMALL_ERROR_TIMEOUT;
    const int loopDelayMs = 10;

    uint32_t startTime = pros::millis();
    uint32_t settleStart = 0;
    LTVCancel = false;

    while (!LTVCancel && pros::millis() - startTime < static_cast<uint32_t>(timeout)) {
        double forwardErr = 0.0;
        double sideErr = 0.0;
        double headingErr = 0.0;
        double posErr = 0.0;
        double desiredTheta = 0.0;

        _poseErrorRobotFrame(
            targetX,
            targetY,
            useFinalHeading,
            targetThetaDeg,
            backwards,
            finalSwitch,
            forwardErr,
            sideErr,
            headingErr,
            posErr,
            desiredTheta
        );

        double finalHeadingErrDeg = _wrapDeg(targetThetaDeg - chassis.getPose().theta);

        bool posGood = posErr < posTolInches;
        bool thetaGood = (!useFinalHeading) || std::fabs(finalHeadingErrDeg) < thetaTolDeg;

        if (posGood && thetaGood) {
            if (settleStart == 0) settleStart = pros::millis();
            if (pros::millis() - settleStart >= static_cast<uint32_t>(settleTimeMs)) break;
        } else {
            settleStart = 0;
        }

        double vRef = _clampDouble(LTV_KV * posErr, 0.0, maxSpeed * 0.65);
        if (backwards) vRef = -vRef;

        double absV = std::fabs(vRef);
        double kx = LTV_KX + LTV_V_GAIN * absV;
        double ky = LTV_KY + LTV_V_GAIN * absV;
        double kth = LTV_KTH + 0.004 * absV;

        double headingErrDeg = headingErr * RAD2DEG;

        double v = vRef + kx * forwardErr;
        double turn = ky * sideErr + kth * headingErrDeg;

        if (useFinalHeading && posErr < 3.0) {
            v = 0.0;
            turn = kth * finalHeadingErrDeg;
        }

        if (!backwards && v < 0.0 && posErr > 2.0) v = 0.0;
        if (backwards && v > 0.0 && posErr > 2.0) v = 0.0;

        _tankFromVTurn(v, turn, maxSpeed, minSpeed, posErr + std::fabs(headingErrDeg));
        pros::delay(loopDelayMs);
    }

    chassis.tank(0, 0, true);
}

inline void LTVTaskFn(void*);

inline void ensureLTVTask() {
    if (LTVTask == nullptr) {
        LTVTask = new pros::Task(LTVTaskFn, nullptr, "LTV");
        if (LTVTask == nullptr) pros::lcd::print(7, "LTV task FAIL");
    }
}

inline void cancelLTV() {
    LTVCancel = true;
    LTVHasCmd = false;
    LTVActive = false;
    chassis.tank(0, 0, true);
}

inline void LTVTaskFn(void*) {
    while (true) {
        if (LTVHasCmd) {
            LTVActive = true;

            _ltvCore(
                LTVCmd.targetX,
                LTVCmd.targetY,
                LTVCmd.useFinalHeading,
                LTVCmd.targetThetaDeg,
                LTVCmd.backwards,
                LTVCmd.timeout,
                LTVCmd.minSpeed,
                LTVCmd.maxSpeed
            );

            LTVHasCmd = false;
            LTVActive = false;
        }

        pros::delay(5);
    }
}

inline void LTVToPoint(
    double targetX,
    double targetY,
    int timeout = 2000,
    LTVParamsPublic params = {}
) {
    ensureLTVTask();
    while (LTVActive || LTVHasCmd) pros::delay(5);

    LTVCmd = {
        targetX,
        targetY,
        false,
        0.0,
        !params.forwards,
        timeout,
        params.minSpeed,
        params.maxSpeed > 0 ? params.maxSpeed : 127.0
    };

    LTVCancel = false;
    LTVHasCmd = true;

    if (!params.async) {
        while (LTVHasCmd || LTVActive) pros::delay(5);
    }
}

inline void LTVToPose(
    double targetX,
    double targetY,
    double targetThetaDeg,
    int timeout = 2000,
    LTVParamsPublic params = {}
) {
    ensureLTVTask();
    while (LTVActive || LTVHasCmd) pros::delay(5);

    LTVCmd = {
        targetX,
        targetY,
        true,
        targetThetaDeg,
        !params.forwards,
        timeout,
        params.minSpeed,
        params.maxSpeed > 0 ? params.maxSpeed : 127.0
    };

    LTVCancel = false;
    LTVHasCmd = true;

    if (!params.async) {
        while (LTVHasCmd || LTVActive) pros::delay(5);
    }
}

inline void cancelMotionControllers() {
    cancelRamsete();
    cancelSE2();
    cancelLTV();
}

// ========================================================================
// MCL DISTANCE LOCALIZATION
// ========================================================================

struct MCLParticle {
    double x;
    double y;
    double thetaDeg;
    double weight;
};

struct MCLMeasurements {
    double front;
    double left;
    double right;
    double back;
    int count;
};

enum class MCLSensorSide {
    FRONT,
    LEFT,
    RIGHT,
    BACK
};

inline pros::Task* MCLTask = nullptr;
inline volatile bool MCLEnabled = false;
inline volatile bool MCLInitialized = false;

inline MCLParticle MCLParticles[MCL_NUM_PARTICLES];
inline MCLParticle MCLScratch[MCL_NUM_PARTICLES];

inline lemlib::Pose MCLLastOdomPose(0, 0, 0);
inline uint32_t MCLRandState = 0x31415926u;

inline double _mclRand01() {
    MCLRandState = 1664525u * MCLRandState + 1013904223u;
    return static_cast<double>((MCLRandState >> 8) & 0x00FFFFFFu) / static_cast<double>(0x01000000u);
}

inline double _mclRandSigned() {
    return 2.0 * _mclRand01() - 1.0;
}

inline double _mclReadInches(pros::Distance& sensor) {
    double inches = static_cast<double>(sensor.get()) / 25.4;

    if (!std::isfinite(inches)) return -1.0;
    if (inches < MCL_MIN_SENSOR_IN || inches > MCL_MAX_SENSOR_IN) return -1.0;

    return inches;
}

inline MCLMeasurements _mclReadMeasurements() {
    MCLMeasurements m{};

    m.front = _mclReadInches(distance_sensor_front);
    m.left = _mclReadInches(distance_sensor_left);
    m.right = _mclReadInches(distance_sensor_right);
    m.back = _mclReadInches(distance_sensor_back);

    m.count = 0;
    if (m.front > 0) m.count++;
    if (m.left > 0) m.count++;
    if (m.right > 0) m.count++;
    if (m.back > 0) m.count++;

    return m;
}

inline void _mclSensorVector(
    MCLSensorSide side,
    double thetaDeg,
    double& dirX,
    double& dirY,
    double& offset
) {
    double t = thetaDeg * M_PI / 180.0;
    double s = std::sin(t);
    double c = std::cos(t);

    switch (side) {
        case MCLSensorSide::FRONT:
            dirX = s;
            dirY = c;
            offset = MCL_FRONT_OFFSET;
            break;

        case MCLSensorSide::BACK:
            dirX = -s;
            dirY = -c;
            offset = MCL_BACK_OFFSET;
            break;

        case MCLSensorSide::LEFT:
            dirX = -c;
            dirY = s;
            offset = MCL_LEFT_OFFSET;
            break;

        case MCLSensorSide::RIGHT:
        default:
            dirX = c;
            dirY = -s;
            offset = MCL_RIGHT_OFFSET;
            break;
    }
}

inline double _mclRayToWall(double sx, double sy, double dirX, double dirY) {
    constexpr double FIELD = MCL_FIELD_HALF_SIZE;
    double best = 1e9;

    if (std::fabs(dirX) > 1e-9) {
        double t = (FIELD - sx) / dirX;
        double y = sy + t * dirY;
        if (t > 0 && y >= -FIELD && y <= FIELD) best = std::min(best, t);

        t = (-FIELD - sx) / dirX;
        y = sy + t * dirY;
        if (t > 0 && y >= -FIELD && y <= FIELD) best = std::min(best, t);
    }

    if (std::fabs(dirY) > 1e-9) {
        double t = (FIELD - sy) / dirY;
        double x = sx + t * dirX;
        if (t > 0 && x >= -FIELD && x <= FIELD) best = std::min(best, t);

        t = (-FIELD - sy) / dirY;
        x = sx + t * dirX;
        if (t > 0 && x >= -FIELD && x <= FIELD) best = std::min(best, t);
    }

    return best < 1e8 ? best : -1.0;
}

inline double _mclExpectedDistance(const MCLParticle& p, MCLSensorSide side) {
    double dirX = 0.0;
    double dirY = 0.0;
    double offset = 0.0;

    _mclSensorVector(side, p.thetaDeg, dirX, dirY, offset);

    double sx = p.x + dirX * offset;
    double sy = p.y + dirY * offset;

    return _mclRayToWall(sx, sy, dirX, dirY);
}

inline void _mclAddSensorError(double measured, double expected, double& errSq, int& used) {
    if (measured <= 0.0 || expected <= 0.0) return;

    double err = measured - expected;
    errSq += (err * err) / (MCL_SENSOR_SIGMA * MCL_SENSOR_SIGMA);
    used++;
}

inline double _mclParticleWeight(const MCLParticle& p, const MCLMeasurements& m) {
    double errSq = 0.0;
    int used = 0;

    _mclAddSensorError(m.front, _mclExpectedDistance(p, MCLSensorSide::FRONT), errSq, used);
    _mclAddSensorError(m.left, _mclExpectedDistance(p, MCLSensorSide::LEFT), errSq, used);
    _mclAddSensorError(m.right, _mclExpectedDistance(p, MCLSensorSide::RIGHT), errSq, used);
    _mclAddSensorError(m.back, _mclExpectedDistance(p, MCLSensorSide::BACK), errSq, used);

    if (used < MCL_MIN_VALID_SENSORS) return 1.0;

    return std::exp(-0.5 * errSq);
}

inline void resetMCLParticlesAroundCurrentPose() {
    lemlib::Pose p = chassis.getPose();

    MCLRandState ^= pros::millis() + 0x9E3779B9u;

    for (int i = 0; i < MCL_NUM_PARTICLES; i++) {
        MCLParticles[i].x = p.x + _mclRandSigned() * MCL_INIT_XY_NOISE;
        MCLParticles[i].y = p.y + _mclRandSigned() * MCL_INIT_XY_NOISE;
        MCLParticles[i].thetaDeg = _wrapDeg(p.theta + _mclRandSigned() * MCL_INIT_THETA_NOISE);
        MCLParticles[i].weight = 1.0 / static_cast<double>(MCL_NUM_PARTICLES);
    }

    MCLLastOdomPose = p;
    MCLInitialized = true;
}

inline lemlib::Pose _mclEstimatePose() {
    double x = 0.0;
    double y = 0.0;
    double sinSum = 0.0;
    double cosSum = 0.0;

    for (int i = 0; i < MCL_NUM_PARTICLES; i++) {
        double w = MCLParticles[i].weight;

        x += MCLParticles[i].x * w;
        y += MCLParticles[i].y * w;

        double t = MCLParticles[i].thetaDeg * M_PI / 180.0;
        sinSum += std::sin(t) * w;
        cosSum += std::cos(t) * w;
    }

    double theta = std::atan2(sinSum, cosSum) * 180.0 / M_PI;

    return lemlib::Pose(
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(_wrapDeg(theta))
    );
}

inline void _mclNormalizeWeights() {
    double sum = 0.0;

    for (int i = 0; i < MCL_NUM_PARTICLES; i++) {
        sum += MCLParticles[i].weight;
    }

    if (sum <= 1e-12 || !std::isfinite(sum)) {
        for (int i = 0; i < MCL_NUM_PARTICLES; i++) {
            MCLParticles[i].weight = 1.0 / static_cast<double>(MCL_NUM_PARTICLES);
        }
        return;
    }

    for (int i = 0; i < MCL_NUM_PARTICLES; i++) {
        MCLParticles[i].weight /= sum;
    }
}

inline void _mclResample() {
    double step = 1.0 / static_cast<double>(MCL_NUM_PARTICLES);
    double r = _mclRand01() * step;
    double c = MCLParticles[0].weight;

    int i = 0;

    for (int m = 0; m < MCL_NUM_PARTICLES; m++) {
        double u = r + m * step;

        while (u > c && i < MCL_NUM_PARTICLES - 1) {
            i++;
            c += MCLParticles[i].weight;
        }

        MCLScratch[m] = MCLParticles[i];
        MCLScratch[m].weight = step;
    }

    for (int m = 0; m < MCL_NUM_PARTICLES; m++) {
        MCLParticles[m] = MCLScratch[m];
    }
}

inline bool mclLocalizeOnce() {
    if (!MCLInitialized) {
        resetMCLParticlesAroundCurrentPose();
    }

    MCLMeasurements meas = _mclReadMeasurements();

    if (meas.count < MCL_MIN_VALID_SENSORS) {
        MCLLastOdomPose = chassis.getPose();
        return false;
    }

    lemlib::Pose odom = chassis.getPose();

    double dx = odom.x - MCLLastOdomPose.x;
    double dy = odom.y - MCLLastOdomPose.y;
    double dTheta = _wrapDeg(odom.theta - MCLLastOdomPose.theta);

    for (int i = 0; i < MCL_NUM_PARTICLES; i++) {
        MCLParticles[i].x += dx + _mclRandSigned() * MCL_PROCESS_XY_NOISE;
        MCLParticles[i].y += dy + _mclRandSigned() * MCL_PROCESS_XY_NOISE;
        MCLParticles[i].thetaDeg = _wrapDeg(
            MCLParticles[i].thetaDeg + dTheta + _mclRandSigned() * MCL_PROCESS_THETA_NOISE
        );

        MCLParticles[i].weight = _mclParticleWeight(MCLParticles[i], meas);
    }

    _mclNormalizeWeights();

    lemlib::Pose estimate = _mclEstimatePose();

    _mclResample();

    double a = _clampDouble(MCL_POSE_BLEND, 0.0, 1.0);

    double correctedX = odom.x + a * (estimate.x - odom.x);
    double correctedY = odom.y + a * (estimate.y - odom.y);

#if MCL_CORRECT_HEADING
    double correctedTheta = _angleMeanDeg(odom.theta, estimate.theta, a);
#else
    double correctedTheta = odom.theta;
#endif

    chassis.setPose(
        static_cast<float>(correctedX),
        static_cast<float>(correctedY),
        static_cast<float>(correctedTheta)
    );

    MCLLastOdomPose = chassis.getPose();

    return true;
}

inline void MCLTaskFn(void*) {
    while (true) {
        if (MCLEnabled) {
            mclLocalizeOnce();
        }

        pros::delay(MCL_TASK_DELAY_MS);
    }
}

inline void ensureMCLTask() {
    if (MCLTask == nullptr) {
        MCLTask = new pros::Task(MCLTaskFn, nullptr, "MCL");
        if (MCLTask == nullptr) pros::lcd::print(7, "MCL task FAIL");
    }
}

inline void startMCL(bool resetParticles = true) {
    ensureMCLTask();

    if (resetParticles || !MCLInitialized) {
        resetMCLParticlesAroundCurrentPose();
    }

    MCLEnabled = true;
}

inline void stopMCL() {
    MCLEnabled = false;
    chassis.tank(0, 0, true);
}

inline void setMCL(bool enabled) {
    if (enabled) {
        startMCL(true);
    } else {
        stopMCL();
    }
}