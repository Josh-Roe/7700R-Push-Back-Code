#include "main.h"
#include "autoroutes.h"
#include "lemlib/api.hpp"
#include "settings.h"
#include "usercontroller.h"
// CONTROLLER SET UP
pros::Controller controller(pros::E_CONTROLLER_MASTER);

// DRIVETRAIN MOTOR SET UP
pros::MotorGroup leftMotors({FRONT_LEFT_DRIVEBASE, MID_LEFT_DRIVEBASE,
                             BACK_LEFT_DRIVEBASE},
                            pros::MotorGearset::blue);
pros::MotorGroup rightMotors({FRONT_RIGHT_DRIVEBASE, MID_RIGHT_DRIVEBASE,
                              BACK_RIGHT_DRIVEBASE},
                             pros::MotorGearset::blue);

// SENSORS SET UP
pros::Imu imu(IMU_SENSOR);
pros::Rotation horizontalEnc(HORIZONTAL_ROTATION_ODOM);
pros::Rotation verticalEnc(VERTICAL_ROTATION_ODOM);
pros::Distance distance_sensor_hood(DISTANCE_SENSOR_HOOD);
pros::Optical color_sensor_hood(COLOR_SENSOR_HOOD);

// OTHER MOTORS SET UP
pros::Motor conveyor(CONVEYOR_MOTOR, pros::MotorGearset::blue,
                     pros::v5::MotorUnits::degrees);
pros::Motor hoodroller(HOOD_ROLLER_MOTOR, pros::MotorGearset::blue,
                       pros::v5::MotorUnits::degrees);

pros::adi::DigitalOut wing(WING);
pros::adi::DigitalOut scraper(SCRAPER);
pros::adi::DigitalOut parkclamp(PARKCLAMP);
pros::adi::DigitalOut park(PARK);
pros::adi::DigitalOut hood_piston(HOODPISTON);

// TRACKING WHEEL OFFSETS
lemlib::TrackingWheel horizontal(&horizontalEnc, lemlib::Omniwheel::NEW_2,
                                 HORIZONTAL_ODOM_OFFSET);
lemlib::TrackingWheel vertical(&verticalEnc, lemlib::Omniwheel::NEW_2,
                               VERTICAL_ODOM_OFFSET);

// DRIVETRAIN SETTINGS
lemlib::Drivetrain drivetrain(&leftMotors, &rightMotors,
                              TRACKWIDTH, // Track Width
                              WHEEL_DIAMETER,
                              RPM, // RPM
                              2);

// LATERAL PID
lemlib::ControllerSettings linearController(
    LATERAL_KP,                  // proportional gain (kP)
    LATERAL_KI,                  // integral gain (kI)
    LATERAL_KD,                  // derivative gain (kD)
    LATERAL_ANTIWINDUP,          // anti windup
    LATERAL_SMALL_ERROR,         // small error range, in inches
    LATERAL_SMALL_ERROR_TIMEOUT, // small error range timeout, in milliseconds
    LATERAL_LARGE_ERROR,         // large error range, in inches
    LATERAL_LARGE_ERROR_TIMEOUT, // large error range timeout, in milliseconds
    LATERAL_SLEW                 // maximum acceleration (slew)
);

// ANGULAR PID
lemlib::ControllerSettings angularController(
    ANGULAR_KP,                  // proportional gain (kP)
    ANGULAR_KI,                  // integral gain (kI)
    ANGULAR_KD,                  // derivative gain (kD)
    ANGULAR_ANTIWINDUP,          // anti windup
    ANGULAR_SMALL_ERROR,         // small error range, in degrees
    ANGULAR_SMALL_ERROR_TIMEOUT, // small error range timeout, in milliseconds
    ANGULAR_LARGE_ERROR,         // large error range, in degrees
    ANGULAR_LARGE_ERROR_TIMEOUT, // large error range timeout, in milliseconds
    ANGULAR_SLEW                 // maximum acceleration (slew)
);

// ODOMETRY SET UP
lemlib::OdomSensors sensors(nullptr, // vertical tracking wheel
                            nullptr, // vertical tracking wheel 2, set to
                                     // nullptr as we don't have a second one
                            &horizontal, // horizontal tracking wheel
                            nullptr,     // horizontal tracking wheel 2, set to
                                     // nullptr as we don't have a second one
                            &imu // inertial sensor
);

// DRIVER CONTROLLER THROTTLE
lemlib::ExpoDriveCurve
    throttleCurve(3,    // joystick deadband out of 127
                  10,   // minimum output where drivetrain will move out of 127
                  1.019 // expo curve gain
    );

// DRIVER CONTROLLER STEER
lemlib::ExpoDriveCurve
    steerCurve(3,    // joystick deadband out of 127
               10,   // minimum output where drivetrain will move out of 127
               1.019 // expo curve gain
    );

// CHASSIS CREATION
lemlib::Chassis chassis(drivetrain, linearController, angularController,
                        sensors, &throttleCurve, &steerCurve);

int autoMode = RUN_THIS_AUTO - 1;
int maxAutoMode = TOTAL_AUTONOMOUS_ROUTES - 1;
std::string autoType[] = {
    "Left Quals Auto", "Right Quals Auto", "Win Point Auto",
    "Left Elims Auto", "Right Elims Auto", "Tuning Auto",
    "Skills Auto",     "Left Rush Auto",   "Right Rush Auto"};

void changeAuto() {
  if (autoMode < maxAutoMode) {
    autoMode++;
  } else {
    autoMode = 0;
  }
}

void initialize() {
  pros::lcd::initialize();
  chassis.calibrate();
  chassis.setPose({0, 0, 0});
  hoodroller.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);
  conveyor.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);
  start_scoring_task();
  setScoringMode("NONE");
  // SCREEN DISPLAY
  pros::Task screenTask([&]() {
    while (true) {
      pros::lcd::print(0, "X: %f", chassis.getPose().x);
      pros::lcd::print(1, "Y: %f", chassis.getPose().y);
      pros::lcd::print(2, "Theta: %f", chassis.getPose().theta);

      // log position telemetry
      lemlib::telemetrySink()->info("Chassis pose: {}", chassis.getPose());

      pros::lcd::print(5, "Auto Selected: %s", autoType[autoMode]);
      pros::lcd::register_btn0_cb(changeAuto);
      pros::delay(50);
    }
  });
}

void disabled() {}

void competition_initialize() {}

void autonomous() {
  switch (autoMode) {
  case 0:
    leftQualsAuto();
    break;
  case 1:
    rightQualsAuto();
    break;
  case 2:
    fullWPAuto();
    break;
  case 3:
    leftElimsAuto();
    break;
  case 4:
    rightElimsAuto();
    break;

  case 5:
    tuningAuto();
    break;

  case 6:
    skillsAuto();
    break;

  case 7:
    leftRush();
    break;

  case 8:
    rightRush();
    break;
  }
}

void opcontrol() { usercontrol(); }
